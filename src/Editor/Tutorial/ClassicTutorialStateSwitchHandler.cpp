#include "ClassicTutorialStateSwitchHandler.h"
#include "EditorTutorialOverlay.h"
#include "Editor/EditorModel.h"
#include "Editor/EditorLayouts/EditorLayoutClassic.h"
#include <QComboBox>
#include <QDebug>
#include <QGroupBox>
#include <QListView>
#include <QTimer>

CClassicTutorialStateSwitchHandler::CClassicTutorialStateSwitchHandler(
    QPointer<CEditorLayoutClassic> pParentWidget,
    QPointer<CEditorTutorialOverlay> pTutorialOverlay) :
  CTutorialStateSwitchHandlerMainBase(pTutorialOverlay,
                                      ":/resources/help/tutorial/TutorialClassic.json"),
  m_ParentWidget(pParentWidget)
{
  connect(m_ParentWidget->RightComboBox(), qOverload<qint32>(&QComboBox::currentIndexChanged),
          this, &CClassicTutorialStateSwitchHandler::SlotRightPanelSwitched);
  connect(pTutorialOverlay, &CEditorTutorialOverlay::SignalOverlayNextInstructionTriggered,
          this, &CClassicTutorialStateSwitchHandler::SlotOverlayNextInstructionTriggered,
          Qt::QueuedConnection);
}

CClassicTutorialStateSwitchHandler::~CClassicTutorialStateSwitchHandler()
{

}

//----------------------------------------------------------------------------------------
//
void CClassicTutorialStateSwitchHandler::OnStateSwitchImpl(ETutorialState newState,
                                                           ETutorialState oldState)
{
  switch (newState)
  {
    case ETutorialState::eBeginTutorial: // fallthrough
    case ETutorialState::eSwitchRightPanelToProjectSettings:
    {
      bool bOk = QMetaObject::invokeMethod(this, "SlotSwitchRightPanel",
                                           Qt::QueuedConnection,
                                           Q_ARG(qint32, EEditorWidget::eResourceDisplay));
      assert(bOk);
      Q_UNUSED(bOk);
      for (EEditorWidget val : EEditorWidget::_values())
      {
        if (EEditorWidget::eProjectSettings != val._to_integral())
        {
          qobject_cast<QListView*>(m_ParentWidget->RightComboBox()->view())
              ->setRowHidden(val, true);
        }
      }
    } break;
    case ETutorialState::eProjectSettings:
    {
      for (EEditorWidget val : EEditorWidget::_values())
      {
        qobject_cast<QListView*>(m_ParentWidget->RightComboBox()->view())
            ->setRowHidden(val, false);
      }

      bool bOk = QMetaObject::invokeMethod(this, "SlotSwitchRightPanel",
                                           Qt::QueuedConnection,
                                           Q_ARG(qint32, EEditorWidget::eProjectSettings));
      assert(bOk);
      Q_UNUSED(bOk);

      m_ParentWidget->LeftComboBox()->setEnabled(false);
      m_ParentWidget->RightComboBox()->setEnabled(false);
      m_ParentWidget->LeftGroupBox()->setEnabled(false);
    } break;
    case ETutorialState::eResourcePanel:
    {
      m_ParentWidget->LeftComboBox()->setEnabled(false);
      m_ParentWidget->RightComboBox()->setEnabled(false);
      m_ParentWidget->LeftGroupBox()->setEnabled(true);
    } break;
    case ETutorialState::eImageResourceSelected:
    {
      m_ParentWidget->LeftComboBox()->setEnabled(false);
      m_ParentWidget->RightComboBox()->setEnabled(false);
    } break;
    case ETutorialState::eSwitchRightPanelToNodeSettings:
    {
      for (EEditorWidget val : EEditorWidget::_values())
      {
        if (EEditorWidget::eSceneNodeWidget != val._to_integral() &&
            EEditorWidget::eProjectSettings != val._to_integral())
        {
          qobject_cast<QListView*>(m_ParentWidget->RightComboBox()->view())
              ->setRowHidden(val, true);
        }
      }
      m_ParentWidget->RightComboBox()->setEnabled(true);
    } break;
    case ETutorialState::eNodePanel:
    {
      m_ParentWidget->EditorModel()->SetScriptTypeFilterForNewScripts("^\\*\\.(?!eos$).*$");
      for (EEditorWidget val : EEditorWidget::_values())
      {
        qobject_cast<QListView*>(m_ParentWidget->RightComboBox()->view())
            ->setRowHidden(val, false);
      }
    } // fallthrough
    case ETutorialState::eNodePanelAdvanced: // fallthrough
      if (ETutorialState::eNodePanelAdvanced == newState._to_integral())
      {
        m_ParentWidget->EditorModel()->SetScriptTypeFilterForNewScripts(".*");
      }
    // fallthrough
    case ETutorialState::eNodePanelDone:
    {
      bool bOk = QMetaObject::invokeMethod(this, "SlotSwitchRightPanel",
                                           Qt::QueuedConnection,
                                           Q_ARG(qint32, EEditorWidget::eSceneNodeWidget));
      assert(bOk);
      Q_UNUSED(bOk);

      m_ParentWidget->LeftComboBox()->setEnabled(false);
      m_ParentWidget->RightComboBox()->setEnabled(newState._to_integral() == ETutorialState::eNodePanelDone);
      m_ParentWidget->LeftGroupBox()->setEnabled(false);
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

      m_ParentWidget->LeftComboBox()->setEnabled(false);
      m_ParentWidget->RightComboBox()->setEnabled(false);
      m_ParentWidget->LeftGroupBox()->setEnabled(true);
    } break;
    case ETutorialState::eFinished:
    {
      if (ETutorialState::eCodePanelEOS == oldState._to_integral())
      {
        m_ParentWidget->LeftComboBox()->setEnabled(true);
        m_ParentWidget->RightComboBox()->setEnabled(true);
        m_ParentWidget->LeftGroupBox()->setEnabled(true);
        m_ParentWidget->RightGroupBox()->setEnabled(true);

        QTimer::singleShot(500, m_pTutorialOverlay, SLOT(Hide()));
      }
    } break;
  default: break;
  }
}

