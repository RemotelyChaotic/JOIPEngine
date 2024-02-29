#include "EditorDebuggableWidget.h"

#include "Player/SceneMainScreen.h"

#include "Systems/Script/ScriptRunnerSignalEmiter.h"
#include "Systems/Project.h"
#include "Systems/Scene.h"
#include "Systems/ScriptRunner.h"

#include <QLayout>
#include <QMenu>

CEditorDebuggableWidget::CEditorDebuggableWidget(QWidget* pParent) :
  CEditorWidgetBase(pParent),
  m_spCurrentProject(nullptr),
  m_wpScriptRunner(),
  m_debugFinishedConnection(),
  m_bDebugging(false)
{
}
CEditorDebuggableWidget::~CEditorDebuggableWidget()
{
}

//----------------------------------------------------------------------------------------
//
void CEditorDebuggableWidget::Initalize(QPointer<QWidget> pSceneView,
                                        tFnGetScene fnGetScene)
{
  m_pSceneView = pSceneView;
  m_fnGetScene = fnGetScene;
}

void CEditorDebuggableWidget::UpdateButtons(QPointer<QPushButton> pDebugButton,
                                            QPointer<QPushButton> pStopDebugButton)
{
  m_pDebugButton = pDebugButton;
  m_pStopDebugButton = pStopDebugButton;
}

//----------------------------------------------------------------------------------------
//
void CEditorDebuggableWidget::UnloadProject()
{
  WIDGET_INITIALIZED_GUARD

  if (m_bDebugging)
  {
    m_debugFinishedConnection =
        connect(this, &CEditorDebuggableWidget::SignalDebugFinished, this,
            [this](){
              disconnect(m_debugFinishedConnection);
              SetLoaded(false);
            }, Qt::QueuedConnection);

    SlotDebugStop();
  }

  UnloadProjectImpl();

  if (!m_bDebugging)
  {
    SetLoaded(false);
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorDebuggableWidget::SlotDebugStart()
{
  WIDGET_INITIALIZED_GUARD

  m_pSceneView->setVisible(true);
  QLayout* pLayout = m_pSceneView->layout();
  if (nullptr != pLayout)
  {
    if (nullptr != m_pDebugButton && nullptr != m_pStopDebugButton)
    {
      m_pDebugButton->hide();
      m_pStopDebugButton->setEnabled(true);
      m_pStopDebugButton->show();
    }

    // get Scene name
    auto scene = m_fnGetScene();
    if (std::holds_alternative<std::nullptr_t>(scene))
    {
      return;
    }

    m_spCurrentProject->m_rwLock.lockForRead();
    qint32 iCurrProject = m_spCurrentProject->m_iId;
    m_spCurrentProject->m_rwLock.unlock();
    CSceneMainScreen* pMainSceneScreen = new CSceneMainScreen(m_pSceneView);
    pMainSceneScreen->Initialize(nullptr, true);

    connect(pMainSceneScreen, &CSceneMainScreen::SignalUnloadFinished,
            this, &CEditorDebuggableWidget::SlotDebugUnloadFinished);
    connect(pMainSceneScreen, &CSceneMainScreen::SignalExecutionError,
            this, &CEditorDebuggableWidget::SignalExecutionError,
            Qt::QueuedConnection);

    auto spScriptRunner = pMainSceneScreen->ScriptRunner().lock();
    if (nullptr != spScriptRunner)
    {
      auto spSignalEmmiter = spScriptRunner->SignalEmmitterContext();
      connect(spSignalEmmiter.get(), &CScriptRunnerSignalContext::executionError,
              this, &CEditorDebuggableWidget::SignalExecutionError,
              Qt::QueuedConnection);
    }

    if (std::holds_alternative<QString>(scene))
    {
      pMainSceneScreen->LoadProject(iCurrProject, std::get<QString>(scene));
    }
    if (std::holds_alternative<tspScene>(scene))
    {
      pMainSceneScreen->LoadProject(iCurrProject, std::get<tspScene>(scene));
    }

    pLayout->addWidget(pMainSceneScreen);

    emit SignalDebugStarted();

    m_bDebugging = true;
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorDebuggableWidget::SlotDebugStop()
{
  WIDGET_INITIALIZED_GUARD

  if (nullptr != m_pStopDebugButton)
  {
    m_pStopDebugButton->setEnabled(false);
  }

  QLayout* pLayout = m_pSceneView->layout();
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
                   this, &CEditorDebuggableWidget::SignalExecutionError);
      }

      pMainSceneScreen->SlotQuit();
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorDebuggableWidget::SlotDebugUnloadFinished()
{
  WIDGET_INITIALIZED_GUARD

  if (nullptr != m_pDebugButton && nullptr != m_pStopDebugButton)
  {
    m_pDebugButton->show();
    m_pStopDebugButton->setEnabled(true);
    m_pStopDebugButton->hide();
  }

  QLayout* pLayout = m_pSceneView->layout();
  if (nullptr != pLayout)
  {
    auto pItem = pLayout->takeAt(0);
    if (nullptr != pItem)
    {
      CSceneMainScreen* pMainSceneScreen =
          qobject_cast<CSceneMainScreen*>(pItem->widget());

      disconnect(pMainSceneScreen, &CSceneMainScreen::SignalUnloadFinished,
                 this, &CEditorDebuggableWidget::SlotDebugUnloadFinished);
      disconnect(pMainSceneScreen, &CSceneMainScreen::SignalExecutionError,
                 this, &CEditorDebuggableWidget::SignalExecutionError);

      delete pMainSceneScreen;
      delete pItem;
    }
  }

  m_pSceneView->setVisible(false);
  m_bDebugging = false;
  emit SignalDebugFinished();
}
