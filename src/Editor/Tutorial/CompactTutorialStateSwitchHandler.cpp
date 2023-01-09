#include "CompactTutorialStateSwitchHandler.h"
#include "EditorTutorialOverlay.h"
#include "Editor/EditorLayouts/EditorLayoutCompact.h"
#include <QDebug>
#include <QFile>
#include <QTimer>

CCompactTutorialStateSwitchHandler::CCompactTutorialStateSwitchHandler(
    QPointer<CEditorLayoutCompact> pParentWidget,
    QPointer<CEditorTutorialOverlay> pTutorialOverlay) :
  CTutorialStateSwitchHandlerMainBase(pTutorialOverlay,
                                      ":/resources/help/tutorial/TutorialCompact.json"),
  m_ParentWidget(pParentWidget)
{
  connect(m_ParentWidget, &CEditorLayoutCompact::SignalViewChanged,
          this, &CCompactTutorialStateSwitchHandler::SlotViewSwitched);
  connect(pTutorialOverlay, &CEditorTutorialOverlay::SignalOverlayNextInstructionTriggered,
          this, &CCompactTutorialStateSwitchHandler::SlotOverlayNextInstructionTriggered,
          Qt::QueuedConnection);
}

CCompactTutorialStateSwitchHandler::~CCompactTutorialStateSwitchHandler()
{

}

//----------------------------------------------------------------------------------------
//
void CCompactTutorialStateSwitchHandler::OnStateSwitchImpl(ETutorialState newState,
                                                           ETutorialState oldState)
{
  switch (newState)
  {
    case ETutorialState::eBeginTutorial: // fallthrough
    case ETutorialState::eSwitchRightPanelToProjectSettings:
    {
      bool bOk = QMetaObject::invokeMethod(this, "SlotSwitchView",
                                           Qt::QueuedConnection,
                                           Q_ARG(qint32, EEditorWidget::eResourceDisplay));
      assert(bOk);
      Q_UNUSED(bOk);
      for (EEditorWidget val : EEditorWidget::_values())
      {
        if (EEditorWidget::eProjectSettings != val._to_integral())
        {
          m_ParentWidget->SetButtonVisible(val, false);
        }
        else
        {
          m_ParentWidget->SetButtonVisible(val, true);
        }
      }
    } break;
    case ETutorialState::eProjectSettings: // fallthrough
    case ETutorialState::eResourcePanel:
    {
      for (EEditorWidget val : EEditorWidget::_values())
      {
        if (EEditorWidget::eProjectSettings != val._to_integral())
        {
          m_ParentWidget->SetButtonVisible(val, false);
        }
        else
        {
          m_ParentWidget->SetButtonVisible(val, true);
        }
      }

      if (ETutorialState::eResourcePanel == newState._to_integral())
      {
        m_ParentWidget->SetButtonVisible(EEditorWidget::eResourceWidget, true);
      }

      bool bOk = QMetaObject::invokeMethod(this, "SlotSwitchView",
                                           Qt::QueuedConnection,
                                           Q_ARG(qint32, EEditorWidget::eProjectSettings));
      assert(bOk);
      Q_UNUSED(bOk);
    } break;
    case ETutorialState::eImageResourceSelected:
    {
      for (EEditorWidget val : EEditorWidget::_values())
      {
        if (EEditorWidget::eProjectSettings != val._to_integral() &&
            EEditorWidget::eResourceWidget != val._to_integral())
        {
          m_ParentWidget->SetButtonVisible(val, false);
        }
        else
        {
          m_ParentWidget->SetButtonVisible(val, true);
        }
      }

      bool bOk = QMetaObject::invokeMethod(this, "SlotSwitchView",
                                           Qt::QueuedConnection,
                                           Q_ARG(qint32, EEditorWidget::eResourceWidget));
      assert(bOk);
      Q_UNUSED(bOk);
    } break;
    case ETutorialState::eSwitchRightPanelToNodeSettings:
    {
      for (EEditorWidget val : EEditorWidget::_values())
      {
        if (EEditorWidget::eProjectSettings != val._to_integral() &&
            EEditorWidget::eResourceWidget != val._to_integral() &&
            EEditorWidget::eSceneNodeWidget != val._to_integral())
        {
          m_ParentWidget->SetButtonVisible(val, false);
        }
        else
        {
          m_ParentWidget->SetButtonVisible(val, true);
        }
      }

      bool bOk = QMetaObject::invokeMethod(this, "SlotSwitchView",
                                           Qt::QueuedConnection,
                                           Q_ARG(qint32, EEditorWidget::eResourceWidget));
      assert(bOk);
      Q_UNUSED(bOk);
    } break;
    case ETutorialState::eNodePanel: // fallthrough
    case ETutorialState::eNodePanelAdvanced:
    {
      for (EEditorWidget val : EEditorWidget::_values())
      {
        if (EEditorWidget::eProjectSettings != val._to_integral() &&
            EEditorWidget::eResourceWidget != val._to_integral() &&
            EEditorWidget::eSceneNodeWidget != val._to_integral())
        {
          m_ParentWidget->SetButtonVisible(val, false);
        }
        else
        {
          m_ParentWidget->SetButtonVisible(val, true);
        }
      }

      bool bOk = QMetaObject::invokeMethod(this, "SlotSwitchView",
                                           Qt::QueuedConnection,
                                           Q_ARG(qint32, EEditorWidget::eSceneNodeWidget));
      assert(bOk);
      Q_UNUSED(bOk);
    } break;
    case ETutorialState::eNodePanelDone:
    {
      for (EEditorWidget val : EEditorWidget::_values())
      {
        m_ParentWidget->SetButtonVisible(val, true);
      }

      bool bOk = QMetaObject::invokeMethod(this, "SlotSwitchView",
                                           Qt::QueuedConnection,
                                           Q_ARG(qint32, EEditorWidget::eSceneNodeWidget));
      assert(bOk);
      Q_UNUSED(bOk);
    } break;
    case ETutorialState::eCodePanel:
    {
      for (EEditorWidget val : EEditorWidget::_values())
      {
        m_ParentWidget->SetButtonVisible(val, true);
      }

      bool bOk = QMetaObject::invokeMethod(this, "SlotSwitchView",
                                           Qt::QueuedConnection,
                                           Q_ARG(qint32, EEditorWidget::eSceneCodeEditorWidget));
      assert(bOk);
      Q_UNUSED(bOk);
    } break;
    case ETutorialState::eFinished:
    {
      if (ETutorialState::eCodePanel == oldState._to_integral())
      {
        QTimer::singleShot(500, m_pTutorialOverlay, SLOT(Hide()));
      }
    } break;
  default: break;
  }
}

