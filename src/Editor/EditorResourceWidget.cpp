#include "EditorResourceWidget.h"
#include "Application.h"
#include "EditorActionBar.h"
#include "EditorModel.h"
#include "Resources/CommandAddResource.h"
#include "Resources/CommandChangeCurrentResource.h"
#include "Resources/CommandChangeFilter.h"
#include "Resources/CommandChangeSource.h"
#include "Resources/CommandChangeTitleCard.h"
#include "Resources/CommandRemoveResource.h"
#include "Resources/ResourceTreeItem.h"
#include "Resources/ResourceTreeItemModel.h"
#include "Resources/ResourceTreeItemSortFilterProxyModel.h"
#include "Resources/WebResourceOverlay.h"
#include "Systems/DatabaseManager.h"
#include "Systems/HelpFactory.h"
#include "Systems/Project.h"
#include "Tutorial/ResourceTutorialStateSwitchHandler.h"
#include "Utils/UndoRedoFilter.h"
#include "Widgets/HelpOverlay.h"
#include "Widgets/ResourceDisplayWidget.h"

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
#include <QUndoStack>

#include <cassert>

namespace
{
  const QString c_sResourceTreeHelpId =   "Editor/ResourceTree";
  const QString c_sSearchBarHelpId =      "Editor/SearchBar";
  const QString c_sResourceHelpId =       "Editor/Resource/Resource";

  const char c_sProperty[] = "SelectedResource";
}

//----------------------------------------------------------------------------------------
//
CEditorResourceWidget::CEditorResourceWidget(QWidget* pParent) :
  CEditorWidgetBase(pParent),
  m_spSourceOverlay(std::make_unique<CWebResourceOverlay>(this)),
  m_spWebOverlay(std::make_unique<CWebResourceOverlay>(this)),
  m_spNAManager(std::make_unique<QNetworkAccessManager>()),
  m_spUi(std::make_shared<Ui::CEditorResourceWidget>()),
  m_spTutorialStateSwitchHandler(nullptr),
  m_spSettings(CApplication::Instance()->Settings()),
  m_spCurrentProject(nullptr),
  m_pResponse(nullptr)
{
  m_spUi->setupUi(this);
  connect(m_spSourceOverlay.get(), &CWebResourceOverlay::SignalResourceSelected,
          this, &CEditorResourceWidget::SlotWebSourceSelected);
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

  dynamic_cast<CResourceTreeItemSortFilterProxyModel*>(m_spUi->pResourceTree->model())
      ->setSourceModel(nullptr);

  m_spWebOverlay.reset();
  m_spSourceOverlay.reset();
}

//----------------------------------------------------------------------------------------
//
void CEditorResourceWidget::Initialize()
{
  m_bInitialized = false;

  m_spTutorialStateSwitchHandler =
      std::make_shared<CResourceTutorialStateSwitchHandler>(this, m_spUi);
  EditorModel()->AddTutorialStateSwitchHandler(m_spTutorialStateSwitchHandler);

  m_wpDbManager = CApplication::Instance()->System<CDatabaseManager>();

  m_spUi->pResourceTree->header()->setStretchLastSection(true);
  m_spUi->pResourceTree->header()->setSectionResizeMode(QHeaderView::ResizeToContents);

  CResourceTreeItemSortFilterProxyModel* pProxyModel =
      new CResourceTreeItemSortFilterProxyModel(m_spUi->pResourceTree);
  pProxyModel->setSourceModel(ResourceTreeModel());
  m_spUi->pResourceTree->setModel(pProxyModel);

  QItemSelectionModel* pSelectionModel = m_spUi->pResourceTree->selectionModel();
  connect(pSelectionModel, &QItemSelectionModel::currentChanged,
          this, &CEditorResourceWidget::SlotCurrentChanged);

  auto pFilter =
      new CUndoRedoFilter(m_spUi->pResourceTree, nullptr);
  connect(pFilter, &CUndoRedoFilter::UndoTriggered, this, [this]() { UndoStack()->undo(); });
  connect(pFilter, &CUndoRedoFilter::RedoTriggered, this, [this]() { UndoStack()->redo(); });

  m_spWebOverlay->Hide();

  setAcceptDrops(true);

  pProxyModel->sort(resource_item::c_iColumnName, Qt::AscendingOrder);
  pProxyModel->setFilterRegExp(QRegExp(".*", Qt::CaseInsensitive, QRegExp::RegExp));

  m_spUi->pFilter->SetFilterUndo(true);

  // help
  auto wpHelpFactory = CApplication::Instance()->System<CHelpFactory>().lock();
  if (nullptr != wpHelpFactory)
  {
    m_spUi->pResourceTree->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sResourceTreeHelpId);
    wpHelpFactory->RegisterHelp(c_sResourceTreeHelpId, ":/resources/help/editor/resources/resources_tree_help.html");
    m_spUi->pFilter->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sSearchBarHelpId);
    wpHelpFactory->RegisterHelp(c_sSearchBarHelpId, ":/resources/help/editor/resources/filter_widget_help.html");

    wpHelpFactory->RegisterHelp(c_sResourceHelpId, ":/resources/help/editor/resources/resource_help.html");
  }

  m_spUi->splitter->setSizes({height() *4/5, height() / 5});

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
  m_spUi->pResourceTree->setEditTriggers(
        EditorModel()->IsReadOnly() ? QAbstractItemView::NoEditTriggers :
                                      QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);

  SetLoaded(true);
}

