#include "MainScreenTutorialStateSwitchHandler.h"
#include "CommandBackground.h"
#include "CommandClickFilter.h"
#include "CommandHighlight.h"
#include "CommandText.h"
#include "EditorTutorialOverlay.h"
#include "Editor/EditorMainScreen.h"
#include "Systems/JSON/JsonInstructionSetParser.h"
#include "Systems/JSON/JsonInstructionSetRunner.h"
#include <QDebug>
#include <QFile>
#include <QListView>
#include <QTimer>

CMainScreenTutorialStateSwitchHandler::CMainScreenTutorialStateSwitchHandler(
    QPointer<CEditorMainScreen> pParentWidget,
    const std::shared_ptr<Ui::CEditorMainScreen>& spUi,
    QPointer<CEditorTutorialOverlay> pTutorialOverlay) :
  QObject(nullptr),
  ITutorialStateSwitchHandler(),
  m_spTutorialParser(std::make_unique<CJsonInstructionSetParser>()),
  m_spUi(spUi),
  m_spTutorialRunner(nullptr),
  m_ParentWidget(pParentWidget),
  m_pTutorialOverlay(pTutorialOverlay),
  m_currentState(ETutorialState::eFinished)
{
  connect(m_spUi->pRightComboBox, qOverload<qint32>(&QComboBox::currentIndexChanged),
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
          qobject_cast<QListView*>(m_spUi->pRightComboBox->view())
              ->setRowHidden(val, true);
        }
      }
    } break;
    case ETutorialState::eProjectSettings:
    {
      for (EEditorWidget val : EEditorWidget::_values())
      {
        qobject_cast<QListView*>(m_spUi->pRightComboBox->view())
            ->setRowHidden(val, false);
      }

      bool bOk = QMetaObject::invokeMethod(this, "SlotSwitchRightPanel",
                                           Qt::QueuedConnection,
                                           Q_ARG(qint32, EEditorWidget::eProjectSettings));
      assert(bOk);
      Q_UNUSED(bOk);

      m_spUi->pLeftComboBox->setEnabled(false);
      m_spUi->pRightComboBox->setEnabled(false);
      m_spUi->pLeftPanelGroupBox->setEnabled(false);
    } break;
    case ETutorialState::eResourcePanel:
    {
      m_spUi->pLeftComboBox->setEnabled(false);
      m_spUi->pRightComboBox->setEnabled(false);
      m_spUi->pLeftPanelGroupBox->setEnabled(true);
    } break;
    case ETutorialState::eImageResourceSelected:
    {
      m_spUi->pLeftComboBox->setEnabled(false);
      m_spUi->pRightComboBox->setEnabled(false);
    } break;
    case ETutorialState::eSwitchRightPanelToNodeSettings:
    {
      for (EEditorWidget val : EEditorWidget::_values())
      {
        if (EEditorWidget::eSceneNodeWidget != val._to_integral() &&
            EEditorWidget::eProjectSettings != val._to_integral())
        {
          qobject_cast<QListView*>(m_spUi->pRightComboBox->view())
              ->setRowHidden(val, true);
        }
      }
      m_spUi->pRightComboBox->setEnabled(true);
    } break;
    case ETutorialState::eNodePanel:
    {
      for (EEditorWidget val : EEditorWidget::_values())
      {
        qobject_cast<QListView*>(m_spUi->pRightComboBox->view())
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

      m_spUi->pLeftComboBox->setEnabled(false);
      m_spUi->pRightComboBox->setEnabled(newState._to_integral() == ETutorialState::eNodePanelDone);
      m_spUi->pLeftPanelGroupBox->setEnabled(false);
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

      m_spUi->pLeftComboBox->setEnabled(false);
      m_spUi->pRightComboBox->setEnabled(false);
      m_spUi->pLeftPanelGroupBox->setEnabled(true);
    } break;
    case ETutorialState::eFinished:
    {
      if (ETutorialState::eCodePanel == oldState._to_integral())
      {
        m_spUi->pLeftComboBox->setEnabled(true);
        m_spUi->pRightComboBox->setEnabled(true);
        m_spUi->pLeftPanelGroupBox->setEnabled(true);
        m_spUi->pRightPanelGroupBox->setEnabled(true);

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
  m_spUi->pLeftComboBox->setCurrentIndex(iNewIndex);
}

//----------------------------------------------------------------------------------------
//
void CMainScreenTutorialStateSwitchHandler::SlotSwitchRightPanel(qint32 iNewIndex)
{
  m_spUi->pRightComboBox->setCurrentIndex(iNewIndex);
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
