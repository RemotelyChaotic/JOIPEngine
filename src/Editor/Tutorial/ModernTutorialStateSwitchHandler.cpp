#include "ModernTutorialStateSwitchHandler.h"
#include "EditorTutorialOverlay.h"
#include "Editor/EditorModel.h"
#include "Editor/EditorLayouts/EditorLayoutClassic.h"
#include "Editor/EditorLayouts/EditorLayoutModern.h"
#include <QComboBox>
#include <QDebug>
#include <QGroupBox>
#include <QListView>
#include <QTimer>

CModernTutorialStateSwitchHandler::CModernTutorialStateSwitchHandler(
    QPointer<CEditorLayoutModern> pParentWidget,
    QPointer<CEditorTutorialOverlay> pTutorialOverlay) :
  CTutorialStateSwitchHandlerMainBase(pTutorialOverlay,
                                      ":/resources/help/tutorial/TutorialModern.json"),
  m_ParentWidget(pParentWidget)
{
  connect(m_ParentWidget->TopEditor()->RightComboBox(),
          qOverload<qint32>(&QComboBox::currentIndexChanged),
          this, &CModernTutorialStateSwitchHandler::SlotRightPanelSwitched);
  connect(pTutorialOverlay, &CEditorTutorialOverlay::SignalOverlayNextInstructionTriggered,
          this, &CModernTutorialStateSwitchHandler::SlotOverlayNextInstructionTriggered,
          Qt::QueuedConnection);
}

CModernTutorialStateSwitchHandler::~CModernTutorialStateSwitchHandler()
{

}

//----------------------------------------------------------------------------------------
//
void CModernTutorialStateSwitchHandler::OnStateSwitchImpl(ETutorialState newState,
                                                          ETutorialState oldState)
{
  switch (newState)
  {
    case ETutorialState::eBeginTutorial: // fallthrough
    case ETutorialState::eSwitchRightPanelToProjectSettings:
    {
      for (EEditorWidget val : EEditorWidget::_values())
      {
        if (EEditorWidget::eProjectSettings != val._to_integral())
        {
          qobject_cast<QListView*>(m_ParentWidget->TopEditor()->RightComboBox()->view())
              ->setRowHidden(val._to_integral() - 1, true);
        }
      }
      m_ParentWidget->BottomEditor()->setEnabled(false);
    } break;
    case ETutorialState::eProjectSettings:
    {
      for (EEditorWidget val : EEditorWidget::_values())
      {
        qobject_cast<QListView*>(m_ParentWidget->TopEditor()->RightComboBox()->view())
            ->setRowHidden(val._to_integral(), false);
      }

      bool bOk = QMetaObject::invokeMethod(this, "SlotSwitchRightPanel",
                                           Qt::QueuedConnection,
                                           Q_ARG(qint32, EEditorWidget::eProjectSettings));
      assert(bOk);
      Q_UNUSED(bOk);

      m_ParentWidget->TopEditor()->LeftComboBox()->setEnabled(false);
      m_ParentWidget->TopEditor()->RightComboBox()->setEnabled(false);
      m_ParentWidget->TopEditor()->LeftGroupBox()->setEnabled(false);
      m_ParentWidget->BottomEditor()->setEnabled(false);
    } break;
    case ETutorialState::eResourcePanel:
    {
      m_ParentWidget->TopEditor()->LeftComboBox()->setEnabled(false);
      m_ParentWidget->TopEditor()->RightComboBox()->setEnabled(false);
      m_ParentWidget->TopEditor()->LeftGroupBox()->setEnabled(true);
      m_ParentWidget->BottomEditor()->setEnabled(true);
    } break;
    case ETutorialState::eImageResourceSelected:
    {
      m_ParentWidget->TopEditor()->LeftComboBox()->setEnabled(false);
      m_ParentWidget->TopEditor()->RightComboBox()->setEnabled(false);
    } break;
    case ETutorialState::eSwitchRightPanelToNodeSettings:
    {
      for (EEditorWidget val : EEditorWidget::_values())
      {
        if (EEditorWidget::eSceneNodeWidget != val._to_integral() &&
            EEditorWidget::eProjectSettings != val._to_integral())
        {
          qobject_cast<QListView*>(m_ParentWidget->TopEditor()->RightComboBox()->view())
              ->setRowHidden(val - 1, true);
        }
      }
      m_ParentWidget->TopEditor()->RightComboBox()->setEnabled(true);
    } break;
    case ETutorialState::eNodePanel:
    {
      m_ParentWidget->EditorModel()->SetScriptTypeFilterForNewScripts("^\\*\\.(?!eos$).*$");
      for (EEditorWidget val : EEditorWidget::_values())
      {
        qobject_cast<QListView*>(m_ParentWidget->TopEditor()->RightComboBox()->view())
            ->setRowHidden(val - 1, false);
      }
    } // fallthrough
    case ETutorialState::eNodePanelAdvanced:
      if (ETutorialState::eNodePanelAdvanced == newState._to_integral())
      {
        m_ParentWidget->EditorModel()->SetScriptTypeFilterForNewScripts(".*");
      } // fallthrough
    case ETutorialState::eNodePanelDone:
    {
      bool bOk = QMetaObject::invokeMethod(this, "SlotSwitchRightPanel",
                                           Qt::QueuedConnection,
                                           Q_ARG(qint32, EEditorWidget::eSceneNodeWidget));
      assert(bOk);
      Q_UNUSED(bOk);

      m_ParentWidget->TopEditor()->LeftComboBox()->setEnabled(false);
      m_ParentWidget->TopEditor()->RightComboBox()->setEnabled(newState._to_integral() == ETutorialState::eNodePanelDone);
      m_ParentWidget->TopEditor()->LeftGroupBox()->setEnabled(false);
    } break;
    case ETutorialState::eCodePanelEOS: // fallthrough
    case ETutorialState::eCodePanel:
    {
      bool bOk = QMetaObject::invokeMethod(this, "SlotSwitchLeftPanel",
                                           Qt::QueuedConnection,
                                           Q_ARG(qint32, EEditorWidget::eResourceWidget));
      bOk &= QMetaObject::invokeMethod(this, "SlotSwitchRightPanel",
                                       Qt::QueuedConnection,
                                       Q_ARG(qint32, EEditorWidget::eSceneCodeEditorWidget));
      assert(bOk);
      Q_UNUSED(bOk);

      m_ParentWidget->TopEditor()->LeftComboBox()->setEnabled(false);
      m_ParentWidget->TopEditor()->RightComboBox()->setEnabled(false);
      m_ParentWidget->TopEditor()->LeftGroupBox()->setEnabled(true);
    } break;
    case ETutorialState::eFinished:
    {
      if (ETutorialState::eCodePanelEOS == oldState._to_integral())
      {
        m_ParentWidget->TopEditor()->LeftComboBox()->setEnabled(true);
        m_ParentWidget->TopEditor()->RightComboBox()->setEnabled(true);
        m_ParentWidget->TopEditor()->LeftGroupBox()->setEnabled(true);
        m_ParentWidget->TopEditor()->RightGroupBox()->setEnabled(true);

        QTimer::singleShot(500, m_pTutorialOverlay, SLOT(Hide()));
      }
    } break;
  default: break;
  }
}

