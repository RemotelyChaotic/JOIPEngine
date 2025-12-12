#include "ProjectSceneManager.h"

#include "Systems/Player/SceneNodeResolver.h"
#include "Systems/Database/Scene.h"

CProjectSceneManagerWrapper::CProjectSceneManagerWrapper(QObject* pParent) :
  CProjectEventTargetWrapper(pParent),
  m_wpSceneNodeResolver()
{

}
CProjectSceneManagerWrapper::~CProjectSceneManagerWrapper()
{

}

//----------------------------------------------------------------------------------------
//
void CProjectSceneManagerWrapper::Dispatched(const QString&)
{
  // nothing to do
}

//----------------------------------------------------------------------------------------
//
QString CProjectSceneManagerWrapper::EventTarget()
{
  return "CProjectSceneManagerWrapper";
}

//----------------------------------------------------------------------------------------
//
void CProjectSceneManagerWrapper::Initalize(std::weak_ptr<CSceneNodeResolver> wpSceneNodeResolver,
                                            std::weak_ptr<CProjectEventCallbackRegistry> wpRegistry)
{
  m_wpSceneNodeResolver = wpSceneNodeResolver;
  InitializeEventRegistry(wpRegistry);
  if (auto spRegistry = m_wpRegistry.lock())
  {
    spRegistry->AddDispatchTarget({EventTarget(), "change"}, this);
  }
}

//----------------------------------------------------------------------------------------
//
bool CProjectSceneManagerWrapper::contains(QString sScene)
{
  if (auto spResolver = m_wpSceneNodeResolver.lock())
  {
    return spResolver->AllScenes().contains(sScene);
  }
  return false;
}

//----------------------------------------------------------------------------------------
//
void CProjectSceneManagerWrapper::disable(QString sScene)
{
  if (auto spResolver = m_wpSceneNodeResolver.lock())
  {
    spResolver->DisableScene(sScene);
  }
}

//----------------------------------------------------------------------------------------
//
void CProjectSceneManagerWrapper::enable(QString sScene)
{
  if (auto spResolver = m_wpSceneNodeResolver.lock())
  {
    spResolver->EnableScene(sScene);
  }
}

//----------------------------------------------------------------------------------------
//
bool CProjectSceneManagerWrapper::isEnabled(QString sScene)
{
  if (auto spResolver = m_wpSceneNodeResolver.lock())
  {
    return spResolver->IsSceneEnabled(sScene);
  }
  return true;
}

//----------------------------------------------------------------------------------------
//
QString CProjectSceneManagerWrapper::getCurrentSceneId()
{
  if (auto spResolver = m_wpSceneNodeResolver.lock())
  {
    tspScene spScene = spResolver->CurrentScene();
    QReadLocker locker(&spScene->m_rwLock);
    return spScene->m_sName;
  }
  return QString();
}

//----------------------------------------------------------------------------------------
//
void CProjectSceneManagerWrapper::gotoScene(QString sScene)
{
  if (auto spResolver = m_wpSceneNodeResolver.lock())
  {
    emit spResolver->SignalChangeSceneRequest(sScene, true);
  }
}
