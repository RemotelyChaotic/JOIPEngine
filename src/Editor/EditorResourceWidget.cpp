#include "EditorResourceWidget.h"
#include "Application.h"
#include "EditorActionBar.h"
#include "ResourceTreeItem.h"
#include "ResourceTreeItemModel.h"
#include "WebResourceOverlay.h"
#include "Backend/DatabaseManager.h"
#include "Backend/Project.h"
#include "Widgets/ResourceDisplayWidget.h"
#include "ui_EditorResourceWidget.h"
#include "ui_EditorActionBar.h"

// Testing
#include "modeltest.h"

#include <QDebug>
#include <QDomDocument>
#include <QFileDialog>
#include <QFileInfo>
#include <QItemSelectionModel>
#include <QNetworkAccessManager>

#include <cassert>

CEditorResourceWidget::CEditorResourceWidget(QWidget* pParent, CEditorActionBar* pActionBar) :
  CEditorWidgetBase(pParent, pActionBar),
  m_spUi(std::make_unique<Ui::CEditorResourceWidget>()),
  m_spNAManager(std::make_unique<QNetworkAccessManager>()),
  m_spSettings(CApplication::Instance()->Settings()),
  m_spCurrentProject(nullptr),
  m_pResponse(nullptr)
{
  m_spUi->setupUi(this);
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

  CResourceTreeItemModel* pModel = new CResourceTreeItemModel(m_spUi->pResourceTree);
  m_spUi->pResourceTree->setModel(pModel);

  QItemSelectionModel* pSelectionModel = m_spUi->pResourceTree->selectionModel();
  connect(pSelectionModel, &QItemSelectionModel::currentChanged,
          this, &CEditorResourceWidget::SlotCurrentChanged);

  // connections for actionbar
  if (nullptr != m_pActionBar)
  {
    connect(m_pActionBar->m_spUi->pAddButton, &QPushButton::clicked,
            this, &CEditorResourceWidget::SlotAddButtonClicked);
    connect(m_pActionBar->m_spUi->pAddWebButton, &QPushButton::clicked,
            this, &CEditorResourceWidget::SlotAddWebButtonClicked);
    connect(m_pActionBar->m_spUi->pRemoveButton, &QPushButton::clicked,
            this, &CEditorResourceWidget::SlotRemoveButtonClicked);
    connect(m_pActionBar->m_spUi->pTitleCardButton, &QPushButton::clicked,
            this, &CEditorResourceWidget::SlotTitleCardButtonClicked);
    connect(m_pActionBar->m_spUi->pMapButton, &QPushButton::clicked,
            this, &CEditorResourceWidget::SlotMapButtonClicked);

    m_spWebOverlay = std::make_unique<CWebResourceOverlay>(m_pActionBar->m_spUi->pAddWebButton);
    m_spWebOverlay->Hide();
    connect(m_spWebOverlay.get(), &CWebResourceOverlay::SignalResourceSelected,
            this, &CEditorResourceWidget::SlotWebResourceSelected);
  }

  m_bInitialized = true;
}