//----------------------------------------------------------------------------------------
//
void CModernTutorialStateSwitchHandler::SlotSwitchLeftPanel(qint32 iNewIndex)
{
  qint32 iIndex = m_ParentWidget->TopEditor()->LeftComboBox()->findData(iNewIndex);
  m_ParentWidget->TopEditor()->LeftComboBox()->setCurrentIndex(iIndex);
}

//----------------------------------------------------------------------------------------
//
void CModernTutorialStateSwitchHandler::SlotSwitchRightPanel(qint32 iNewIndex)
{
  qint32 iIndex = m_ParentWidget->TopEditor()->RightComboBox()->findData(iNewIndex);
  m_ParentWidget->TopEditor()->RightComboBox()->setCurrentIndex(iIndex);
}

//----------------------------------------------------------------------------------------
//
void CModernTutorialStateSwitchHandler::SlotRightPanelSwitched(qint32 iNewIndex)
{
  if (ETutorialState::eSwitchRightPanelToProjectSettings == m_currentState._to_integral() &&
      EEditorWidget::eProjectSettings == iNewIndex+1)
  {
    bool bOk = QMetaObject::invokeMethod(this, "SlotOverlayNextInstructionTriggered",
                                         Qt::QueuedConnection);
    assert(bOk);
    Q_UNUSED(bOk);
  }
  else if (ETutorialState::eSwitchRightPanelToNodeSettings == m_currentState._to_integral() &&
           EEditorWidget::eSceneNodeWidget == iNewIndex+1)
  {
    bool bOk = QMetaObject::invokeMethod(this, "SlotOverlayNextInstructionTriggered",
                                         Qt::QueuedConnection);
    assert(bOk);
    Q_UNUSED(bOk);
  }
  else if (ETutorialState::eNodePanelDone == m_currentState._to_integral() &&
           EEditorWidget::eSceneCodeEditorWidget == iNewIndex+1)
  {
    bool bOk = QMetaObject::invokeMethod(this, "SlotOverlayNextInstructionTriggered",
                                         Qt::QueuedConnection);
    assert(bOk);
    Q_UNUSED(bOk);
  }
}