//----------------------------------------------------------------------------------------
//
void CCompactTutorialStateSwitchHandler::SlotSwitchView(qint32 iView)
{
  m_ParentWidget->ChangeView(iView);
}

//----------------------------------------------------------------------------------------
//
void CCompactTutorialStateSwitchHandler::SlotViewSwitched(qint32 iView)
{
  if (ETutorialState::eSwitchRightPanelToProjectSettings == m_currentState._to_integral() &&
      EEditorWidget::eProjectSettings == iView)
  {
    bool bOk = QMetaObject::invokeMethod(this, "SlotOverlayNextInstructionTriggered",
                                         Qt::QueuedConnection);
    assert(bOk);
    Q_UNUSED(bOk);
  }
  else if (ETutorialState::eSwitchRightPanelToNodeSettings == m_currentState._to_integral() &&
           EEditorWidget::eSceneNodeWidget == iView)
  {
    bool bOk = QMetaObject::invokeMethod(this, "SlotOverlayNextInstructionTriggered",
                                         Qt::QueuedConnection);
    assert(bOk);
    Q_UNUSED(bOk);
  }
  else if (ETutorialState::eResourcePanel == m_currentState._to_integral() &&
           EEditorWidget::eResourceWidget == iView)
  {
    bool bOk = QMetaObject::invokeMethod(this, "SlotOverlayNextInstructionTriggered",
                                         Qt::QueuedConnection);
    assert(bOk);
    Q_UNUSED(bOk);
  }
  else if (ETutorialState::eNodePanelDone == m_currentState._to_integral() &&
           EEditorWidget::eSceneCodeEditorWidget == iView)
  {
    bool bOk = QMetaObject::invokeMethod(this, "SlotOverlayNextInstructionTriggered",
                                         Qt::QueuedConnection);
    assert(bOk);
    Q_UNUSED(bOk);
  }
}