//----------------------------------------------------------------------------------------
//
void CEditorResourceWidget::UnloadProject()
{
  WIDGET_INITIALIZED_GUARD

  m_spCurrentProject = nullptr;

  m_spUi->pResourceDisplayWidget->UnloadResource();
  m_spUi->pResourceDisplayWidget->UnloadPlayer();
  m_spSourceOverlay->Hide();
  m_spWebOverlay->Hide();

  m_spUi->pResourceTree->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);

  SetLoaded(false);
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
    disconnect(ActionBar()->m_spUi->SourceButton, &QPushButton::clicked,
            this, &CEditorResourceWidget::SlotSetSourceButtonClicked);
    disconnect(ActionBar()->m_spUi->TitleCardButton, &QPushButton::clicked,
            this, &CEditorResourceWidget::SlotTitleCardButtonClicked);
    disconnect(ActionBar()->m_spUi->MapButton, &QPushButton::clicked,
            this, &CEditorResourceWidget::SlotMapButtonClicked);

    // read only mode reset
    ActionBar()->m_spUi->AddResourceButton->setEnabled(true);
    ActionBar()->m_spUi->AddWebResourceButton->setEnabled(true);
    ActionBar()->m_spUi->RemoveResourceButton->setEnabled(true);
    ActionBar()->m_spUi->SourceButton->setEnabled(true);
    ActionBar()->m_spUi->TitleCardButton->setEnabled(true);
    ActionBar()->m_spUi->MapButton->setEnabled(true);
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
    if (!m_spSettings->Offline())
    {
      ActionBar()->m_spUi->AddWebResourceButton->show();
      connect(ActionBar()->m_spUi->AddWebResourceButton, &QPushButton::clicked,
              this, &CEditorResourceWidget::SlotAddWebButtonClicked);
    }
    else
    {
      ActionBar()->m_spUi->AddWebResourceButton->hide();
    }
    connect(ActionBar()->m_spUi->RemoveResourceButton, &QPushButton::clicked,
            this, &CEditorResourceWidget::SlotRemoveButtonClicked);
    connect(ActionBar()->m_spUi->SourceButton, &QPushButton::clicked,
            this, &CEditorResourceWidget::SlotSetSourceButtonClicked);
    connect(ActionBar()->m_spUi->TitleCardButton, &QPushButton::clicked,
            this, &CEditorResourceWidget::SlotTitleCardButtonClicked);
    connect(ActionBar()->m_spUi->MapButton, &QPushButton::clicked,
            this, &CEditorResourceWidget::SlotMapButtonClicked);

    // read only mode -> grey out some options
    if (EditorModel()->IsReadOnly())
    {
      ActionBar()->m_spUi->AddResourceButton->setEnabled(false);
      ActionBar()->m_spUi->AddWebResourceButton->setEnabled(false);
      ActionBar()->m_spUi->RemoveResourceButton->setEnabled(false);
      ActionBar()->m_spUi->SourceButton->setEnabled(false);
      ActionBar()->m_spUi->TitleCardButton->setEnabled(false);
      ActionBar()->m_spUi->MapButton->setEnabled(false);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorResourceWidget::dragEnterEvent(QDragEnterEvent* pEvent)
{
  if (EditorModel()->IsReadOnly()) { return; }

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
  if (EditorModel()->IsReadOnly()) { return; }

  const QMimeData* pMimeData = pEvent->mimeData();

  // check for our needed mime type, here a file or a list of files
  if (pMimeData->hasUrls())
  {
    QStringList vsFileNames;
    QList<QUrl> vUrlList = pMimeData->urls();
    std::map<QUrl, QByteArray> vsFiles;
    for (const QUrl& sPath : qAsConst(vUrlList)) { vsFiles.insert({sPath, QByteArray()}); }
    UndoStack()->push(new CCommandAddResource(m_spCurrentProject, this, vsFiles));
    emit SignalProjectEdited();
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorResourceWidget::on_pFilter_SignalFilterChanged(const QString& sText)
{
  WIDGET_INITIALIZED_GUARD
  CResourceTreeItemSortFilterProxyModel* pProxyModel =
      dynamic_cast<CResourceTreeItemSortFilterProxyModel*>(m_spUi->pResourceTree->model());

  Q_UNUSED(sText)
  UndoStack()->push(new CCommandChangeFilter(pProxyModel, m_spUi->pFilter));
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

  QString sCurrentFolder = CApplication::Instance()->Settings()->ContentFolder();
  QStringList  vsFileNames = QFileDialog::getOpenFileNames(this,
      tr("Add File"), sCurrentFolder, SResourceFormats::JoinedFormatsForFilePicker());

  std::map<QUrl, QByteArray> vsFiles;
  for (const QString& sPath : qAsConst(vsFileNames))
  {
    vsFiles.insert({ QUrl::fromLocalFile(sPath), QByteArray()});
  }
  UndoStack()->push(new CCommandAddResource(m_spCurrentProject, this, vsFiles));
  emit SignalProjectEdited();
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
    QStringList vsRemovedResources;
    foreach (QModelIndex index, indexes)
    {
      if (pModel->IsResourceType(pProxyModel->mapToSource(index)))
      {
        const QString sName = pModel->data(pProxyModel->mapToSource(index), Qt::DisplayRole).toString();
        vsRemovedResources << sName;

        // only interrested in first item which is the actual item we need
        break;
      }
    }

    UndoStack()->push(new CCommandRemoveResource(m_spCurrentProject, vsRemovedResources));
    emit SignalProjectEdited();
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorResourceWidget::SlotSetSourceButtonClicked()
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

        m_spSourceOverlay->setProperty(c_sProperty, sName);
        m_spSourceOverlay->Show();

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
        QWriteLocker locker(&m_spCurrentProject->m_rwLock);
        const QString sOldTitleCard = m_spCurrentProject->m_sTitleCard;
        locker.unlock();

        QPointer<CEditorResourceWidget> pThis(this);
        UndoStack()->push(
              new CCommandChangeTitleCard(m_spCurrentProject, sOldTitleCard, sName,
                                          [pThis]() {
                if (nullptr != pThis)
                {
                  emit pThis->SignalProjectEdited();
                }
              }));

        // only interrested in first item which is the actual item we need
        break;
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
        tspResource spResource = spDbManager->FindResourceInProject(m_spCurrentProject, sName);
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
    const QString sPrevious =
        pModel->data(pProxyModel->mapToSource(previous), Qt::DisplayRole, resource_item::c_iColumnName).toString();
    const QString sName =
        pModel->data(pProxyModel->mapToSource(current), Qt::DisplayRole, resource_item::c_iColumnName).toString();

    QPointer<CEditorResourceWidget> pThis(this);
    UndoStack()->push(
          new CCommandChangeCurrentResource(m_spCurrentProject,
                                            m_spUi->pResourceDisplayWidget,
                                            m_spUi->pResourceTree->selectionModel(),
                                            pProxyModel, sPrevious, sName,
                                            [this, pThis](const QString& sName){
      if (nullptr != pThis)
      {
        emit SignalResourceSelected(sName);
      }
    }));
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
void CEditorResourceWidget::SlotWebSourceSelected(const QString& sResource)
{
  QString sName = m_spSourceOverlay->property(c_sProperty).toString();
  m_spSourceOverlay->Hide();

  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    tspResource spResource = spDbManager->FindResourceInProject(m_spCurrentProject, sName);
    if (nullptr != spResource)
    {
      QWriteLocker locker(&spResource->m_rwLock);
      QPointer<CEditorResourceWidget> pThis(this);
      UndoStack()->push(new CCommandChangeSource(m_spCurrentProject,
                                                 sName,
                                                 spResource->m_sSource,
                                                 sResource,
                                                 [pThis]() {
        if (nullptr != pThis)
        {
          emit pThis->SignalProjectEdited();
        }
      }));
    }
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

    UndoStack()->push(new CCommandAddResource(m_spCurrentProject, this, {{url, arr}}));
  }
}
