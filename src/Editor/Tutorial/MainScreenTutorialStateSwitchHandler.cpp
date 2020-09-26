#include "MainScreenTutorialStateSwitchHandler.h"
#include "CommandBackground.h"
#include "CommandClickTransparency.h"
#include "CommandHighlight.h"
#include "CommandText.h"
#include "EditorTutorialOverlay.h"
#include "Editor/EditorMainScreen.h"
#include "Systems/JSON/JsonInstructionSetParser.h"
#include "Systems/JSON/JsonInstructionSetRunner.h"
#include <QDebug>
#include <QFile>
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
  m_pTutorialOverlay(pTutorialOverlay)
{
  connect(pTutorialOverlay, &CEditorTutorialOverlay::SignalOverlayNextInstructionTriggered,
          this, &CMainScreenTutorialStateSwitchHandler::SlotOverlayNextInstructionTriggered,
          Qt::QueuedConnection);

  QFile schemaFile(":/resources/data/TutorialScheme.json");
  QFile tutorialFile(":/resources/help/tutorial/Tutorial.json");
  if (schemaFile.open(QIODevice::ReadOnly) && tutorialFile.open(QIODevice::ReadOnly))
  {
    m_spTutorialParser->SetJsonBaseSchema(schemaFile.readAll());
    m_spTutorialParser->RegisterInstructionSetPath("Tutorial", "/");

    m_spTutorialParser->RegisterInstruction("background", std::make_shared<CCommandBackground>(m_pTutorialOverlay));
    m_spTutorialParser->RegisterInstruction("clickTransparency", std::make_shared<CCommandClickTransparency>(m_pTutorialOverlay));
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
  m_pTutorialOverlay->Hide();
}

//----------------------------------------------------------------------------------------
//
void CMainScreenTutorialStateSwitchHandler::OnStateSwitch(ETutorialState newState,
                                                          ETutorialState oldState)
{
  if (oldState._to_integral() == ETutorialState::eUnstarted)
  {
    // deleay showing a bit, so the user can see the UI pop up
    QTimer::singleShot(500, m_pTutorialOverlay, SLOT(Show()));
  }

  switch (newState)
  {
    case ETutorialState::eBeginTutorial:
    {

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
