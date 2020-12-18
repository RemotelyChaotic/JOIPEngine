#include "EditorCodeWidget.h"
#include "Application.h"
#include "EditorActionBar.h"
#include "EditorModel.h"
#include "Player/SceneMainScreen.h"
#include "Script/BackgroundSnippetOverlay.h"
#include "Script/IconSnippetOverlay.h"
#include "Script/MetronomeSnippetOverlay.h"
#include "Script/ResourceSnippetOverlay.h"
#include "Script/ScriptEditorModel.h"
#include "Script/ScriptHighlighter.h"
#include "Script/TextSnippetOverlay.h"
#include "Script/TimerSnippetOverlay.h"
#include "Script/ThreadSnippetOverlay.h"
#include "Systems/DatabaseManager.h"
#include "Systems/HelpFactory.h"
#include "Systems/Project.h"
#include "Systems/Scene.h"
#include "Systems/ScriptRunner.h"
#include "Systems/Script/ScriptRunnerSignalEmiter.h"
#include "Tutorial/CodeWidgetTutorialStateSwitchHandler.h"
#include "Widgets/HelpOverlay.h"

#include <QDebug>
#include <QStandardItemModel>

namespace {
  const qint32 c_iIndexNoScripts = 0;
  const qint32 c_iIndexScripts   = 1;

  const QString c_sScriptSelectionHelpId =  "Editor/ScriptSelection";
  const QString c_sSciptEditorHelpId =      "Editor/SciptEditor";
}

//----------------------------------------------------------------------------------------
//
CEditorCodeWidget::CEditorCodeWidget(QWidget* pParent) :
  CEditorWidgetBase(pParent),
  m_spBackgroundSnippetOverlay(std::make_unique<CBackgroundSnippetOverlay>(this)),
  m_spIconSnippetOverlay(std::make_unique<CIconSnippetOverlay>(this)),
  m_spMetronomeSnippetOverlay(std::make_unique<CMetronomeSnippetOverlay>(this)),
  m_spResourceSnippetOverlay(std::make_unique<CResourceSnippetOverlay>(this)),
  m_spTextSnippetOverlay(std::make_unique<CTextSnippetOverlay>(this)),
  m_spTimerSnippetOverlay(std::make_unique<CTimerSnippetOverlay>(this)),
  m_spThreadSnippetOverlay(std::make_unique<CThreadSnippetOverlay>(this)),
  m_spUi(std::make_shared<Ui::CEditorCodeWidget>()),
  m_spTutorialStateSwitchHandler(nullptr),
  m_spSettings(CApplication::Instance()->Settings()),
  m_spCurrentProject(nullptr),
  m_wpDbManager(),
  m_wpScriptRunner(),
  m_pDummyModel(new QStandardItemModel(this)),
  m_debugFinishedConnection(),
  m_bDebugging(false),
  m_bChangingIndex(false),
  m_sLastCachedScript(QString())
{
  m_spUi->setupUi(this);
  m_spUi->pSceneView->setVisible(false);

  connect(m_spUi->pCodeEdit->document(), &QTextDocument::contentsChange,
          this, &CEditorCodeWidget::SlotCodeEditContentsChange);
}

CEditorCodeWidget::~CEditorCodeWidget()
{
  m_spThreadSnippetOverlay.reset();
  m_spTimerSnippetOverlay.reset();
  m_spTextSnippetOverlay.reset();
  m_spResourceSnippetOverlay.reset();
  m_spMetronomeSnippetOverlay.reset();
  m_spIconSnippetOverlay.reset();
  m_spBackgroundSnippetOverlay.reset();
}

