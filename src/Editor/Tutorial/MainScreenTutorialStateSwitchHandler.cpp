#include "MainScreenTutorialStateSwitchHandler.h"
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
  ITutorialStateSwitchHandler(),
  m_spTutorialParser(std::make_unique<CJsonInstructionSetParser>()),
  m_spUi(spUi),
  m_spTutorialRunner(nullptr),
  m_ParentWidget(pParentWidget),
  m_pTutorialOverlay(pTutorialOverlay)
{
  QFile schemaFile(":/resources/data/TutorialScheme.json");
  QFile tutorialFile(":/resources/help/tutorial/Tutorial.json");
  if (schemaFile.open(QIODevice::ReadOnly) && tutorialFile.open(QIODevice::ReadOnly))
  {
    m_spTutorialParser->SetJsonBaseSchema(schemaFile.readAll());
    m_spTutorialRunner =
      m_spTutorialParser->ParseJson(tutorialFile.readAll());

    // Test:
    m_spTutorialRunner->Run();
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
}
