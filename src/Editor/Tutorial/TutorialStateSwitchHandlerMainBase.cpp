#include "TutorialStateSwitchHandlerMainBase.h"
#include "CommandBackground.h"
#include "CommandClickFilter.h"
#include "CommandEnd.h"
#include "CommandHighlight.h"
#include "CommandText.h"
#include "Systems/JSON/JsonInstructionSetParser.h"
#include "Systems/JSON/JsonInstructionSetRunner.h"
#include <QDebug>
#include <QFile>
#include <QTimer>

CTutorialStateSwitchHandlerMainBase::CTutorialStateSwitchHandlerMainBase(
    QPointer<CEditorTutorialOverlay> pTutorialOverlay,
    const QString& sTutorial) :
  QObject(nullptr),
  ITutorialStateSwitchHandler(),
  m_spTutorialParser(std::make_unique<CJsonInstructionSetParser>()),
  m_spTutorialRunner(nullptr),
  m_pTutorialOverlay(pTutorialOverlay),
  m_currentState(ETutorialState::eFinished)
{
  assert(nullptr != m_pTutorialOverlay);

  QFile schemaFile(":/resources/help/tutorial/TutorialScheme.json");
  QFile tutorialFile(sTutorial);
  if (schemaFile.open(QIODevice::ReadOnly) && tutorialFile.open(QIODevice::ReadOnly))
  {
    m_spTutorialParser->SetJsonBaseSchema(schemaFile.readAll());
    m_spTutorialParser->RegisterInstructionSetPath("Tutorial", "/");

    m_spTutorialParser->RegisterInstruction("background", std::make_shared<CCommandBackground>(m_pTutorialOverlay));
    m_spTutorialParser->RegisterInstruction("clickFilter", std::make_shared<CCommandClickFilter>(m_pTutorialOverlay));
    m_spTutorialParser->RegisterInstruction("highlight",  std::make_shared<CCommandHighlight>(m_pTutorialOverlay));
    m_spTutorialParser->RegisterInstruction("text",  std::make_shared<CCommandText>(m_pTutorialOverlay));
    m_spTutorialParser->RegisterInstruction("end",  std::make_shared<CCommandEnd>());

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

CTutorialStateSwitchHandlerMainBase::~CTutorialStateSwitchHandlerMainBase()
{

}

//----------------------------------------------------------------------------------------
//
void CTutorialStateSwitchHandlerMainBase::OnStateSwitch(ETutorialState newState,
                                                        ETutorialState oldState)
{
  m_currentState = newState;

  if (oldState._to_integral() == ETutorialState::eUnstarted)
  {
    // deleay showing a bit, so the user can see the UI pop up
    QTimer::singleShot(500, m_pTutorialOverlay, SLOT(Show()));
  }

  OnStateSwitchImpl(newState, oldState);

  // run json script for the new state
  if (nullptr != m_spTutorialRunner)
  {
    m_spTutorialRunner->Run(newState._to_string(), ERunerMode::eRunOne, true);
  }
}

//----------------------------------------------------------------------------------------
//
void CTutorialStateSwitchHandlerMainBase::OnResetStates()
{
  m_currentState = ETutorialState::eFinished;
  m_pTutorialOverlay->Hide();
}

//----------------------------------------------------------------------------------------
//
void CTutorialStateSwitchHandlerMainBase::SlotOverlayNextInstructionTriggered()
{
  if (nullptr != m_spTutorialRunner)
  {
    CJsonInstructionSetRunner::tRetVal retVal = m_spTutorialRunner->CallNextCommand(ERunerMode::eRunOne, true);
    if (!std::holds_alternative<SJsonException>(retVal))
    {
      if (!std::get<SRunnerRetVal>(retVal).m_bHasMoreCommands)
      {
        m_pTutorialOverlay->NextTutorialState();
      }
    }
    else
    {
      SJsonException exception = std::get<SJsonException>(retVal);
      QString sException = exception.m_sException;
      QString sError = "Uncaught exception " + sException;
      qWarning() << sError;
    }
  }
}
