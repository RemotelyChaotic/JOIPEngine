#include "EditorMainScreen.h"
#include "Application.h"
#include "Backend/DatabaseManager.h"
#include "Backend/Project.h"
#include "ui_EditorMainScreen.h"

CEditorMainScreen::CEditorMainScreen(QWidget* pParent) :
  QWidget(pParent),
  m_spUi(std::make_unique<Ui::CEditorMainScreen>()),
  m_spCurrentProject(nullptr),
  m_wpDbManager(),
  m_bInitialized(false)
{
  m_spUi->setupUi(this);
  Initialize();
}

CEditorMainScreen::~CEditorMainScreen()
{
}

//----------------------------------------------------------------------------------------
//
void CEditorMainScreen::Initialize()
{
  m_bInitialized = false;

  m_wpDbManager = CApplication::Instance()->System<CDatabaseManager>();

  m_bInitialized = true;
}

//----------------------------------------------------------------------------------------
//
void CEditorMainScreen::InitNewProject(const QString& sNewProjectName)
{
  if (!m_bInitialized) { return; }
  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    spDbManager->AddProject(sNewProjectName);
    m_spCurrentProject = spDbManager->FindProject(sNewProjectName);

    m_spCurrentProject->m_rwLock.lockForRead();
    qint32 iId = m_spCurrentProject->m_iId;
    m_spCurrentProject->m_rwLock.unlock();
    LoadProject(iId);

    // save to create folder structure
    spDbManager->SerializeProject(iId);
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorMainScreen::LoadProject(qint32 iId)
{
  if (!m_bInitialized) { return; }
  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    m_spCurrentProject = spDbManager->FindProject(iId);
  }

  m_spUi->pResourceWidget->LoadProject(m_spCurrentProject);
}

//----------------------------------------------------------------------------------------
//
void CEditorMainScreen::UnloadProject()
{
  if (!m_bInitialized) { return; }

  m_spCurrentProject = nullptr;

  m_spUi->pResourceWidget->UnloadProject();
}
