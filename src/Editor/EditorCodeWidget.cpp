#include "EditorCodeWidget.h"
#include "Application.h"
#include "EditorActionBar.h"
#include "Player/SceneMainScreen.h"
#include "Script/BackgroundSnippetOverlay.h"
#include "Script/IconSnippetOverlay.h"
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
#include "Widgets/HelpOverlay.h"
#include "ui_EditorCodeWidget.h"
#include "ui_EditorActionBar.h"

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
  m_spUi(new Ui::CEditorCodeWidget),
  m_spBackgroundSnippetOverlay(std::make_unique<CBackgroundSnippetOverlay>(this)),
  m_spIconSnippetOverlay(std::make_unique<CIconSnippetOverlay>(this)),
  m_spResourceSnippetOverlay(std::make_unique<CResourceSnippetOverlay>(this)),
  m_spTextSnippetOverlay(std::make_unique<CTextSnippetOverlay>(this)),
  m_spTimerSnippetOverlay(std::make_unique<CTimerSnippetOverlay>(this)),
  m_spThreadSnippetOverlay(std::make_unique<CThreadSnippetOverlay>(this)),
  m_spSettings(CApplication::Instance()->Settings()),
  m_spCurrentProject(nullptr),
  m_wpDbManager(),
  m_wpScriptRunner(),
  m_pDummyModel(new QStandardItemModel(this)),
  m_iLastIndex(-1)
{
  m_spUi->setupUi(this);
  m_spUi->pSceneView->setVisible(false);
  m_pHighlighter = new CScriptHighlighter(m_spUi->pCodeEdit->document());
}

CEditorCodeWidget::~CEditorCodeWidget()
{
  m_spThreadSnippetOverlay.reset();
  m_spTimerSnippetOverlay.reset();
  m_spTimerSnippetOverlay.reset();
  m_spTextSnippetOverlay.reset();
  m_spResourceSnippetOverlay.reset();
  m_spIconSnippetOverlay.reset();
  m_spBackgroundSnippetOverlay.reset();
}