//----------------------------------------------------------------------------------------
//
void CEditorCodeWidget::Initialize()
{
  m_bInitialized = false;

  m_spTutorialStateSwitchHandler =
      std::make_shared<CCodeWidgetTutorialStateSwitchHandler>(this, m_spUi);
  EditorModel()->AddTutorialStateSwitchHandler(m_spTutorialStateSwitchHandler);
  ScriptEditorModel()->SetReloadFileWithoutQuestion(true);

  m_wpDbManager = CApplication::Instance()->System<CDatabaseManager>();

  m_spBackgroundSnippetOverlay->Initialize(ResourceTreeModel());
  m_spIconSnippetOverlay->Initialize(ResourceTreeModel());
  m_spMetronomeSnippetOverlay->Initialize(ResourceTreeModel());
  m_spResourceSnippetOverlay->Initialize(ResourceTreeModel());
  m_spTextSnippetOverlay->Initialize(ResourceTreeModel());

  m_spBackgroundSnippetOverlay->Hide();
  m_spIconSnippetOverlay->Hide();
  m_spMetronomeSnippetOverlay->Hide();
  m_spResourceSnippetOverlay->Hide();
  m_spTextSnippetOverlay->Hide();
  m_spTimerSnippetOverlay->Hide();
  m_spThreadSnippetOverlay->Hide();

  connect(m_spBackgroundSnippetOverlay.get(), &CBackgroundSnippetOverlay::SignalBackgroundCode,
          this, &CEditorCodeWidget::SlotInsertGeneratedCode);
  connect(m_spIconSnippetOverlay.get(), &CIconSnippetOverlay::SignalIconCode,
          this, &CEditorCodeWidget::SlotInsertGeneratedCode);
  connect(m_spMetronomeSnippetOverlay.get(), &CMetronomeSnippetOverlay::SignalMetronomeSnippetCode,
          this, &CEditorCodeWidget::SlotInsertGeneratedCode);
  connect(m_spResourceSnippetOverlay.get(), &CResourceSnippetOverlay::SignalResourceCode,
          this, &CEditorCodeWidget::SlotInsertGeneratedCode);
  connect(m_spTextSnippetOverlay.get(), &CTextSnippetOverlay::SignalTextSnippetCode,
          this, &CEditorCodeWidget::SlotInsertGeneratedCode);
  connect(m_spTimerSnippetOverlay.get(), &CTimerSnippetOverlay::SignalTimerCode,
          this, &CEditorCodeWidget::SlotInsertGeneratedCode);
  connect(m_spThreadSnippetOverlay.get(), &CThreadSnippetOverlay::SignalThreadCode,
          this, &CEditorCodeWidget::SlotInsertGeneratedCode);

  auto wpHelpFactory = CApplication::Instance()->System<CHelpFactory>().lock();
  if (nullptr != wpHelpFactory)
  {
    m_spUi->pResourceComboBox->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sScriptSelectionHelpId);
    wpHelpFactory->RegisterHelp(c_sScriptSelectionHelpId, ":/resources/help/editor/code/scriptselection_combobox_help.html");
    m_spUi->pCodeEdit->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sSciptEditorHelpId);
    wpHelpFactory->RegisterHelp(c_sSciptEditorHelpId, ":/resources/help/editor/code/script_editor_help.html");
  }

  m_spUi->pResourceComboBox->setModel(m_pDummyModel);

  connect(ScriptEditorModel(), &CScriptEditorModel::SignalFileChangedExternally,
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

  auto pModel = ScriptEditorModel();
  m_spUi->pResourceComboBox->setModel(pModel);
  connect(pModel, &CScriptEditorModel::rowsInserted,
          this, &CEditorCodeWidget::SlotRowsInserted, Qt::QueuedConnection);
  connect(pModel, &CScriptEditorModel::rowsRemoved,
          this, &CEditorCodeWidget::SlotRowsRemoved, Qt::QueuedConnection);

  m_spCurrentProject = spProject;
  m_spResourceSnippetOverlay->LoadProject(m_spCurrentProject);
  m_spMetronomeSnippetOverlay->LoadProject(m_spCurrentProject);
  m_spTextSnippetOverlay->LoadProject(m_spCurrentProject);

  if (0 < ScriptEditorModel()->rowCount())
  {
    on_pResourceComboBox_currentIndexChanged(0);
    m_spUi->pStackedWidget->setCurrentIndex(c_iIndexScripts);
  }
  else
  {
    m_spUi->pStackedWidget->setCurrentIndex(c_iIndexNoScripts);
  }

  m_spUi->pCodeEdit->setReadOnly(EditorModel()->IsReadOnly());

  SetLoaded(true);
}

