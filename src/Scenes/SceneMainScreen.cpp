#include "SceneMainScreen.h"
#include "Application.h"
#include "ProjectRunner.h"
#include "Settings.h"
#include "Backend/DatabaseManager.h"
#include "Backend/Project.h"
#include "ui_SceneMainScreen.h"

CSceneMainScreen::CSceneMainScreen(QWidget* pParent) :
  QWidget(pParent),
  m_spUi(std::make_unique<Ui::CSceneMainScreen>()),
  m_spProjectRunner(std::make_unique<CProjectRunner>()),
  m_spSettings(CApplication::Instance()->Settings()),
  m_spCurrentProject(nullptr),
  m_wpDbManager(),
  m_bInitialized(false)
{
  m_spUi->setupUi(this);
  Initialize();
}

CSceneMainScreen::~CSceneMainScreen()
{
}

//----------------------------------------------------------------------------------------
//
void CSceneMainScreen::Initialize()
{
  m_bInitialized = false;

  m_wpDbManager = CApplication::Instance()->System<CDatabaseManager>();

  m_spUi->pInfoDisplay->Initialize();
  m_spUi->pTextBoxDisplay->Initialize();
  m_spUi->pTimerDisplay->Initialize();

  // initializing done
  m_bInitialized = true;
}

//----------------------------------------------------------------------------------------
//
void CSceneMainScreen::LoadProject(qint32 iId)
{
  if (!m_bInitialized) { return; }
  if (nullptr != m_spCurrentProject)
  {
    qWarning() << "Old Project was not unloaded before loading project.";
  }

  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    m_spCurrentProject = spDbManager->FindProject(iId);
    m_spProjectRunner->LoadProject(m_spCurrentProject);
  }
}

//----------------------------------------------------------------------------------------
//
void CSceneMainScreen::UnloadProject()
{
  if (!m_bInitialized) { return; }

  m_spProjectRunner->UnloadProject();
  m_spCurrentProject = nullptr;
}
