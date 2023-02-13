#include "EditorResourceWidget.h"
#include "Application.h"
#include "Editor/EditorActionBar.h"
#include "Editor/EditorModel.h"
#include "Editor/EditorWidgetTypes.h"
#include "Editor/Resources/CommandAddResource.h"
#include "Editor/Resources/CommandChangeCurrentResource.h"
#include "Editor/Resources/CommandChangeFilter.h"
#include "Editor/Resources/CommandChangeSource.h"
#include "Editor/Resources/CommandChangeTitleCard.h"
#include "Editor/Resources/CommandRemoveResource.h"
#include "Editor/Resources/ResourceTreeItem.h"
#include "Editor/Resources/ResourceTreeItemModel.h"
#include "Editor/Resources/ResourceTreeItemSortFilterProxyModel.h"
#include "Editor/Resources/WebResourceOverlay.h"
#include "Editor/Tutorial/ResourceTutorialStateSwitchHandler.h"
#include "Systems/DatabaseManager.h"
#include "Systems/HelpFactory.h"
#include "Systems/Project.h"
#include "Utils/UndoRedoFilter.h"
#include "Widgets/HelpOverlay.h"
#include "Widgets/ResourceDisplayWidget.h"

// Testing
//#include <QtTest/QAbstractItemModelTester>

#include <QDebug>
#include <QDomDocument>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QMimeData>
#include <QNetworkAccessManager>
#include <QSortFilterProxyModel>
#include <QStyle>
#include <QUndoStack>

#include <cassert>

DECLARE_EDITORWIDGET(CEditorResourceWidget, EEditorWidget::eResourceWidget)

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
  connect(m_spUi->pResourceModelView, &CResourceModelView::SignalResourceSelected,
          this, &CEditorResourceWidget::SlotViewResourceSelected);
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

  m_spUi->pResourceModelView->Initialize(UndoStack(), ResourceTreeModel());

  m_spWebOverlay->Hide();

  setAcceptDrops(true);

  m_spUi->pFilter->SetFilterUndo(true);

  // help
  auto wpHelpFactory = CApplication::Instance()->System<CHelpFactory>().lock();
  if (nullptr != wpHelpFactory)
  {
    m_spUi->pResourceModelView->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sResourceTreeHelpId);
    wpHelpFactory->RegisterHelp(c_sResourceTreeHelpId, ":/resources/help/editor/resources/resources_tree_help.html");
    m_spUi->pFilter->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sSearchBarHelpId);
    wpHelpFactory->RegisterHelp(c_sSearchBarHelpId, ":/resources/help/editor/resources/filter_widget_help.html");

    wpHelpFactory->RegisterHelp(c_sResourceHelpId, ":/resources/help/editor/resources/resource_help.html");
  }

  m_spUi->splitter->setSizes({height() *5/7, height() * 2/7});

  m_bInitialized = true;
}

//----------------------------------------------------------------------------------------
//
void CEditorResourceWidget::LoadProject(tspProject spCurrentProject)
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr != m_spCurrentProject)
  {
    assert(false);
    qWarning() << "Old Project was not unloaded before loading project.";
  }

  m_spCurrentProject = spCurrentProject;
  m_spUi->pResourceModelView->ProjectLoaded(m_spCurrentProject,
                                            EditorModel()->IsReadOnly());

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

  m_spUi->pResourceModelView->ProjectUnloaded();

  SetLoaded(false);
}

