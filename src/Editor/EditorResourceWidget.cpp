#include "EditorResourceWidget.h"
#include "Application.h"
#include "EditorActionBar.h"
#include "Backend/DatabaseManager.h"
#include "Backend/Project.h"
#include "Resources/ResourceTreeItem.h"
#include "Resources/ResourceTreeItemModel.h"
#include "Resources/ResourceTreeItemSortFilterProxyModel.h"
#include "Resources/WebResourceOverlay.h"
#include "Widgets/ResourceDisplayWidget.h"
#include "ui_EditorResourceWidget.h"
#include "ui_EditorActionBar.h"

// Testing
//#include "modeltest.h"

#include <QDebug>
#include <QDomDocument>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QItemSelectionModel>
#include <QMessageBox>
#include <QMimeData>
#include <QNetworkAccessManager>
#include <QSortFilterProxyModel>

#include <cassert>

CEditorResourceWidget::CEditorResourceWidget(QWidget* pParent) :
  CEditorWidgetBase(pParent),
  m_spUi(std::make_unique<Ui::CEditorResourceWidget>()),
  m_spWebOverlay(std::make_unique<CWebResourceOverlay>()),
  m_spNAManager(std::make_unique<QNetworkAccessManager>()),
  m_spSettings(CApplication::Instance()->Settings()),
  m_spCurrentProject(nullptr),
  m_pResponse(nullptr)
{
  m_spUi->setupUi(this);
  connect(m_spWebOverlay.get(), &CWebResourceOverlay::SignalResourceSelected,
          this, &CEditorResourceWidget::SlotWebResourceSelected);
}

CEditorResourceWidget::~CEditorResourceWidget()
{
  if (nullptr != m_pResponse)
  {
    m_pResponse->abort();
    m_pResponse->disconnect();
    delete m_pResponse;
    m_pResponse = nullptr;
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorResourceWidget::Initialize()
{
  m_bInitialized = false;

  m_wpDbManager = CApplication::Instance()->System<CDatabaseManager>();

  m_spUi->pResourceTree->header()->setStretchLastSection(true);
  m_spUi->pResourceTree->header()->setSectionResizeMode(QHeaderView::ResizeToContents);

  CResourceTreeItemSortFilterProxyModel* pProxyModel =
      new CResourceTreeItemSortFilterProxyModel(m_spUi->pResourceTree);
  pProxyModel->setSourceModel(ResourceModel());
  m_spUi->pResourceTree->setModel(pProxyModel);

  QItemSelectionModel* pSelectionModel = m_spUi->pResourceTree->selectionModel();
  connect(pSelectionModel, &QItemSelectionModel::currentChanged,
          this, &CEditorResourceWidget::SlotCurrentChanged);

  m_spWebOverlay->Hide();

  setAcceptDrops(true);

  pProxyModel->sort(resource_item::c_iColumnName, Qt::AscendingOrder);
  pProxyModel->setFilterRegExp(QRegExp(".*", Qt::CaseInsensitive, QRegExp::RegExp));

  m_spUi->pResourceDisplayWidget->setFixedHeight(256);

  m_bInitialized = true;
}


//----------------------------------------------------------------------------------------
//
void CEditorResourceWidget::LoadProject(tspProject spCurrentProject)
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr != m_spCurrentProject)
  {
    qWarning() << "Old Project was not unloaded before loading project.";
  }

  m_spCurrentProject = spCurrentProject;
}

//----------------------------------------------------------------------------------------
//
void CEditorResourceWidget::UnloadProject()
{
  WIDGET_INITIALIZED_GUARD

  m_spCurrentProject = nullptr;

  m_spUi->pResourceDisplayWidget->UnloadResource();
  m_spWebOverlay->Hide();
}