//----------------------------------------------------------------------------------------
//
void CEditorCodeWidget::Initialize()
{
  m_bInitialized = false;

  m_wpDbManager = CApplication::Instance()->System<CDatabaseManager>();
  m_wpScriptRunner = CApplication::Instance()->System<CScriptRunner>();

  m_spBackgroundSnippetOverlay->Initialize(ResourceTreeModel());
  m_spIconSnippetOverlay->Initialize(ResourceTreeModel());
  m_spResourceSnippetOverlay->Initialize(ResourceTreeModel());

  m_spBackgroundSnippetOverlay->Hide();
  m_spIconSnippetOverlay->Hide();
  m_spResourceSnippetOverlay->Hide();
  m_spTextSnippetOverlay->Hide();
  m_spTimerSnippetOverlay->Hide();
  m_spThreadSnippetOverlay->Hide();

  connect(m_spBackgroundSnippetOverlay.get(), &CBackgroundSnippetOverlay::SignalBackgroundCode,
          this, &CEditorCodeWidget::SlotInsertGeneratedCode);
  connect(m_spIconSnippetOverlay.get(), &CIconSnippetOverlay::SignalIconCode,
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

  if (0 < ScriptEditorModel()->rowCount())
  {
    on_pResourceComboBox_currentIndexChanged(0);
    m_spUi->pStackedWidget->setCurrentIndex(c_iIndexScripts);
  }
  else
  {
    m_spUi->pStackedWidget->setCurrentIndex(c_iIndexNoScripts);
  }

  auto spScriptRunner = m_wpScriptRunner.lock();
  if (nullptr != spScriptRunner)
  {
    auto spSignalEmmiter = spScriptRunner->SignalEmmitterContext();
    connect(spSignalEmmiter.get(), &CScriptRunnerSignalContext::executionError,
            m_spUi->pCodeEdit, &CScriptEditorWidget::SlotExecutionError, Qt::QueuedConnection);
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorCodeWidget::UnloadProject()
{
  WIDGET_INITIALIZED_GUARD

  SlotDebugStop();

  m_spUi->pCodeEdit->blockSignals(true);
  m_spUi->pCodeEdit->ResetWidget();
  m_spUi->pCodeEdit->clear();
  m_spUi->pCodeEdit->blockSignals(false);

  auto spScriptRunner = m_wpScriptRunner.lock();
  if (nullptr != spScriptRunner)
  {
    auto spSignalEmmiter = spScriptRunner->SignalEmmitterContext();
    disconnect(spSignalEmmiter.get(), &CScriptRunnerSignalContext::executionError,
            m_spUi->pCodeEdit, &CScriptEditorWidget::SlotExecutionError);
  }

  m_spResourceSnippetOverlay->UnloadProject();

  m_spCurrentProject = nullptr;

  m_iLastIndex = -1;

  auto pModel = ScriptEditorModel();
  disconnect(pModel, &CScriptEditorModel::rowsInserted,
             this, &CEditorCodeWidget::SlotRowsInserted);
  disconnect(pModel, &CScriptEditorModel::rowsRemoved,
             this, &CEditorCodeWidget::SlotRowsRemoved);

  m_spUi->pResourceComboBox->setModel(m_pDummyModel);
  m_spUi->pResourceComboBox->clear();

  m_spBackgroundSnippetOverlay->Hide();
  m_spIconSnippetOverlay->Hide();
  m_spResourceSnippetOverlay->Hide();
  m_spTextSnippetOverlay->Hide();
  m_spTimerSnippetOverlay->Hide();
  m_spThreadSnippetOverlay->Hide();
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
  auto pScriptItem = ScriptEditorModel()->CachedScript(m_spUi->pResourceComboBox->currentIndex());
  if (nullptr != pScriptItem)
  {
    pScriptItem->m_data = m_spUi->pCodeEdit->toPlainText().toUtf8();
    ScriptEditorModel()->SetSceneScriptModifiedFlag(pScriptItem->m_sId, pScriptItem->m_bChanged);
  }
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
    disconnect(ActionBar()->m_spUi->AddTimerCode, &QPushButton::clicked,
            m_spTimerSnippetOverlay.get(), &CTimerSnippetOverlay::Show);
    disconnect(ActionBar()->m_spUi->AddThreadCode, &QPushButton::clicked,
            m_spThreadSnippetOverlay.get(), &CThreadSnippetOverlay::Show);
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
    connect(ActionBar()->m_spUi->AddTimerCode, &QPushButton::clicked,
            m_spTimerSnippetOverlay.get(), &CTimerSnippetOverlay::Show);
    connect(ActionBar()->m_spUi->AddThreadCode, &QPushButton::clicked,
            m_spThreadSnippetOverlay.get(), &CThreadSnippetOverlay::Show);
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

  // save old contents
  auto pScriptItem = ScriptEditorModel()->CachedScript(m_iLastIndex);
  if (nullptr != pScriptItem)
  {
    pScriptItem->m_data = m_spUi->pCodeEdit->toPlainText().toUtf8();
    ScriptEditorModel()->SetSceneScriptModifiedFlag(pScriptItem->m_sId, pScriptItem->m_bChanged);
  }

  m_spUi->pCodeEdit->blockSignals(true);
  m_spUi->pCodeEdit->ResetWidget();
  m_spUi->pCodeEdit->clear();
  m_spUi->pCodeEdit->blockSignals(false);

  // load new contents
  pScriptItem = ScriptEditorModel()->CachedScript(iIndex);
  if (nullptr != pScriptItem)
  {
    if (nullptr != ActionBar())
    {
      ActionBar()->m_spUi->DebugButton->setEnabled(nullptr != pScriptItem->m_spScene);
    }
    m_spUi->pCodeEdit->blockSignals(true);
    m_spUi->pCodeEdit->setPlainText(QString::fromUtf8(pScriptItem->m_data));
    m_spUi->pCodeEdit->blockSignals(false);
  }
  m_spUi->pCodeEdit->update();

  m_iLastIndex = iIndex;
}

//----------------------------------------------------------------------------------------
//
void CEditorCodeWidget::on_pCodeEdit_textChanged()
{
  WIDGET_INITIALIZED_GUARD

  qint32 index = m_spUi->pResourceComboBox->currentIndex();
  auto pScriptItem = ScriptEditorModel()->CachedScript(index);
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
      ActionBar()->m_spUi->StopDebugButton->show();
    }

    // get Scene name
    QString sSceneName = QString();
    qint32 index = m_spUi->pResourceComboBox->currentIndex();
    auto pScriptItem = ScriptEditorModel()->CachedScript(index);
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
    pMainSceneScreen->LoadProject(iCurrProject, sSceneName);

    pLayout->addWidget(pMainSceneScreen);

    m_spUi->pCodeEdit->ResetWidget();
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorCodeWidget::SlotDebugStop()
{
  WIDGET_INITIALIZED_GUARD

  QLayout* pLayout = m_spUi->pSceneView->layout();
  if (nullptr != pLayout)
  {
    if (nullptr != ActionBar())
    {
      ActionBar()->m_spUi->DebugButton->show();
      ActionBar()->m_spUi->StopDebugButton->hide();
    }

    auto pItem = pLayout->takeAt(0);
    if (nullptr != pItem)
    {
      CSceneMainScreen* pMainSceneScreen =
          qobject_cast<CSceneMainScreen*>(pItem->widget());
      pMainSceneScreen->SlotQuit();
      delete pMainSceneScreen;
    }
  }
  m_spUi->pSceneView->setVisible(false);
}

//----------------------------------------------------------------------------------------
//
void CEditorCodeWidget::SlotFileChangedExternally(const QString& sName)
{
  qint32 index = m_spUi->pResourceComboBox->currentIndex();
  auto pScriptItem = ScriptEditorModel()->CachedScript(sName);
  if (index == ScriptEditorModel()->ScriptIndex(sName) && nullptr != pScriptItem)
  {
    m_spUi->pCodeEdit->blockSignals(true);
    m_spUi->pCodeEdit->ResetWidget();
    m_spUi->pCodeEdit->clear();
    m_spUi->pCodeEdit->blockSignals(false);

    // load new contents
    ActionBar()->m_spUi->DebugButton->setEnabled(nullptr != pScriptItem->m_spScene);
    m_spUi->pCodeEdit->blockSignals(true);
    m_spUi->pCodeEdit->setPlainText(QString::fromUtf8(pScriptItem->m_data));
    m_spUi->pCodeEdit->blockSignals(false);

    m_spUi->pCodeEdit->update();
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