//----------------------------------------------------------------------------------------
//
void CEditorCodeWidget::UnloadProject()
{
  WIDGET_INITIALIZED_GUARD

  if (m_bDebugging)
  {
    m_debugFinishedConnection =
        connect(this, &CEditorCodeWidget::SignalDebugFinished, this,
                [this](){
      disconnect(m_debugFinishedConnection);
      SetLoaded(false);
    }, Qt::QueuedConnection);

    SlotDebugStop();
  }

  m_spUi->pCodeEdit->blockSignals(true);
  m_spUi->pCodeEdit->document()->blockSignals(true);
  m_spUi->pCodeEdit->ResetWidget();
  m_spUi->pCodeEdit->clear();
  m_spUi->pCodeEdit->document()->blockSignals(false);
  m_spUi->pCodeEdit->blockSignals(false);
  m_spUi->pCodeEdit->setReadOnly(false);

  m_spResourceSnippetOverlay->UnloadProject();
  m_spMetronomeSnippetOverlay->UnloadProject();
  m_spTextSnippetOverlay->UnloadProject();

  m_spCurrentProject = nullptr;

  m_sLastCachedScript = QString();

  auto pModel = ScriptEditorModel();
  disconnect(pModel, &CScriptEditorModel::rowsInserted,
             this, &CEditorCodeWidget::SlotRowsInserted);
  disconnect(pModel, &CScriptEditorModel::rowsRemoved,
             this, &CEditorCodeWidget::SlotRowsRemoved);

  m_spUi->pResourceComboBox->setModel(m_pDummyModel);
  m_spUi->pResourceComboBox->clear();

  m_spBackgroundSnippetOverlay->Hide();
  m_spIconSnippetOverlay->Hide();
  m_spMetronomeSnippetOverlay->Hide();
  m_spResourceSnippetOverlay->Hide();
  m_spTextSnippetOverlay->Hide();
  m_spTimerSnippetOverlay->Hide();
  m_spThreadSnippetOverlay->Hide();

  if (!m_bDebugging)
  {
    SetLoaded(false);
  }
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
  auto pScriptItem = ScriptEditorModel()->CachedScript(
        ScriptEditorModel()->CachedScriptName(m_spUi->pResourceComboBox->currentIndex()));
  if (nullptr != pScriptItem)
  {
    pScriptItem->m_data = m_spUi->pCodeEdit->toPlainText().toUtf8();
    ScriptEditorModel()->SetSceneScriptModifiedFlag(pScriptItem->m_sId, pScriptItem->m_bChanged);
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorCodeWidget::OnHidden()
{
  ScriptEditorModel()->SetReloadFileWithoutQuestion(true);
}

//----------------------------------------------------------------------------------------
//
void CEditorCodeWidget::OnShown()
{
  ScriptEditorModel()->SetReloadFileWithoutQuestion(false);
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

  qint32 index = ScriptEditorModel()->ScriptIndex(sName);
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
            m_spBackgroundSnippetOverlay.get(), &CBackgroundSnippetOverlay::Show);
    disconnect(ActionBar()->m_spUi->AddShowIconCode, &QPushButton::clicked,
            m_spIconSnippetOverlay.get(), &CIconSnippetOverlay::Show);
    disconnect(ActionBar()->m_spUi->AddShowImageCode, &QPushButton::clicked,
            m_spResourceSnippetOverlay.get(), &CResourceSnippetOverlay::Show);
    disconnect(ActionBar()->m_spUi->AddTextCode, &QPushButton::clicked,
            m_spTextSnippetOverlay.get(), &CTextSnippetOverlay::Show);
    disconnect(ActionBar()->m_spUi->AddMetronomeCode, &QPushButton::clicked,
            m_spMetronomeSnippetOverlay.get(), &CMetronomeSnippetOverlay::Show);
    disconnect(ActionBar()->m_spUi->AddTimerCode, &QPushButton::clicked,
            m_spTimerSnippetOverlay.get(), &CTimerSnippetOverlay::Show);
    disconnect(ActionBar()->m_spUi->AddThreadCode, &QPushButton::clicked,
            m_spThreadSnippetOverlay.get(), &CThreadSnippetOverlay::Show);

    ActionBar()->m_spUi->StopDebugButton->setEnabled(true);
    ActionBar()->m_spUi->AddShowBackgroundCode->setEnabled(true);
    ActionBar()->m_spUi->AddShowIconCode->setEnabled(true);
    ActionBar()->m_spUi->AddShowImageCode->setEnabled(true);
    ActionBar()->m_spUi->AddTextCode->setEnabled(true);
    ActionBar()->m_spUi->AddMetronomeCode->setEnabled(true);
    ActionBar()->m_spUi->AddTimerCode->setEnabled(true);
    ActionBar()->m_spUi->AddThreadCode->setEnabled(true);
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorCodeWidget::OnActionBarChanged()
{
  if (nullptr != ActionBar())
  {
    ActionBar()->ShowCodeActionBar();
    connect(ActionBar()->m_spUi->DebugButton, &QPushButton::clicked,
            this, &CEditorCodeWidget::SlotDebugStart);
    connect(ActionBar()->m_spUi->StopDebugButton, &QPushButton::clicked,
            this, &CEditorCodeWidget::SlotDebugStop);
    connect(ActionBar()->m_spUi->AddShowBackgroundCode, &QPushButton::clicked,
            m_spBackgroundSnippetOverlay.get(), &CBackgroundSnippetOverlay::Show);
    connect(ActionBar()->m_spUi->AddShowIconCode, &QPushButton::clicked,
            m_spIconSnippetOverlay.get(), &CIconSnippetOverlay::Show);
    connect(ActionBar()->m_spUi->AddShowImageCode, &QPushButton::clicked,
            m_spResourceSnippetOverlay.get(), &CResourceSnippetOverlay::Show);
    connect(ActionBar()->m_spUi->AddTextCode, &QPushButton::clicked,
            m_spTextSnippetOverlay.get(), &CTextSnippetOverlay::Show);
    connect(ActionBar()->m_spUi->AddMetronomeCode, &QPushButton::clicked,
            m_spMetronomeSnippetOverlay.get(), &CMetronomeSnippetOverlay::Show);
    connect(ActionBar()->m_spUi->AddTimerCode, &QPushButton::clicked,
            m_spTimerSnippetOverlay.get(), &CTimerSnippetOverlay::Show);
    connect(ActionBar()->m_spUi->AddThreadCode, &QPushButton::clicked,
            m_spThreadSnippetOverlay.get(), &CThreadSnippetOverlay::Show);

    if (EditorModel()->IsReadOnly())
    {
      ActionBar()->m_spUi->AddShowBackgroundCode->setEnabled(false);
      ActionBar()->m_spUi->AddShowIconCode->setEnabled(false);
      ActionBar()->m_spUi->AddShowImageCode->setEnabled(false);
      ActionBar()->m_spUi->AddTextCode->setEnabled(false);
      ActionBar()->m_spUi->AddMetronomeCode->setEnabled(false);
      ActionBar()->m_spUi->AddTimerCode->setEnabled(false);
      ActionBar()->m_spUi->AddThreadCode->setEnabled(false);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorCodeWidget::on_pResourceComboBox_currentIndexChanged(qint32 iIndex)
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }
  m_spCurrentProject->m_rwLock.lockForRead();
  const QString sProjectName = m_spCurrentProject->m_sName;
  m_spCurrentProject->m_rwLock.unlock();

  m_bChangingIndex = true;

  // save old contents
  auto pScriptItem = ScriptEditorModel()->CachedScript(m_sLastCachedScript);
  if (nullptr != pScriptItem)
  {
    pScriptItem->m_data = m_spUi->pCodeEdit->toPlainText().toUtf8();
    ScriptEditorModel()->SetSceneScriptModifiedFlag(pScriptItem->m_sId, pScriptItem->m_bChanged);
  }

  ReloadEditor(iIndex);

  m_bChangingIndex = false;
}

//----------------------------------------------------------------------------------------
//
void CEditorCodeWidget::SlotCodeEditContentsChange(qint32 iPos, qint32 iDel, qint32 iAdd)
{
  Q_UNUSED(iPos)
  WIDGET_INITIALIZED_GUARD

  // nothing changed
  if ((0 == iDel && 0 == iAdd) || m_bChangingIndex) { return; }

  QString sCachedScript = ScriptEditorModel()->CachedScriptName(
        m_spUi->pResourceComboBox->currentIndex());
  auto pScriptItem = ScriptEditorModel()->CachedScript(sCachedScript);
  if (nullptr != pScriptItem)
  {
    pScriptItem->m_bChanged = true;
    pScriptItem->m_data = m_spUi->pCodeEdit->toPlainText().toUtf8();
    ScriptEditorModel()->SetSceneScriptModifiedFlag(pScriptItem->m_sId, true);
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorCodeWidget::SlotDebugStart()
{
  WIDGET_INITIALIZED_GUARD

  m_spUi->pSceneView->setVisible(true);
  QLayout* pLayout = m_spUi->pSceneView->layout();
  if (nullptr != pLayout)
  {
    if (nullptr != ActionBar())
    {
      ActionBar()->m_spUi->DebugButton->hide();
      ActionBar()->m_spUi->StopDebugButton->setEnabled(true);
      ActionBar()->m_spUi->StopDebugButton->show();
    }

    // get Scene name
    QString sSceneName = QString();
    QString sCachedScript = ScriptEditorModel()->CachedScriptName(
          m_spUi->pResourceComboBox->currentIndex());
    auto pScriptItem = ScriptEditorModel()->CachedScript(sCachedScript);
    if (nullptr != pScriptItem)
    {
      auto spScene = pScriptItem->m_spScene;
      if (nullptr != spScene)
      {
        QReadLocker locker(&spScene->m_rwLock);
        sSceneName = spScene->m_sName;
      }
    }

    m_spCurrentProject->m_rwLock.lockForRead();
    qint32 iCurrProject = m_spCurrentProject->m_iId;
    m_spCurrentProject->m_rwLock.unlock();
    CSceneMainScreen* pMainSceneScreen = new CSceneMainScreen(m_spUi->pSceneView);

    connect(pMainSceneScreen, &CSceneMainScreen::SignalUnloadFinished,
            this, &CEditorCodeWidget::SlotDebugUnloadFinished);

    auto spScriptRunner = pMainSceneScreen->ScriptRunner().lock();
    if (nullptr != spScriptRunner)
    {
      auto spSignalEmmiter = spScriptRunner->SignalEmmitterContext();
      connect(spSignalEmmiter.get(), &CScriptRunnerSignalContext::executionError,
              m_spUi->pCodeEdit, &CScriptEditorWidget::SlotExecutionError, Qt::QueuedConnection);
    }

    pMainSceneScreen->LoadProject(iCurrProject, sSceneName);

    pLayout->addWidget(pMainSceneScreen);

    m_spUi->pCodeEdit->ResetWidget();

    m_bDebugging = true;
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorCodeWidget::SlotDebugStop()
{
  WIDGET_INITIALIZED_GUARD

  if (nullptr != ActionBar())
  {
    ActionBar()->m_spUi->StopDebugButton->setEnabled(false);
  }

  QLayout* pLayout = m_spUi->pSceneView->layout();
  if (nullptr != pLayout)
  {
    auto pItem = pLayout->itemAt(0);
    if (nullptr != pItem)
    {
      CSceneMainScreen* pMainSceneScreen =
          qobject_cast<CSceneMainScreen*>(pItem->widget());

      auto spScriptRunner = m_wpScriptRunner.lock();
      if (nullptr != spScriptRunner)
      {
        auto spSignalEmmiter = spScriptRunner->SignalEmmitterContext();
        disconnect(spSignalEmmiter.get(), &CScriptRunnerSignalContext::executionError,
                m_spUi->pCodeEdit, &CScriptEditorWidget::SlotExecutionError);
      }

      pMainSceneScreen->SlotQuit();
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorCodeWidget::SlotDebugUnloadFinished()
{
  WIDGET_INITIALIZED_GUARD

  if (nullptr != ActionBar())
  {
    ActionBar()->m_spUi->DebugButton->show();
    ActionBar()->m_spUi->StopDebugButton->setEnabled(true);
    ActionBar()->m_spUi->StopDebugButton->hide();
  }

  QLayout* pLayout = m_spUi->pSceneView->layout();
  if (nullptr != pLayout)
  {
    auto pItem = pLayout->takeAt(0);
    if (nullptr != pItem)
    {
      CSceneMainScreen* pMainSceneScreen =
          qobject_cast<CSceneMainScreen*>(pItem->widget());

      disconnect(pMainSceneScreen, &CSceneMainScreen::SignalUnloadFinished,
                 this, &CEditorCodeWidget::SlotDebugUnloadFinished);

      delete pMainSceneScreen;
      delete pItem;
    }
  }

  m_spUi->pSceneView->setVisible(false);
  m_bDebugging = false;
  emit SignalDebugFinished();
}

//----------------------------------------------------------------------------------------
//
void CEditorCodeWidget::SlotFileChangedExternally(const QString& sName)
{
  qint32 index = m_spUi->pResourceComboBox->currentIndex();
  auto pScriptItem = ScriptEditorModel()->CachedScript(sName);
  if (index == ScriptEditorModel()->ScriptIndex(sName) && nullptr != pScriptItem)
  {
    m_bChangingIndex = true;

    m_spUi->pCodeEdit->ResetWidget();
    m_spUi->pCodeEdit->clear();
    // load new contents
    ActionBar()->m_spUi->DebugButton->setEnabled(nullptr != pScriptItem->m_spScene);
    m_spUi->pCodeEdit->setPlainText(QString::fromUtf8(pScriptItem->m_data));

    m_spUi->pCodeEdit->update();

    m_bChangingIndex = false;
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorCodeWidget::SlotInsertGeneratedCode(const QString& sCode)
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }
  m_spUi->pCodeEdit->insertPlainText(sCode);
}

//----------------------------------------------------------------------------------------
//
void CEditorCodeWidget::SlotRowsInserted(const QModelIndex& parent, int iFirst, int iLast)
{
  Q_UNUSED(parent) Q_UNUSED(iFirst) Q_UNUSED(iLast)
  if (0 < ScriptEditorModel()->rowCount())
  {
    on_pResourceComboBox_currentIndexChanged(0);
    m_spUi->pStackedWidget->setCurrentIndex(c_iIndexScripts);
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorCodeWidget::SlotRowsRemoved(const QModelIndex& parent, int iFirst, int iLast)
{
  Q_UNUSED(parent) Q_UNUSED(iFirst) Q_UNUSED(iLast)
  if (0 >= ScriptEditorModel()->rowCount())
  {
    m_spUi->pStackedWidget->setCurrentIndex(c_iIndexNoScripts);
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorCodeWidget::ReloadEditor(qint32 iIndex)
{
  m_bChangingIndex = true;

  m_spUi->pCodeEdit->ResetWidget();
  m_spUi->pCodeEdit->clear();

  // load new contents
  m_sLastCachedScript = ScriptEditorModel()->CachedScriptName(iIndex);
  auto  pScriptItem = ScriptEditorModel()->CachedScript(m_sLastCachedScript);
  if (nullptr != pScriptItem)
  {
    if (nullptr != ActionBar())
    {
      ActionBar()->m_spUi->DebugButton->setEnabled(nullptr != pScriptItem->m_spScene);
    }
    m_spUi->pCodeEdit->setPlainText(QString::fromUtf8(pScriptItem->m_data));
    m_spUi->pCodeEdit->SetHighlightDefinition("JavaScript");
  }
  m_spUi->pCodeEdit->update();

  m_bChangingIndex = false;
}