//----------------------------------------------------------------------------------------
//
void CEditorResourceWidget::LoadProject(tspProject spCurrentProject)
{
  WIDGET_INITIALIZED_GUARD

  m_spCurrentProject = spCurrentProject;

  if (nullptr != m_spCurrentProject)
  {
    // load resource-tree
    CResourceTreeItemModel* pModel = dynamic_cast<CResourceTreeItemModel*>(m_spUi->pResourceTree->model());
    pModel->InitializeModel(m_spCurrentProject);

    // Testing
    //new ModelTest(pModel, this);
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorResourceWidget::UnloadProject()
{
  WIDGET_INITIALIZED_GUARD

  m_spCurrentProject = nullptr;

  CResourceTreeItemModel* pModel = dynamic_cast<CResourceTreeItemModel*>(m_spUi->pResourceTree->model());
  pModel->DeInitializeModel();
}

//----------------------------------------------------------------------------------------
//
void CEditorResourceWidget::on_pFilterLineEdit_editingFinished()
{
  WIDGET_INITIALIZED_GUARD
  // TODO: implement properly
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
  QString sFileName = QFileDialog::getOpenFileName(this,
      tr("Add File"), sCurrentFolder,
      sFormatSelection.arg(sImageFormats).arg(sVideoFormats).arg(sAudioFormats).arg(sOtherFormats));

  // add file to respective category
  QFileInfo info(sFileName);
  m_spCurrentProject->m_rwLock.lockForRead();
  const QString sName = m_spCurrentProject->m_sName;
  m_spCurrentProject->m_rwLock.unlock();

  QDir projectDir(m_spSettings->ContentFolder() + "/" + sName);
  if (!info.canonicalFilePath().contains(projectDir.absolutePath()))
  {
    qWarning() << "File is not in subfolder of Project.";
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
      }
      else if (videoFormatsList.contains(sEnding))
      {
        spDbManager->AddResource(m_spCurrentProject, url, EResourceType::eMovie);
      }
      else if (audioFormatsList.contains(sEnding))
      {
        spDbManager->AddResource(m_spCurrentProject, url, EResourceType::eSound);
      }
      else if (otherFormatsList.contains(sEnding))
      {
        spDbManager->AddResource(m_spCurrentProject, url, EResourceType::eOther);
      }
    }
  }
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

  CResourceTreeItemModel* pModel =
      dynamic_cast<CResourceTreeItemModel*>(m_spUi->pResourceTree->model());

  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager && nullptr != pModel)
  {
    QModelIndexList indexes = m_spUi->pResourceTree->selectionModel()->selectedIndexes();
    m_spUi->pResourceTree->selectionModel()->clearSelection();
    foreach (QModelIndex index, indexes)
    {
      if (pModel->IsResourceType(index))
      {
        const QString sName = pModel->data(index, Qt::DisplayRole).toString();
        spDbManager->RemoveResource(m_spCurrentProject, sName);

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

  CResourceTreeItemModel* pModel =
      dynamic_cast<CResourceTreeItemModel*>(m_spUi->pResourceTree->model());

  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager && nullptr != pModel)
  {
    QModelIndexList indexes = m_spUi->pResourceTree->selectionModel()->selectedIndexes();
    m_spUi->pResourceTree->selectionModel()->clearSelection();
    foreach (QModelIndex index, indexes)
    {
      if (pModel->IsResourceType(index))
      {
        const QString sName = pModel->data(index, Qt::DisplayRole).toString();
        tspResource spResource = spDbManager->FindResource(m_spCurrentProject, sName);
        if (nullptr != spResource)
        {
          QWriteLocker locker(&m_spCurrentProject->m_rwLock);
          m_spCurrentProject->m_sTitleCard = sName;
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

  CResourceTreeItemModel* pModel =
      dynamic_cast<CResourceTreeItemModel*>(m_spUi->pResourceTree->model());

  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager && nullptr != pModel)
  {
    QModelIndexList indexes = m_spUi->pResourceTree->selectionModel()->selectedIndexes();
    m_spUi->pResourceTree->selectionModel()->clearSelection();
    foreach (QModelIndex index, indexes)
    {
      if (pModel->IsResourceType(index))
      {
        const QString sName = pModel->data(index, Qt::DisplayRole).toString();
        tspResource spResource = spDbManager->FindResource(m_spCurrentProject, sName);
        if (nullptr != spResource)
        {
          QWriteLocker locker(&m_spCurrentProject->m_rwLock);
          m_spCurrentProject->m_sMap = sName;
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

  CResourceTreeItemModel* pModel =
      dynamic_cast<CResourceTreeItemModel*>(m_spUi->pResourceTree->model());

  if (nullptr != pModel)
  {
    const QString sName =
        pModel->data(current, Qt::DisplayRole, resource_item::c_iColumnName).toString();
    auto spDbManager = m_wpDbManager.lock();
    if (nullptr != spDbManager)
    {
      //m_spUi->pResourceDisplay->UnloadResource();
      //m_spUi->pResourceDisplay->LoadResource(spDbManager->FindResource(m_spCurrentProject, sName));
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
        }
      }
      else if (videoFormatsList.contains(sFormat))
      {
        // TODO: check video
        spDbManager->AddResource(m_spCurrentProject, url, EResourceType::eMovie);
      }
    }
  }
}