//----------------------------------------------------------------------------------------
//
void CClassicTutorialStateSwitchHandler::SlotSwitchLeftPanel(qint32 iNewIndex)
{
  qint32 iIndex = m_ParentWidget->LeftComboBox()->findData(iNewIndex);
  m_ParentWidget->LeftComboBox()->setCurrentIndex(iIndex);
}

//----------------------------------------------------------------------------------------
//
void CClassicTutorialStateSwitchHandler::SlotSwitchRightPanel(qint32 iNewIndex)
{
  qint32 iIndex = m_ParentWidget->RightComboBox()->findData(iNewIndex);
  m_ParentWidget->RightComboBox()->setCurrentIndex(iIndex);
}

//----------------------------------------------------------------------------------------
//
void CClassicTutorialStateSwitchHandler::SlotRightPanelSwitched(qint32 iNewIndex)
{
  if (ETutorialState::eSwitchRightPanelToProjectSettings == m_currentState._to_integral() &&
      EEditorWidget::eProjectSettings == iNewIndex)
  {
    bool bOk = QMetaObject::invokeMethod(this, "SlotOverlayNextInstructionTriggered",
                                         Qt::QueuedConnection);
    assert(bOk);
    Q_UNUSED(bOk);
  }
  else if (ETutorialState::eSwitchRightPanelToNodeSettings == m_currentState._to_integral() &&
           EEditorWidget::eSceneNodeWidget == iNewIndex)
  {
    bool bOk = QMetaObject::invokeMethod(this, "SlotOverlayNextInstructionTriggered",
                                         Qt::QueuedConnection);
    assert(bOk);
    Q_UNUSED(bOk);
  }
  else if (ETutorialState::eNodePanelDone == m_currentState._to_integral() &&
           EEditorWidget::eSceneCodeEditorWidget == iNewIndex)
  {
    bool bOk = QMetaObject::invokeMethod(this, "SlotOverlayNextInstructionTriggered",
                                         Qt::QueuedConnection);
    assert(bOk);
    Q_UNUSED(bOk);
  }
}