//----------------------------------------------------------------------------------------
//
void CEditorResourceWidget::OnActionBarAboutToChange()
{
  if (nullptr != ActionBar())
  {
    disconnect(ActionBar()->m_spUi->AddResourceButton, &QPushButton::clicked,
            this, &CEditorResourceWidget::SlotAddButtonClicked);
    disconnect(ActionBar()->m_spUi->AddWebResourceButton, &QPushButton::clicked,
            this, &CEditorResourceWidget::SlotAddWebButtonClicked);
    disconnect(ActionBar()->m_spUi->RemoveResourceButton, &QPushButton::clicked,
            this, &CEditorResourceWidget::SlotRemoveButtonClicked);
    disconnect(ActionBar()->m_spUi->TitleCardButton, &QPushButton::clicked,
            this, &CEditorResourceWidget::SlotTitleCardButtonClicked);
    disconnect(ActionBar()->m_spUi->MapButton, &QPushButton::clicked,
            this, &CEditorResourceWidget::SlotMapButtonClicked);
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorResourceWidget::OnActionBarChanged()
{
  // connections for actionbar
  if (nullptr != ActionBar())
  {
    ActionBar()->ShowResourceActionBar();
    connect(ActionBar()->m_spUi->AddResourceButton, &QPushButton::clicked,
            this, &CEditorResourceWidget::SlotAddButtonClicked);
    connect(ActionBar()->m_spUi->AddWebResourceButton, &QPushButton::clicked,
            this, &CEditorResourceWidget::SlotAddWebButtonClicked);
    connect(ActionBar()->m_spUi->RemoveResourceButton, &QPushButton::clicked,
            this, &CEditorResourceWidget::SlotRemoveButtonClicked);
    connect(ActionBar()->m_spUi->TitleCardButton, &QPushButton::clicked,
            this, &CEditorResourceWidget::SlotTitleCardButtonClicked);
    connect(ActionBar()->m_spUi->MapButton, &QPushButton::clicked,
            this, &CEditorResourceWidget::SlotMapButtonClicked);
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorResourceWidget::dragEnterEvent(QDragEnterEvent* pEvent)
{
  const QMimeData* pMimeData = pEvent->mimeData();

  // check for our needed mime type, here a file or a list of files
  if (pMimeData->hasUrls())
  {
    pEvent->acceptProposedAction();
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorResourceWidget::dropEvent(QDropEvent* pEvent)
{
  const QMimeData* pMimeData = pEvent->mimeData();

  // check for our needed mime type, here a file or a list of files
  if (pMimeData->hasUrls())
  {
    QStringList imageFormatsList = ImageFormats();
    QString sImageFormats = imageFormatsList.join(" ");

    QStringList videoFormatsList = VideoFormats();
    QString sVideoFormats = videoFormatsList.join(" ");

    QStringList audioFormatsList = AudioFormats();
    QString sAudioFormats = audioFormatsList.join(" ");

    QStringList otherFormatsList = OtherFormats();
    QString sOtherFormats = otherFormatsList.join(" ");

    QStringList vsFileNames;
    QList<QUrl> vUrlList = pMimeData->urls();

    // extract the local paths of the files
    for (qint32 i = 0; i < vUrlList.size(); ++i)
    {
      vsFileNames.append(vUrlList.at(i).toLocalFile());
    }

    // add file to respective category
    AddFiles(vsFileNames, imageFormatsList, videoFormatsList, audioFormatsList, otherFormatsList);
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorResourceWidget::on_pFilter_SignalFilterChanged(const QString& sText)
{
  WIDGET_INITIALIZED_GUARD
  CResourceTreeItemSortFilterProxyModel* pProxyModel =
      dynamic_cast<CResourceTreeItemSortFilterProxyModel*>(m_spUi->pResourceTree->model());

  if (sText.isNull() || sText.isEmpty())
  {
    pProxyModel->setFilterRegExp(QRegExp(".*", Qt::CaseInsensitive, QRegExp::RegExp));
  }
  else
  {
    pProxyModel->setFilterRegExp(QRegExp(sText, Qt::CaseInsensitive, QRegExp::RegExp));
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorResourceWidget::on_pResourceDisplayWidget_OnClick()
{
  WIDGET_INITIALIZED_GUARD

  m_spUi->pResourceDisplayWidget->SlotPlayPause();
}

//----------------------------------------------------------------------------------------
//
void CEditorResourceWidget::on_pResourceDisplayWidget_SignalLoadFinished()
{
  WIDGET_INITIALIZED_GUARD

  m_spUi->pResourceDisplayWidget->SlotSetSliderVisible(true);
}

//----------------------------------------------------------------------------------------
//
void CEditorResourceWidget::SlotAddButtonClicked()
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }

  QStringList imageFormatsList = ImageFormats();
  QString sImageFormats = imageFormatsList.join(" ");

  QStringList videoFormatsList = VideoFormats();
  QString sVideoFormats = videoFormatsList.join(" ");

  QStringList audioFormatsList = AudioFormats();
  QString sAudioFormats = audioFormatsList.join(" ");

  QStringList otherFormatsList = OtherFormats();
  QString sOtherFormats = otherFormatsList.join(" ");

  QString sFormatSelection = "Image Files (%1);;Video Files (%2);;Sound Files (%3);;Other Files (%4)";
  QString sCurrentFolder = CApplication::Instance()->Settings()->ContentFolder();
  QStringList  vsFileNames = QFileDialog::getOpenFileNames(this,
      tr("Add File"), sCurrentFolder,
      sFormatSelection.arg(sImageFormats).arg(sVideoFormats).arg(sAudioFormats).arg(sOtherFormats));

  // add file to respective category
  AddFiles(vsFileNames, imageFormatsList, videoFormatsList, audioFormatsList, otherFormatsList);
}

//----------------------------------------------------------------------------------------
//
void CEditorResourceWidget::SlotAddWebButtonClicked()
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }
  m_spWebOverlay->Show();
}

//----------------------------------------------------------------------------------------
//
void CEditorResourceWidget::SlotRemoveButtonClicked()
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }

  QSortFilterProxyModel* pProxyModel =
    dynamic_cast<QSortFilterProxyModel*>(m_spUi->pResourceTree->model());
  CResourceTreeItemModel* pModel =
    dynamic_cast<CResourceTreeItemModel*>(pProxyModel->sourceModel());

  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager && nullptr != pModel)
  {
    QModelIndexList indexes = m_spUi->pResourceTree->selectionModel()->selectedIndexes();
    m_spUi->pResourceTree->selectionModel()->clearSelection();
    foreach (QModelIndex index, indexes)
    {
      if (pModel->IsResourceType(pProxyModel->mapToSource(index)))
      {
        const QString sName = pModel->data(pProxyModel->mapToSource(index), Qt::DisplayRole).toString();
        spDbManager->RemoveResource(m_spCurrentProject, sName);

        emit SignalProjectEdited();

        // only interrested in first item which is the actual item we need
        break;
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorResourceWidget::SlotTitleCardButtonClicked()
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }

  QSortFilterProxyModel* pProxyModel =
    dynamic_cast<QSortFilterProxyModel*>(m_spUi->pResourceTree->model());
  CResourceTreeItemModel* pModel =
    dynamic_cast<CResourceTreeItemModel*>(pProxyModel->sourceModel());

  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager && nullptr != pModel)
  {
    QModelIndexList indexes = m_spUi->pResourceTree->selectionModel()->selectedIndexes();
    m_spUi->pResourceTree->selectionModel()->clearSelection();
    foreach (QModelIndex index, indexes)
    {
      if (pModel->IsResourceType(pProxyModel->mapToSource(index)))
      {
        const QString sName = pModel->data(pProxyModel->mapToSource(index), Qt::DisplayRole).toString();
        tspResource spResource = spDbManager->FindResource(m_spCurrentProject, sName);
        if (nullptr != spResource)
        {
          QWriteLocker locker(&m_spCurrentProject->m_rwLock);
          m_spCurrentProject->m_sTitleCard = sName;

          emit SignalProjectEdited();

          // only interrested in first item which is the actual item we need
          break;
        }
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorResourceWidget::SlotMapButtonClicked()
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }

  QSortFilterProxyModel* pProxyModel =
    dynamic_cast<QSortFilterProxyModel*>(m_spUi->pResourceTree->model());
  CResourceTreeItemModel* pModel =
    dynamic_cast<CResourceTreeItemModel*>(pProxyModel->sourceModel());

  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager && nullptr != pModel)
  {
    QModelIndexList indexes = m_spUi->pResourceTree->selectionModel()->selectedIndexes();
    m_spUi->pResourceTree->selectionModel()->clearSelection();
    foreach (QModelIndex index, indexes)
    {
      if (pModel->IsResourceType(pProxyModel->mapToSource(index)))
      {
        const QString sName = pModel->data(pProxyModel->mapToSource(index), Qt::DisplayRole).toString();
        tspResource spResource = spDbManager->FindResource(m_spCurrentProject, sName);
        if (nullptr != spResource)
        {
          QWriteLocker locker(&m_spCurrentProject->m_rwLock);
          m_spCurrentProject->m_sMap = sName;

          emit SignalProjectEdited();

          // only interrested in first item which is the actual item we need
          break;
        }
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorResourceWidget::SlotCurrentChanged(const QModelIndex& current,
                                               const QModelIndex& previous)
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }
  Q_UNUSED(previous);

  QSortFilterProxyModel* pProxyModel =
    dynamic_cast<QSortFilterProxyModel*>(m_spUi->pResourceTree->model());
  CResourceTreeItemModel* pModel =
    dynamic_cast<CResourceTreeItemModel*>(pProxyModel->sourceModel());

  if (nullptr != pModel)
  {
    const QString sName =
        pModel->data(pProxyModel->mapToSource(current), Qt::DisplayRole, resource_item::c_iColumnName).toString();
    emit SignalResourceSelected(sName);

    auto spDbManager = m_wpDbManager.lock();
    if (nullptr != spDbManager)
    {
      auto spResource = spDbManager->FindResource(m_spCurrentProject, sName);
      m_spUi->pResourceDisplayWidget->UnloadResource();
      m_spUi->pResourceDisplayWidget->LoadResource(spResource);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorResourceWidget::SlotWebResourceSelected(const QString& sResource)
{
  m_spWebOverlay->Hide();
  QUrl url(sResource);
  if (url.isValid())
  {
    if (nullptr != m_pResponse)
    {
      m_pResponse->abort();
      m_pResponse->disconnect();
      delete m_pResponse;
      m_pResponse = nullptr;
    }
    m_pResponse = m_spNAManager->get(QNetworkRequest(url));
    connect(m_pResponse, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
            this, &CEditorResourceWidget::SlotNetworkReplyError);
    connect(m_pResponse, &QNetworkReply::finished,
            this, &CEditorResourceWidget::SlotNetworkReplyFinished);
  }
  else
  {
    qWarning() << "Non-valid url.";
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorResourceWidget::SlotNetworkReplyError(QNetworkReply::NetworkError code)
{
  QNetworkReply* pReply = dynamic_cast<QNetworkReply*>(sender());
  assert(nullptr != pReply);
  if (nullptr != pReply)
  {
    qWarning() << code << pReply->errorString();
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorResourceWidget::SlotNetworkReplyFinished()
{
  QNetworkReply* pReply = dynamic_cast<QNetworkReply*>(sender());
  assert(nullptr != pReply);
  if (nullptr != pReply)
  {
    QUrl url = pReply->url();
    QByteArray arr = pReply->readAll();
    if (nullptr != m_pResponse)
    {
      m_pResponse->disconnect();
      m_pResponse->deleteLater();
    }

    QStringList imageFormatsList = ImageFormats();
    QStringList videoFormatsList = VideoFormats();
    QStringList audioFormatsList = AudioFormats();

    qint32 iLastIndex = url.fileName().lastIndexOf('.');
    const QString sFileName = url.fileName();
    QString sFormat = "*" + sFileName.mid(iLastIndex, sFileName.size() - iLastIndex);
    auto spDbManager = m_wpDbManager.lock();
    if (nullptr != spDbManager)
    {
      if (imageFormatsList.contains(sFormat))
      {
        QPixmap mPixmap;
        mPixmap.loadFromData(arr);
        if (!mPixmap.isNull())
        {
          spDbManager->AddResource(m_spCurrentProject, url, EResourceType::eImage);
          emit SignalProjectEdited();
        }
      }
      else if (videoFormatsList.contains(sFormat))
      {
        // TODO: check video
        spDbManager->AddResource(m_spCurrentProject, url, EResourceType::eMovie);
        emit SignalProjectEdited();
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorResourceWidget::AddFiles(const QStringList& vsFiles, const QStringList& imageFormatsList,
              const QStringList& videoFormatsList, const QStringList& audioFormatsList,
              const QStringList& otherFormatsList)
{
  // add file to respective category
  bool bAddedFiles = false;
  QStringList vsNeedsToMove;
  const QString sName = PhysicalProjectName(m_spCurrentProject);
  const QDir projectDir(m_spSettings->ContentFolder() + "/" + sName);
  for (QString sFileName : vsFiles)
  {
    QFileInfo info(sFileName);
    if (!info.canonicalFilePath().contains(projectDir.absolutePath()))
    {
      vsNeedsToMove.push_back(sFileName);
    }
    else
    {
      QString sRelativePath = projectDir.relativeFilePath(sFileName);
      QUrl url = QUrl::fromLocalFile(sRelativePath);
      const QString sEnding = "*." + info.suffix();
      auto spDbManager = m_wpDbManager.lock();
      if (nullptr != spDbManager)
      {
        if (imageFormatsList.contains(sEnding))
        {
          spDbManager->AddResource(m_spCurrentProject, url, EResourceType::eImage);
          bAddedFiles = true;
        }
        else if (videoFormatsList.contains(sEnding))
        {
          spDbManager->AddResource(m_spCurrentProject, url, EResourceType::eMovie);
          bAddedFiles = true;
        }
        else if (audioFormatsList.contains(sEnding))
        {
          spDbManager->AddResource(m_spCurrentProject, url, EResourceType::eSound);
          bAddedFiles = true;
        }
        else if (otherFormatsList.contains(sEnding))
        {
          spDbManager->AddResource(m_spCurrentProject, url, EResourceType::eOther);
          bAddedFiles = true;
        }
      }
    }
  }

  // handle action
  if (bAddedFiles)
  {
    emit SignalProjectEdited();
  }
  if (!vsNeedsToMove.isEmpty())
  {
    QMessageBox msgBox;
    msgBox.setText(tr("At least one file is not in the subfolder of project."));
    msgBox.setInformativeText(tr("Do you want to move or copy the file(s)?"));
    QPushButton* pMove = msgBox.addButton(tr("Move"), QMessageBox::AcceptRole);
    QPushButton* pCopy = msgBox.addButton(tr("Copy"), QMessageBox::ActionRole);
    QPushButton* pCancel = msgBox.addButton(QMessageBox::Cancel);
    msgBox.setDefaultButton(pCancel);
    msgBox.setModal(true);
    msgBox.setWindowFlag(Qt::FramelessWindowHint);

    QPointer<CEditorResourceWidget> pMeMyselfMyPointerAndI(this);
    msgBox.exec();
    if (nullptr == pMeMyselfMyPointerAndI)
    {
      return;
    }

    QStringList filesToAdd;
    if (msgBox.clickedButton() == pMove)
    {
      // Move the Files
      const QString sDirToMoveTo = QFileDialog::getExistingDirectory(this,
          tr("Select Destination"), projectDir.absolutePath());
      for (QString sFileName : vsFiles)
      {
        QFileInfo info(sFileName);
        QFile file(info.absoluteFilePath());
        const QString sNewName = sDirToMoveTo + "/" + info.fileName();
        if (!file.rename(sNewName))
        {
          qWarning() << QString(tr("Renaming file '%1' failed.")).arg(sNewName);
        }
        else
        {
          if (sNewName.contains(projectDir.absolutePath()))
          {
            filesToAdd.push_back(sNewName);
          }
        }
      }
      AddFiles(filesToAdd, imageFormatsList, videoFormatsList, audioFormatsList, otherFormatsList);
    }
    else if (msgBox.clickedButton() == pCopy)
    {
      // copy the Files
      const QString sDirToCopyTo = QFileDialog::getExistingDirectory(this,
          tr("Select Destination"), projectDir.absolutePath());
      for (QString sFileName : vsFiles)
      {
        QFileInfo info(sFileName);
        QFile file(info.absoluteFilePath());
        const QString sNewName = sDirToCopyTo + "/" + info.fileName();
        if (!file.copy(sNewName))
        {
          qWarning() << QString(tr("Copying file '%1' failed.")).arg(sNewName);
        }
        else
        {
          if (sNewName.contains(projectDir.absolutePath()))
          {
            filesToAdd.push_back(sNewName);
          }
        }
      }
      AddFiles(filesToAdd, imageFormatsList, videoFormatsList, audioFormatsList, otherFormatsList);
    }
    else if (msgBox.clickedButton() == pCancel)
    {
      // nothing to do
    }
  }
}
