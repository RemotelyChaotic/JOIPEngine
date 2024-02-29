#include "EditorCodeWidget.h"
#include "Application.h"

#include "Editor/EditorActionBar.h"
#include "Editor/EditorEditableFileModel.h"
#include "Editor/EditorModel.h"

#include "Editor/Resources/ResourceTreeItemModel.h"

#include "Editor/Script/CodeDisplayWidget.h"
#include "Editor/Script/CommandChangeOpenedScript.h"

#include "Editor/Tutorial/CodeWidgetTutorialStateSwitchHandler.h"

#include "Systems/DatabaseManager.h"
#include "Systems/HelpFactory.h"
#include "Systems/Project.h"
#include "Systems/Scene.h"

#include "Widgets/HelpOverlay.h"

#include <QDebug>
#include <QMenu>
#include <QStandardItemModel>
#include <QUndoStack>

DECLARE_EDITORWIDGET(CEditorCodeWidget, EEditorWidget::eSceneCodeEditorWidget)

namespace {
  const qint32 c_iIndexNoScripts = 0;
  const qint32 c_iIndexScripts   = 1;

  const QString c_sScriptSelectionHelpId =  "Editor/ScriptSelection";
}

//----------------------------------------------------------------------------------------
//
CEditorCodeWidget::CEditorCodeWidget(QWidget* pParent) :
  CEditorDebuggableWidget(pParent),
  m_spUi(std::make_shared<Ui::CEditorCodeWidget>()),
  m_spTutorialStateSwitchHandler(nullptr),
  m_spSettings(CApplication::Instance()->Settings()),
  m_wpDbManager(),
  m_pDummyModel(new QStandardItemModel(this)),
  m_bChangingIndex(false),
  m_sLastCachedScript(QString())
{
  m_spUi->setupUi(this);
  m_spUi->pSceneView->setVisible(false);

  m_pFilteredScriptModel = new CFilteredEditorEditableFileModel(this);

  // set initial splitter sizes
  m_spUi->pCodeSplitter->setSizes(QList<int>() << height()/2 << height()/2);

  connect(m_spUi->pCodeEditorView, &CCodeDisplayWidget::SignalContentsChange,
          this, &CEditorCodeWidget::SlotCodeEditContentsChange);
}

CEditorCodeWidget::~CEditorCodeWidget()
{
}

//----------------------------------------------------------------------------------------
//
void CEditorCodeWidget::Initialize()
{
  m_bInitialized = false;

  // initalize Debugger view
  CEditorDebuggableWidget::Initalize(m_spUi->pSceneView,
                                     std::bind(&CEditorCodeWidget::GetScene, this));
  connect(this, &CEditorDebuggableWidget::SignalDebugStarted,
          this, &CEditorCodeWidget::SlotDebugStarted);
  connect(this, &CEditorDebuggableWidget::SignalExecutionError,
          m_spUi->pCodeEditorView, &CCodeDisplayWidget::SlotExecutionError);

  m_spTutorialStateSwitchHandler =
      std::make_shared<CCodeWidgetTutorialStateSwitchHandler>(this, m_spUi);
  EditorModel()->AddTutorialStateSwitchHandler(m_spTutorialStateSwitchHandler);
  EditableFileModel()->SetReloadFileWithoutQuestion(true);
  m_pFilteredScriptModel->FilterForTypes({SScriptDefinitionData::c_sScriptTypeJs,
                                          SScriptDefinitionData::c_sScriptTypeEos,
                                          SScriptDefinitionData::c_sScriptTypeLua,
                                          SScriptDefinitionData::c_sScriptTypeQml,
                                          SScriptDefinitionData::c_sScriptTypeLayout});

  m_wpDbManager = CApplication::Instance()->System<CDatabaseManager>();

  m_spUi->pCodeEditorView->Initialize(EditorModel(),
                                      ResourceTreeModel(), UndoStack());

  auto wpHelpFactory = CApplication::Instance()->System<CHelpFactory>().lock();
  if (nullptr != wpHelpFactory)
  {
    m_spUi->pResourceComboBox->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sScriptSelectionHelpId);
    wpHelpFactory->RegisterHelp(c_sScriptSelectionHelpId, ":/resources/help/editor/code/scriptselection_combobox_help.html");
  }

  m_spUi->pResourceComboBox->setModel(m_pDummyModel);
  m_pFilteredScriptModel->setSourceModel(m_pDummyModel);

  connect(EditableFileModel(), &CEditorEditableFileModel::SignalFileChangedExternally,
          this, &CEditorCodeWidget::SlotFileChangedExternally);

  m_spUi->pStackedWidget->setCurrentIndex(c_iIndexNoScripts);

  m_bInitialized = true;
}

