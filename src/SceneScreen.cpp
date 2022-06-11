#include "SceneScreen.h"
#include "Application.h"
#include "Settings.h"
#include "WindowContext.h"
#include "Systems/DatabaseManager.h"
#include "Systems/HelpFactory.h"
#include "Systems/Project.h"
#include "Widgets/HelpOverlay.h"
#include "ui_SceneScreen.h"

namespace
{
  const qint32 c_iPageIndexChoice = 0;
  const qint32 c_iPageIndexScene = 1;

  const QString c_sOpenHelpId = "Player/Open";
  const QString c_sCancelHelpId = "MainScreen/Cancel";
}

CSceneScreen::CSceneScreen(const std::shared_ptr<CWindowContext>& spWindowContext,
                                             QWidget* pParent) :
  QWidget(pParent),
  IAppStateScreen(spWindowContext),
  m_spUi(std::make_unique<Ui::CSceneScreen>()),
  m_wpDbManager()
{
  m_spUi->setupUi(this);
  Initialize();
}

CSceneScreen::~CSceneScreen()
{
}

//----------------------------------------------------------------------------------------
//
void CSceneScreen::Initialize()
{
  m_bInitialized = false;

  m_wpDbManager = CApplication::Instance()->System<CDatabaseManager>();

  auto wpHelpFactory = CApplication::Instance()->System<CHelpFactory>().lock();
  if (nullptr != wpHelpFactory)
  {
    m_spUi->pOpenExistingProjectButton->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sOpenHelpId);
    wpHelpFactory->RegisterHelp(c_sOpenHelpId, ":/resources/help/player/open_button_help.html");
    m_spUi->pCancelButton->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sCancelHelpId);
    wpHelpFactory->RegisterHelp(c_sCancelHelpId, ":/resources/help/cancel_button_help.html");
  }

  connect(m_spUi->pMainSceneScreen, &CSceneMainScreen::SignalExitClicked,
          this, &CSceneScreen::SlotExitClicked, Qt::DirectConnection);
  connect(m_spUi->pMainSceneScreen, &CSceneMainScreen::SignalUnloadFinished,
          this, &CSceneScreen::SlotSceneUnloadFinished, Qt::DirectConnection);
  connect(m_spUi->pProjectCardSelectionWidget, &CProjectCardSelectionWidget::SignalUnloadFinished,
          this, &CSceneScreen::SlotCardsUnloadFinished);

  m_spUi->pStackedWidget->setCurrentIndex(c_iPageIndexChoice);

  m_bInitialized = true;
}

//----------------------------------------------------------------------------------------
//
void CSceneScreen::Load()
{
  m_spUi->pProjectCardSelectionWidget->LoadProjects(EDownloadStateFlag::eFinished);
}

//----------------------------------------------------------------------------------------
//
void CSceneScreen::Unload()
{
  m_spUi->pProjectCardSelectionWidget->UnloadProjects();
}

//----------------------------------------------------------------------------------------
//
void CSceneScreen::on_pOpenExistingProjectButton_clicked()
{
  WIDGET_INITIALIZED_GUARD

  qint32 iId = -1;
  tspProject spProject = nullptr;
  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    iId = m_spUi->pProjectCardSelectionWidget->SelectedId();
    spProject = spDbManager->FindProject(iId);
    if (nullptr == spProject)
    {
      return;
    }
  }

  QReadLocker locker(&spProject->m_rwLock);
  if (spProject->m_bUsesWeb && CApplication::Instance()->Settings()->Offline())
  {
    m_spUi->pProjectCardSelectionWidget->ShowWarning(
          tr("Can't open JOIP-Project that requires online resources in offline mode in the player."));
  }
  else
  {
    locker.unlock();

    // will eventually emit a signal but we can ignore that
    m_spUi->pProjectCardSelectionWidget->UnloadProjects();

    emit m_spWindowContext->SignalSetDownloadButtonVisible(false);
    emit m_spWindowContext->SignalSetHelpButtonVisible(false);

    m_spUi->pMainSceneScreen->LoadProject(iId);
    m_spUi->pStackedWidget->setCurrentIndex(c_iPageIndexScene);
  }
}

//----------------------------------------------------------------------------------------
//
void CSceneScreen::on_pCancelButton_clicked()
{
  WIDGET_INITIALIZED_GUARD
  m_spUi->pStackedWidget->setCurrentIndex(c_iPageIndexChoice);
  emit m_spWindowContext->SignalChangeAppState(EAppState::eMainScreen);
}

//----------------------------------------------------------------------------------------
//
void CSceneScreen::SlotExitClicked()
{
  WIDGET_INITIALIZED_GUARD
  m_spUi->pStackedWidget->setCurrentIndex(c_iPageIndexChoice);
  emit m_spWindowContext->SignalChangeAppState(EAppState::eMainScreen);
}

//----------------------------------------------------------------------------------------
//
void CSceneScreen::SlotSceneUnloadFinished()
{
  WIDGET_INITIALIZED_GUARD
  emit UnloadFinished();
}

//----------------------------------------------------------------------------------------
//
void CSceneScreen::SlotCardsUnloadFinished()
{
  WIDGET_INITIALIZED_GUARD
  emit UnloadFinished();
}
