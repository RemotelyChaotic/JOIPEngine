#include "ProjectSceneManager.h"
#include "ProjectRunner.h"
#include "Systems/Scene.h"

CProjectSceneManagerWrapper::CProjectSceneManagerWrapper(QObject* pParent) :
  CProjectEventTargetWrapper(pParent),
  m_wpProjectRunner()
{

}
CProjectSceneManagerWrapper::~CProjectSceneManagerWrapper()
{

}

//----------------------------------------------------------------------------------------
//
void CProjectSceneManagerWrapper::Initalize(std::weak_ptr<CProjectRunner> wpProjectRunner,
                                            std::weak_ptr<CProjectEventCallbackRegistry> wpRegistry)
{
  m_wpProjectRunner = wpProjectRunner;
  InitializeEventRegistry(wpRegistry);
}

//----------------------------------------------------------------------------------------
//
void CProjectSceneManagerWrapper::disable(QString sScene)
{
  if (auto spRunner = m_wpProjectRunner.lock())
  {
    spRunner->DisableScene(sScene);
  }
}

//----------------------------------------------------------------------------------------
//
void CProjectSceneManagerWrapper::enable(QString sScene)
{
  if (auto spRunner = m_wpProjectRunner.lock())
  {
    spRunner->EnableScene(sScene);
  }
}

//----------------------------------------------------------------------------------------
//
bool CProjectSceneManagerWrapper::isEnabled(QString sScene)
{
  if (auto spRunner = m_wpProjectRunner.lock())
  {
    spRunner->IsSceneEnabled(sScene);
  }
  return true;
}

//----------------------------------------------------------------------------------------
//
QString CProjectSceneManagerWrapper::getCurrentSceneId()
{
  if (auto spRunner = m_wpProjectRunner.lock())
  {
    tspScene spScene = spRunner->CurrentScene();
    QReadLocker locker(&spScene->m_rwLock);
    return spScene->m_sName;
  }
  return QString();
}

//----------------------------------------------------------------------------------------
//
void CProjectSceneManagerWrapper::gotoScene(QString sScene)
{
  if (auto spRunner = m_wpProjectRunner.lock())
  {
    spRunner->SignalChangeSceneRequest(sScene);
  }
}
