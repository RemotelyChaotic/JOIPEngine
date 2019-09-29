#include "SceneScreen.h"
#include "Application.h"
#include "WindowContext.h"
#include "Backend/DatabaseManager.h"
#include "ui_SceneScreen.h"

namespace
{
  const qint32 c_iPageIndexChoice = 0;
  const qint32 c_iPageIndexScene = 1;
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

  connect(m_spUi->pMainSceneScreen, &CSceneMainScreen::SignalExitClicked,
          this, &CSceneScreen::SlotExitClicked, Qt::DirectConnection);

  m_spUi->pStackedWidget->setCurrentIndex(c_iPageIndexChoice);

  m_bInitialized = true;
}

//----------------------------------------------------------------------------------------
//
void CSceneScreen::Load()
{
  m_spUi->pProjectCardSelectionWidget->LoadProjects();
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
  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    iId = m_spUi->pProjectCardSelectionWidget->SelectedId();
    if (spDbManager->FindProject(iId) == nullptr)
    {
      return;
    }
  }

  m_spUi->pMainSceneScreen->UnloadProject();
  m_spUi->pMainSceneScreen->LoadProject(iId);
  m_spUi->pStackedWidget->setCurrentIndex(c_iPageIndexScene);
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