//----------------------------------------------------------------------------------------
//
void CEditorCodeWidget::LoadProject(tspProject spProject)
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == spProject) { return; }

  auto pModel = EditableFileModel();
  m_pFilteredScriptModel->setSourceModel(pModel);
  m_spUi->pResourceComboBox->setModel(m_pFilteredScriptModel);
  connect(pModel, &CEditorEditableFileModel::rowsInserted,
          this, &CEditorCodeWidget::SlotRowsInserted, Qt::QueuedConnection);
  connect(pModel, &CEditorEditableFileModel::rowsRemoved,
          this, &CEditorCodeWidget::SlotRowsRemoved, Qt::QueuedConnection);

  m_spCurrentProject = spProject;
  m_spUi->pCodeEditorView->LoadProject(spProject);

  if (0 < m_pFilteredScriptModel->rowCount())
  {
    // force cvhange actionBar to not get dangling references
    // in on_pResourceComboBox_currentIndexChanged
    if (nullptr != ActionBar())
    {
      m_spUi->pCodeEditorView->OnActionBarAboutToChange(&ActionBar()->m_spUi);
      m_spUi->pCodeEditorView->OnActionBarChanged(&ActionBar()->m_spUi);
    }
    else
    {
      m_spUi->pCodeEditorView->OnActionBarAboutToChange(nullptr);
      m_spUi->pCodeEditorView->OnActionBarChanged(nullptr);
    }

    on_pResourceComboBox_currentIndexChanged(0);
    m_spUi->pStackedWidget->setCurrentIndex(c_iIndexScripts);
  }
  else
  {
    m_spUi->pStackedWidget->setCurrentIndex(c_iIndexNoScripts);
  }

  SetLoaded(true);
}

//----------------------------------------------------------------------------------------
//
void CEditorCodeWidget::UnloadProjectImpl()
{
  WIDGET_INITIALIZED_GUARD

  m_spUi->pCodeEditorView->UnloadProject();

  m_spCurrentProject = nullptr;

  m_sLastCachedScript = QString();

  auto pModel = EditableFileModel();
  disconnect(pModel, &CEditorEditableFileModel::rowsInserted,
             this, &CEditorCodeWidget::SlotRowsInserted);
  disconnect(pModel, &CEditorEditableFileModel::rowsRemoved,
             this, &CEditorCodeWidget::SlotRowsRemoved);

  m_spUi->pResourceComboBox->setModel(m_pDummyModel);
  m_spUi->pResourceComboBox->clear();
}

