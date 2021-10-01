#include "MainScreenTutorialStateSwitchHandler.h"
#include "CommandBackground.h"
#include "CommandClickFilter.h"
#include "CommandHighlight.h"
#include "CommandText.h"
#include "EditorTutorialOverlay.h"
#include "Editor/EditorMainScreen.h"
#include "Editor/EditorLayouts/EditorLayoutClassic.h"
#include "Systems/JSON/JsonInstructionSetParser.h"
#include "Systems/JSON/JsonInstructionSetRunner.h"
#include <QComboBox>
#include <QDebug>
#include <QFile>
#include <QListView>
#include <QTimer>

CMainScreenTutorialStateSwitchHandler::CMainScreenTutorialStateSwitchHandler(
    QPointer<CEditorLayoutClassic> pParentWidget,
    QPointer<CEditorTutorialOverlay> pTutorialOverlay) :
  QObject(nullptr),
  ITutorialStateSwitchHandler(),
  m_spTutorialParser(std::make_unique<CJsonInstructionSetParser>()),
  m_spTutorialRunner(nullptr),
  m_ParentWidget(pParentWidget),
  m_pTutorialOverlay(pTutorialOverlay),
  m_currentState(ETutorialState::eFinished)
{
  connect(m_ParentWidget->RightComboBox(), qOverload<qint32>(&QComboBox::currentIndexChanged),
          this, &CMainScreenTutorialStateSwitchHandler::SlotRightPanelSwitched);
  connect(pTutorialOverlay, &CEditorTutorialOverlay::SignalOverlayNextInstructionTriggered,
          this, &CMainScreenTutorialStateSwitchHandler::SlotOverlayNextInstructionTriggered,
          Qt::QueuedConnection);

  QFile schemaFile(":/resources/help/tutorial/TutorialScheme.json");
  QFile tutorialFile(":/resources/help/tutorial/Tutorial.json");
  if (schemaFile.open(QIODevice::ReadOnly) && tutorialFile.open(QIODevice::ReadOnly))
  {
    m_spTutorialParser->SetJsonBaseSchema(schemaFile.readAll());
    m_spTutorialParser->RegisterInstructionSetPath("Tutorial", "/");

    m_spTutorialParser->RegisterInstruction("background", std::make_shared<CCommandBackground>(m_pTutorialOverlay));
    m_spTutorialParser->RegisterInstruction("clickFilter", std::make_shared<CCommandClickFilter>(m_pTutorialOverlay));
    m_spTutorialParser->RegisterInstruction("highlight",  std::make_shared<CCommandHighlight>(m_pTutorialOverlay));
    m_spTutorialParser->RegisterInstruction("text",  std::make_shared<CCommandText>(m_pTutorialOverlay));

    m_spTutorialRunner =
      m_spTutorialParser->ParseJson(tutorialFile.readAll());

    if (nullptr == m_spTutorialRunner)
    {
      qCritical() << QT_TR_NOOP("Could not create tutorial JSON runner object.");
    }
  }
  else
  {
    qCritical() << QT_TR_NOOP("Could not open tutorial files.");
  }
}

CMainScreenTutorialStateSwitchHandler::~CMainScreenTutorialStateSwitchHandler()
{

}

//----------------------------------------------------------------------------------------
//
void CMainScreenTutorialStateSwitchHandler::OnResetStates()
{
  m_currentState = ETutorialState::eFinished;
  m_pTutorialOverlay->Hide();
}

//----------------------------------------------------------------------------------------
//
void CMainScreenTutorialStateSwitchHandler::OnStateSwitch(ETutorialState newState,
                                                          ETutorialState oldState)
{
  m_currentState = newState;

  if (oldState._to_integral() == ETutorialState::eUnstarted)
  {
    // deleay showing a bit, so the user can see the UI pop up
    QTimer::singleShot(500, m_pTutorialOverlay, SLOT(Show()));
  }

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
      for (EEditorWidget val : EEditorWidget::_values())
      {
        qobject_cast<QListView*>(m_ParentWidget->RightComboBox()->view())
            ->setRowHidden(val, false);
      }
    } // fallthrough
    case ETutorialState::eNodePanelAdvanced: // fallthrough
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
      if (ETutorialState::eCodePanel == oldState._to_integral())
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

  // run json script for the new state
  if (nullptr != m_spTutorialRunner)
  {
    m_spTutorialRunner->Run(newState._to_string());
  }
}

//----------------------------------------------------------------------------------------
//
void CMainScreenTutorialStateSwitchHandler::SlotSwitchLeftPanel(qint32 iNewIndex)
{
  m_ParentWidget->LeftComboBox()->setCurrentIndex(iNewIndex);
}

//----------------------------------------------------------------------------------------
//
void CMainScreenTutorialStateSwitchHandler::SlotSwitchRightPanel(qint32 iNewIndex)
{
  m_ParentWidget->RightComboBox()->setCurrentIndex(iNewIndex);
}

//----------------------------------------------------------------------------------------
//
void CMainScreenTutorialStateSwitchHandler::SlotRightPanelSwitched(qint32 iNewIndex)
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

//----------------------------------------------------------------------------------------
//
void CMainScreenTutorialStateSwitchHandler::SlotOverlayNextInstructionTriggered()
{
  if (nullptr != m_spTutorialRunner)
  {
    if (!m_spTutorialRunner->CallNextCommand())
    {
      m_pTutorialOverlay->NextTutorialState();
    }
  }
}