//----------------------------------------------------------------------------------------
//
void CEditorResourceWidget::OnActionBarAboutToChange()
{
  if (nullptr != ActionBar())
  {
    disconnect(ActionBar()->m_spUi->TreeViewButton, &QPushButton::clicked,
            this, &CEditorResourceWidget::SlotChangeViewButtonClicked);
    disconnect(ActionBar()->m_spUi->CdUpButton, &QPushButton::clicked,
            this, &CEditorResourceWidget::SlotCdUpClicked);
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

    connect(ActionBar()->m_spUi->TreeViewButton, &QPushButton::clicked,
            this, &CEditorResourceWidget::SlotChangeViewButtonClicked);
    connect(ActionBar()->m_spUi->CdUpButton, &QPushButton::clicked,
            this, &CEditorResourceWidget::SlotCdUpClicked);
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

    HandleResize(true);
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
void CEditorResourceWidget::resizeEvent(QResizeEvent* pEvent)
{
  CEditorWidgetBase::resizeEvent(pEvent);
  HandleResize(false);
}

//----------------------------------------------------------------------------------------
//
void CEditorResourceWidget::on_pFilter_SignalFilterChanged(const QString& sText)
{
  WIDGET_INITIALIZED_GUARD
  QPointer<CResourceTreeItemSortFilterProxyModel> pProxyModel =
      m_spUi->pResourceModelView->Proxy();

  if (nullptr != pProxyModel)
  {
    Q_UNUSED(sText)
    UndoStack()->push(new CCommandChangeFilter(pProxyModel, m_spUi->pFilter));
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

  QString sCurrentFolder = PhysicalProjectPath(m_spCurrentProject);
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
void CEditorResourceWidget::SlotChangeViewButtonClicked()
{
  WIDGET_INITIALIZED_GUARD

  // do nothing in case that both views are visible, since this shouldn't get triggered in
  // that case
  if (m_spUi->pResourceModelView->View() == CResourceModelView::eExplorer)
  {
    m_spUi->pResourceModelView->SetView(CResourceModelView::eTree);
    ActionBar()->m_spUi->TreeViewButton->setObjectName(QStringLiteral("ExplorerViewButton"));
    ActionBar()->m_spUi->TreeViewButton->setToolTip(QStringLiteral("Show resources in a file explorer"));
    ActionBar()->m_spUi->TreeViewButton->style()->unpolish(ActionBar()->m_spUi->TreeViewButton);
    ActionBar()->m_spUi->TreeViewButton->style()->polish(ActionBar()->m_spUi->TreeViewButton);
  }
  else if (m_spUi->pResourceModelView->View() == CResourceModelView::eTree)
  {
    m_spUi->pResourceModelView->SetView(CResourceModelView::eExplorer);
    ActionBar()->m_spUi->TreeViewButton->setObjectName(QStringLiteral("TreeViewButton"));
    ActionBar()->m_spUi->TreeViewButton->setToolTip(QStringLiteral("Show resources as a tree"));
    ActionBar()->m_spUi->TreeViewButton->style()->unpolish(ActionBar()->m_spUi->TreeViewButton);
    ActionBar()->m_spUi->TreeViewButton->style()->polish(ActionBar()->m_spUi->TreeViewButton);
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorResourceWidget::SlotCdUpClicked()
{
  WIDGET_INITIALIZED_GUARD

  m_spUi->pResourceModelView->CdUp();
}

//----------------------------------------------------------------------------------------
//
void CEditorResourceWidget::SlotRemoveButtonClicked()
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }

  UndoStack()->push(new CCommandRemoveResource(m_spCurrentProject,
                                               m_spUi->pResourceModelView->SelectedResources()));
  emit SignalProjectEdited();
}

//----------------------------------------------------------------------------------------
//
void CEditorResourceWidget::SlotSetSourceButtonClicked()
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }

  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    for (QString sName : m_spUi->pResourceModelView->SelectedResources())
    {
      m_spSourceOverlay->setProperty(c_sProperty, sName);
      m_spSourceOverlay->Show();

      // only interrested in first item which is the actual item we need
      break;
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorResourceWidget::SlotTitleCardButtonClicked()
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }

  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    for (QString sName : m_spUi->pResourceModelView->SelectedResources())
    {
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

//----------------------------------------------------------------------------------------
//
void CEditorResourceWidget::SlotMapButtonClicked()
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }

  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    for (QString sName : m_spUi->pResourceModelView->SelectedResources())
    {
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

//----------------------------------------------------------------------------------------
//
void CEditorResourceWidget::SlotViewResourceSelected(const QString& sResource)
{
  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    tspResource spResource = spDbManager->FindResourceInProject(m_spCurrentProject, sResource);
    m_spUi->pResourceDisplayWidget->UnloadResource();
    m_spUi->pResourceDisplayWidget->LoadResource(spResource);
  }

  emit SignalResourceSelected(sResource);
}

//----------------------------------------------------------------------------------------
//
void CEditorResourceWidget::HandleResize(bool bForceUpdate)
{
  bool bLandscape = width() > height();
  if (nullptr != ActionBar() && ActionBar()->CurrentActionBar() == EEditorWidget::eResourceWidget)
  {
    bool bOldLandScape = m_spUi->pResourceModelView->Landscape();
    if (bOldLandScape != bLandscape || bForceUpdate)
    {
      ActionBar()->m_spUi->TreeViewButton->setObjectName(QStringLiteral("ExplorerViewButton"));
      ActionBar()->m_spUi->TreeViewButton->setToolTip(QStringLiteral("Show resources in a file explorer"));
    }

    ActionBar()->m_spUi->TreeViewButton->setVisible(!bLandscape);
    ActionBar()->m_spUi->CdUpButton->setVisible(!bLandscape);

    if (bOldLandScape != bLandscape || bForceUpdate)
    {
      m_spUi->pResourceModelView->SetView(bLandscape ? CResourceModelView::eBoth :
                                                       CResourceModelView::eTree);
      m_spUi->pResourceModelView->SetLandscape(bLandscape);
    }

    ActionBar()->m_spUi->TreeViewButton->style()->unpolish(ActionBar()->m_spUi->TreeViewButton);
    ActionBar()->m_spUi->TreeViewButton->style()->polish(ActionBar()->m_spUi->TreeViewButton);

    m_spUi->splitter->setOrientation(bLandscape ? Qt::Orientation::Horizontal :
                                                  Qt::Orientation::Vertical);
  }
}