//----------------------------------------------------------------------------------------
//
void CEditorCodeWidget::SaveProject()
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }
  m_spCurrentProject->m_rwLock.lockForRead();
  const QString sProjectName = m_spCurrentProject->m_sName;
  m_spCurrentProject->m_rwLock.unlock();

  // save current contents
  auto pScriptItem = EditableFileModel()->CachedFile(
        CachedResourceName(m_spUi->pResourceComboBox->currentIndex()));
  if (nullptr != pScriptItem)
  {
    pScriptItem->m_data = m_spUi->pCodeEditorView->GetCurrentText().toUtf8();
    EditableFileModel()->SetSceneScriptModifiedFlag(pScriptItem->m_sId, pScriptItem->m_bChanged);
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorCodeWidget::OnHidden()
{
  EditableFileModel()->SetReloadFileWithoutQuestion(true);
}

//----------------------------------------------------------------------------------------
//
void CEditorCodeWidget::OnShown()
{
  EditableFileModel()->SetReloadFileWithoutQuestion(false);
  ReloadEditor(m_spUi->pResourceComboBox->currentIndex());
}

//----------------------------------------------------------------------------------------
//
void CEditorCodeWidget::LoadResource(tspResource spResource)
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }

  spResource->m_rwLock.lockForRead();
  const QString sName = spResource->m_sName;
  spResource->m_rwLock.unlock();

  qint32 index = EditableFileModel()->FileIndex(sName);
  if (-1 != index)
  {
    m_spUi->pResourceComboBox->setCurrentIndex(index);
    on_pResourceComboBox_currentIndexChanged(index);
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorCodeWidget::OnActionBarAboutToChange()
{
  if (nullptr != ActionBar())
  {
    disconnect(ActionBar()->m_spUi->DebugButton, &QPushButton::clicked,
            this, &CEditorCodeWidget::SlotDebugStart);
    disconnect(ActionBar()->m_spUi->StopDebugButton, &QPushButton::clicked,
            this, &CEditorCodeWidget::SlotDebugStop);
    disconnect(ActionBar()->m_spUi->AddShowBackgroundCode, &QPushButton::clicked,
            this, &CEditorCodeWidget::SlotShowOverlay);
    disconnect(ActionBar()->m_spUi->AddShowIconCode, &QPushButton::clicked,
            this, &CEditorCodeWidget::SlotShowOverlay);
    disconnect(ActionBar()->m_spUi->AddShowImageCode, &QPushButton::clicked,
            this, &CEditorCodeWidget::SlotShowOverlay);
    disconnect(ActionBar()->m_spUi->AddTextCode, &QPushButton::clicked,
            this, &CEditorCodeWidget::SlotShowOverlay);
    disconnect(ActionBar()->m_spUi->AddMetronomeCode, &QPushButton::clicked,
            this, &CEditorCodeWidget::SlotShowOverlay);
    disconnect(ActionBar()->m_spUi->AddNotificationCode, &QPushButton::clicked,
            this, &CEditorCodeWidget::SlotShowOverlay);
    disconnect(ActionBar()->m_spUi->AddTimerCode, &QPushButton::clicked,
            this, &CEditorCodeWidget::SlotShowOverlay);
    disconnect(ActionBar()->m_spUi->AddThreadCode, &QPushButton::clicked,
            this, &CEditorCodeWidget::SlotShowOverlay);
    disconnect(ActionBar()->m_spUi->AddDeviceCode, &QPushButton::clicked,
            this, &CEditorCodeWidget::SlotShowOverlay);

    ActionBar()->m_spUi->StopDebugButton->setEnabled(true);
    ActionBar()->m_spUi->AddShowBackgroundCode->setEnabled(true);
    ActionBar()->m_spUi->AddShowIconCode->setEnabled(true);
    ActionBar()->m_spUi->AddShowImageCode->setEnabled(true);
    ActionBar()->m_spUi->AddTextCode->setEnabled(true);
    ActionBar()->m_spUi->AddMetronomeCode->setEnabled(true);
    ActionBar()->m_spUi->AddNotificationCode->setEnabled(true);
    ActionBar()->m_spUi->AddTimerCode->setEnabled(true);
    ActionBar()->m_spUi->AddThreadCode->setEnabled(true);
    ActionBar()->m_spUi->AddDeviceCode->setEnabled(true);

    m_spUi->pCodeEditorView->OnActionBarAboutToChange(&ActionBar()->m_spUi);

    UpdateButtons(nullptr, nullptr);
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorCodeWidget::OnActionBarChanged()
{
  if (nullptr != ActionBar())
  {
    ActionBar()->ShowCodeActionBar();

    UpdateButtons(ActionBar()->m_spUi->DebugButton,
                  ActionBar()->m_spUi->StopDebugButton);

    connect(ActionBar()->m_spUi->DebugButton, &QPushButton::clicked,
            this, &CEditorCodeWidget::SlotDebugStart);
    connect(ActionBar()->m_spUi->StopDebugButton, &QPushButton::clicked,
            this, &CEditorCodeWidget::SlotDebugStop);
    connect(ActionBar()->m_spUi->AddShowBackgroundCode, &QPushButton::clicked,
            this, &CEditorCodeWidget::SlotShowOverlay);
    connect(ActionBar()->m_spUi->AddShowIconCode, &QPushButton::clicked,
            this, &CEditorCodeWidget::SlotShowOverlay);
    connect(ActionBar()->m_spUi->AddShowImageCode, &QPushButton::clicked,
            this, &CEditorCodeWidget::SlotShowOverlay);
    connect(ActionBar()->m_spUi->AddTextCode, &QPushButton::clicked,
            this, &CEditorCodeWidget::SlotShowOverlay);
    connect(ActionBar()->m_spUi->AddMetronomeCode, &QPushButton::clicked,
            this, &CEditorCodeWidget::SlotShowOverlay);
    connect(ActionBar()->m_spUi->AddNotificationCode, &QPushButton::clicked,
            this, &CEditorCodeWidget::SlotShowOverlay);
    connect(ActionBar()->m_spUi->AddTimerCode, &QPushButton::clicked,
            this, &CEditorCodeWidget::SlotShowOverlay);
    connect(ActionBar()->m_spUi->AddThreadCode, &QPushButton::clicked,
            this, &CEditorCodeWidget::SlotShowOverlay);
    connect(ActionBar()->m_spUi->AddDeviceCode, &QPushButton::clicked,
            this, &CEditorCodeWidget::SlotShowOverlay);

    if (EditorModel()->IsReadOnly())
    {
      ActionBar()->m_spUi->AddShowBackgroundCode->setEnabled(false);
      ActionBar()->m_spUi->AddShowIconCode->setEnabled(false);
      ActionBar()->m_spUi->AddShowImageCode->setEnabled(false);
      ActionBar()->m_spUi->AddTextCode->setEnabled(false);
      ActionBar()->m_spUi->AddMetronomeCode->setEnabled(false);
      ActionBar()->m_spUi->AddNotificationCode->setEnabled(false);
      ActionBar()->m_spUi->AddTimerCode->setEnabled(false);
      ActionBar()->m_spUi->AddThreadCode->setEnabled(false);
      ActionBar()->m_spUi->AddDeviceCode->setEnabled(false);
    }

    m_spUi->pCodeEditorView->OnActionBarChanged(&ActionBar()->m_spUi);

    if (0 < EditableFileModel()->rowCount())
    {
      auto pScriptItem = EditableFileModel()->CachedFile(
            CachedResourceName(m_spUi->pResourceComboBox->currentIndex()));
      if (nullptr != pScriptItem)
      {
        if (nullptr != ActionBar())
        {
          m_spUi->pCodeEditorView->SetScriptType(pScriptItem->m_sFileType);
        }
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorCodeWidget::on_pResourceComboBox_currentIndexChanged(qint32 iIndex)
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }

  if (CachedResourceName(iIndex) != m_sLastCachedScript)
  {
    UndoStack()->push(new CCommandChangeOpenedScript(m_spUi->pResourceComboBox,
                                                     m_spUi->pCodeEditorView,
                                                     this, std::bind(&CEditorCodeWidget::ReloadEditor, this, std::placeholders::_1),
                                                     &m_bChangingIndex, &m_sLastCachedScript,
                                                     m_sLastCachedScript,
                                                     CachedResourceName(iIndex)));
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorCodeWidget::SlotCodeEditContentsChange(qint32 iPos, qint32 iDel, qint32 iAdd)
{
  Q_UNUSED(iPos)
  WIDGET_INITIALIZED_GUARD

  // nothing changed
  if ((0 == iDel && 0 == iAdd) || m_bChangingIndex) { return; }

  QString sCachedScript = CachedResourceName(
        m_spUi->pResourceComboBox->currentIndex());
  auto pScriptItem = EditableFileModel()->CachedFile(sCachedScript);
  if (nullptr != pScriptItem)
  {
    pScriptItem->m_bChanged = true;
    pScriptItem->m_data = m_spUi->pCodeEditorView->GetCurrentText().toUtf8();
    EditableFileModel()->SetSceneScriptModifiedFlag(pScriptItem->m_sId, true);
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorCodeWidget::SlotDebugStarted()
{
  m_spUi->pCodeEditorView->ResetWidget();
}

//----------------------------------------------------------------------------------------
//
void CEditorCodeWidget::SlotFileChangedExternally(const QString& sName)
{
  qint32 index =
      m_pFilteredScriptModel->mapToSource(
          m_pFilteredScriptModel->index(m_spUi->pResourceComboBox->currentIndex(), 0)).row();
  auto pScriptItem = EditableFileModel()->CachedFile(sName);
  if (index == EditableFileModel()->FileIndex(sName) && nullptr != pScriptItem)
  {
    m_bChangingIndex = true;

    m_spUi->pCodeEditorView->ResetWidget();
    m_spUi->pCodeEditorView->Clear();
    // load new contents
    ActionBar()->m_spUi->DebugButton->setEnabled(!pScriptItem->m_vspScenes.empty());
    m_spUi->pCodeEditorView->SetContent(QString::fromUtf8(pScriptItem->m_data));

    m_spUi->pCodeEditorView->Update();

    m_bChangingIndex = false;
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorCodeWidget::SlotShowOverlay()
{
  WIDGET_INITIALIZED_GUARD
  m_spUi->pCodeEditorView->SlotShowOverlay(dynamic_cast<QWidget*>(sender()));
}

//----------------------------------------------------------------------------------------
//
void CEditorCodeWidget::SlotRowsInserted(const QModelIndex& parent, int iFirst, int iLast)
{
  Q_UNUSED(parent) Q_UNUSED(iFirst) Q_UNUSED(iLast)
  if (0 < EditableFileModel()->rowCount())
  {
    if (m_spUi->pResourceComboBox->currentIndex() < 0)
    {
      on_pResourceComboBox_currentIndexChanged(0);
    }
    m_spUi->pStackedWidget->setCurrentIndex(c_iIndexScripts);
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorCodeWidget::SlotRowsRemoved(const QModelIndex& parent, int iFirst, int iLast)
{
  Q_UNUSED(parent) Q_UNUSED(iFirst) Q_UNUSED(iLast)
  if (0 >= EditableFileModel()->rowCount())
  {
    m_spUi->pStackedWidget->setCurrentIndex(c_iIndexNoScripts);
  }
}

//----------------------------------------------------------------------------------------
//
QString CEditorCodeWidget::CachedResourceName(qint32 iIndex)
{
  return EditableFileModel()->CachedResourceName(
      m_pFilteredScriptModel->mapToSource(
          m_pFilteredScriptModel->index(iIndex, 0)).row());
}

//----------------------------------------------------------------------------------------
//
CEditorCodeWidget::tSceneToDebug CEditorCodeWidget::GetScene()
{
  QStringList vsPossibleScenesToDebug;

  QString sCachedScript = CachedResourceName(
      m_spUi->pResourceComboBox->currentIndex());
  auto pScriptItem = EditableFileModel()->CachedFile(sCachedScript);
  if (nullptr != pScriptItem)
  {
    auto vspScenes = pScriptItem->m_vspScenes;
    if (!vspScenes.empty())
    {
      for (const tspScene& spScene : vspScenes)
      {
        if (nullptr != spScene)
        {
          QReadLocker locker(&spScene->m_rwLock);
          vsPossibleScenesToDebug << spScene->m_sName;
        }
      }
    }
  }

  QString sSceneName = QString();

  if (vsPossibleScenesToDebug.size() == 1)
  {
    sSceneName = vsPossibleScenesToDebug[0];
  }
  else if (vsPossibleScenesToDebug.size() > 1)
  {
    QMenu menu;

    for (const QString& sSceneNameLoc : qAsConst(vsPossibleScenesToDebug))
    {
      QAction* pAction = new QAction(sSceneNameLoc, &menu);
      connect(pAction, &QAction::triggered, pAction,
              [&sSceneName, sSceneNameLoc]() { sSceneName = sSceneNameLoc; });
      menu.addAction(pAction);
    }

    QPointer<CEditorDebuggableWidget> pThis(this);
    menu.exec(ActionBar()->m_spUi->DebugButton->parentWidget()->mapToGlobal(
        ActionBar()->m_spUi->DebugButton->pos()));
    if (nullptr == pThis)
    {
      return nullptr;
    }
  }

  return sSceneName;
}

//----------------------------------------------------------------------------------------
//
void CEditorCodeWidget::ReloadEditor(qint32 iIndex)
{
  m_bChangingIndex = true;

  m_spUi->pCodeEditorView->ResetWidget();
  m_spUi->pCodeEditorView->Clear();

  // load new contents
  m_sLastCachedScript = CachedResourceName(iIndex);
  auto  pScriptItem = EditableFileModel()->CachedFile(m_sLastCachedScript);
  if (nullptr != pScriptItem)
  {
    if (nullptr != ActionBar())
    {
      ActionBar()->m_spUi->DebugButton->setEnabled(!pScriptItem->m_vspScenes.empty());
    }
    m_spUi->pCodeEditorView->SetScriptType(pScriptItem->m_sFileType);
    m_spUi->pCodeEditorView->SetContent(QString::fromUtf8(pScriptItem->m_data));
    m_spUi->pCodeEditorView->SetHighlightDefinition(pScriptItem->m_sHighlightDefinition);
  }
  m_spUi->pCodeEditorView->Update();

  m_bChangingIndex = false;
}
