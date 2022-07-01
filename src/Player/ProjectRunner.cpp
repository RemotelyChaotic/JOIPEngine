#include "ProjectRunner.h"
#include "Application.h"
#include "Editor/NodeEditor/EndNodeModel.h"
#include "Editor/NodeEditor/FlowScene.h"
#include "Editor/NodeEditor/FlowView.h"
#include "Editor/NodeEditor/PathMergerModel.h"
#include "Editor/NodeEditor/PathSplitterModel.h"
#include "Editor/NodeEditor/NodeEditorRegistry.h"
#include "Editor/NodeEditor/SceneNodeModel.h"
#include "Editor/NodeEditor/SceneTranstitionData.h"
#include "Editor/NodeEditor/StartNodeModel.h"
#include "Systems/DatabaseManager.h"
#include "Systems/Project.h"
#include "Systems/Resource.h"
#include "Systems/Scene.h"

#include <nodes/Node>
#include <nodes/NodeData>

#include <QDebug>
#include <QFile>

#include <random>
#include <chrono>

using QtNodes::Node;

//----------------------------------------------------------------------------------------
//
CProjectRunner::CProjectRunner(QObject* pParent) :
  QObject (pParent),
  m_spCurrentProject(nullptr),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>()),
  m_nodeMap(),
  m_disabledScenes(),
  m_pFlowScene(nullptr),
  m_pCurrentNode(nullptr)
{

}

CProjectRunner::~CProjectRunner()
{
  UnloadProject();
  if (nullptr != m_pFlowScene)
  {
    m_pFlowScene->clearScene();
    delete m_pFlowScene;
    m_pFlowScene = nullptr;
  }
}

//----------------------------------------------------------------------------------------
//
bool CProjectRunner::MightBeRegexScene(const QString& sName)
{
  return sName.contains('+') || sName.contains('*') || sName.contains('|') || sName.contains('{') ||
         sName.contains('}') || sName.contains('[') || sName.contains(']');
}

//----------------------------------------------------------------------------------------
//
void CProjectRunner::LoadProject(tspProject spProject, const QString sStartScene)
{
  QString sError;
  if (nullptr != m_spCurrentProject)
  {
    assert(false);
    sError = tr("Old Project was not unloaded before loading project.");
    qWarning() << sError;
    emit SignalError(sError, QtMsgType::QtCriticalMsg);
  }

  m_spCurrentProject = spProject;

  if (nullptr != m_pFlowScene)
  {
    m_pFlowScene->clearScene();
    delete m_pFlowScene;
  }
  m_pFlowScene = new CFlowScene(CNodeEditorRegistry::RegisterDataModels());
  connect(m_pFlowScene, &CFlowScene::nodeCreated, this, &CProjectRunner::SlotNodeCreated);

  bool bOk = LoadFlowScene();
  assert(bOk && "Could not load Flow scene. Why????");
  if (!bOk)
  {
    sError = tr("Could not load Flow scene.");
    qWarning() << sError;
    emit SignalError(sError, QtMsgType::QtCriticalMsg);
    return;
  }

  bOk = ResolveStart(sStartScene);
  assert(bOk && "Starting scene could not be resolved.");
  if (!bOk)
  {
    sError = QString(tr("Starting scene '%1' could not be resolved.")).arg(sStartScene);
    qWarning() << sError;
    emit SignalError(sError, QtMsgType::QtCriticalMsg);
    return;
  }

  CSceneNodeModel* pNodeDataModel = dynamic_cast<CSceneNodeModel*>(m_pCurrentNode->nodeDataModel());
  if (!sStartScene.isNull() && !sStartScene.isEmpty() && nullptr != pNodeDataModel)
  {
    m_nodeMap.clear();
    m_nodeMap.insert({pNodeDataModel->SceneName(), m_pCurrentNode});
    return;
  }

  bOk = ResolveNextScene();
  if (!bOk)
  {
    sError = tr("Could not resolve next scene.");
    qWarning() << sError;
    emit SignalError(sError, QtMsgType::QtCriticalMsg);
    return;
  }
}

//----------------------------------------------------------------------------------------
//
void CProjectRunner::UnloadProject()
{
  m_pCurrentNode = nullptr;
  m_spCurrentProject = nullptr;
  m_spCurrentScene = nullptr;
  m_nodeMap.clear();
  m_disabledScenes.clear();
  if (nullptr != m_pFlowScene)
  {
    m_pFlowScene->clearScene();
    delete m_pFlowScene;
    m_pFlowScene = nullptr;
  }
}

//----------------------------------------------------------------------------------------
//
tspScene CProjectRunner::CurrentScene() const
{
  return m_spCurrentScene;
}

//----------------------------------------------------------------------------------------
//
void CProjectRunner::DisableScene(const QString& sScene)
{
  m_disabledScenes.insert(sScene);
}

//----------------------------------------------------------------------------------------
//
void CProjectRunner::EnableScene(const QString& sScene)
{
  auto it = m_disabledScenes.find(sScene);
  if (m_disabledScenes.end() != it)
  {
    m_disabledScenes.erase(it);
  }
}

//----------------------------------------------------------------------------------------
//
bool CProjectRunner::IsSceneEnabled(const QString& sScene) const
{
  auto it = m_disabledScenes.find(sScene);
  return m_disabledScenes.end() == it;
}

//----------------------------------------------------------------------------------------
//
tspScene CProjectRunner::NextScene(const QString sName)
{
  auto it = m_nodeMap.find(sName);
  if (m_nodeMap.end() != it)
  {
    CSceneNodeModel* pSceneModel =
      dynamic_cast<CSceneNodeModel*>(it->second->nodeDataModel());

    if (nullptr != pSceneModel)
    {
      m_pCurrentNode = it->second;
      qint32 iId = pSceneModel->SceneId();
      auto spDbManager = m_wpDbManager.lock();
      if (nullptr != spDbManager)
      {
        tspScene spScene = spDbManager->FindScene(m_spCurrentProject, iId);
        if (nullptr != spScene)
        {
          m_spCurrentScene = spScene;
          return spScene;
        }
      }
    }
  }
  return nullptr;
}

//----------------------------------------------------------------------------------------
//
QStringList CProjectRunner::PossibleScenes()
{
  QStringList out;
  for (auto it = m_nodeMap.begin(); m_nodeMap.end() != it; ++it)
  {
    out << it->first;
  }
  return out;
}

//----------------------------------------------------------------------------------------
//
void CProjectRunner::ResolveFindScenes(const QString sName)
{
  if (nullptr == m_pFlowScene)
  {
    QString sError(tr("Internal error."));
    qWarning() << sError;
    emit SignalError(sError, QtMsgType::QtCriticalMsg);
    return;
  }

  m_nodeMap.clear();

  auto vpNodes = m_pFlowScene->allNodes();
  for (auto pNode : vpNodes)
  {
    CSceneNodeModel* pSceneModel = dynamic_cast<CSceneNodeModel*>(pNode->nodeDataModel());
    if (nullptr != pSceneModel && !sName.isNull() && !sName.isEmpty())
    {
      // could it be a regexp expression?
      if (MightBeRegexScene(sName))
      {
        QRegExp rx(sName);
        qint32 iPos = 0;
        if ((iPos = rx.indexIn(pSceneModel->SceneName(), iPos)) != -1)
        {
          auto it = m_disabledScenes.find(pSceneModel->SceneName());
          if (m_disabledScenes.end() == it)
          {
            m_nodeMap.insert({pSceneModel->SceneName(), pNode});
          }
        }
      }
      else
      {
        if (pSceneModel->SceneName() == sName)
        {
          auto it = m_disabledScenes.find(pSceneModel->SceneName());
          if (m_disabledScenes.end() == it)
          {
            m_nodeMap.insert({pSceneModel->SceneName(), pNode});
          }
        }
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CProjectRunner::ResolveScenes()
{
  ResolveNextScene();
}

//----------------------------------------------------------------------------------------
//
bool CProjectRunner::LoadFlowScene()
{
  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    QReadLocker projectLocker(&m_spCurrentProject->m_rwLock);
    if (!m_spCurrentProject->m_sSceneModel.isNull() &&
        !m_spCurrentProject->m_sSceneModel.isEmpty())
    {
      const QString sModelName = m_spCurrentProject->m_sSceneModel;
      projectLocker.unlock();

      auto spResource = spDbManager->FindResourceInProject(m_spCurrentProject, sModelName);
      if (nullptr != spResource)
      {
        QString sResourceBundle;
        QString sPath = ResourceUrlToAbsolutePath(spResource);
        {
          QReadLocker locker(&spResource->m_rwLock);
          sResourceBundle = spResource->m_sResourceBundle;
        }
        CDatabaseManager::LoadBundle(m_spCurrentProject, sResourceBundle);
        QFile modelFile(sPath);
        if (modelFile.open(QIODevice::ReadOnly))
        {
          QByteArray arr = modelFile.readAll();
          m_pFlowScene->loadFromMemory(arr);
        }
        else
        {
          QString sError(tr("Could not open scene model file: %1.").arg(modelFile.errorString()));
          qWarning() << sError;
          emit SignalError(sError, QtMsgType::QtWarningMsg);
          return false;
        }
      }
    }
    else
    {
      QString sError(tr("Could not open scene model file: scene not found."));
      qWarning() << sError;
      emit SignalError(sError, QtMsgType::QtWarningMsg);
      return false;
    }
  }
  else
  {
    return false;
  }
  return true;
}

//----------------------------------------------------------------------------------------
//
void CProjectRunner::ResolveNextPossibleNodes(Node* pNode, std::vector<std::pair<QString, QtNodes::Node*>>& vpRet)
{
  if (nullptr == pNode) { return; }

  quint32 iOutPorts = pNode->nodeDataModel()->nPorts(PortType::Out);

  CPathSplitterModel* pSplitterModel =
    dynamic_cast<CPathSplitterModel*>(pNode->nodeDataModel());

  if (pSplitterModel == nullptr ||
      pSplitterModel->TransitionType()._to_integral() == ESceneTransitionType::eSelection)
  {
    for (quint32 i = 0; iOutPorts > i; i++)
    {
      auto pConnectionsMap =
          pNode->nodeState().connections(PortType::Out, static_cast<qint32>(i));
      for (auto it = pConnectionsMap.begin(); pConnectionsMap.end() != it; ++it)
      {
        QtNodes::Node* pNextNode = it->second->getNode(PortType::In);
        CSceneNodeModel* pSceneModel =
          dynamic_cast<CSceneNodeModel*>(pNextNode->nodeDataModel());
        CEndNodeModel* pEndModel =
          dynamic_cast<CEndNodeModel*>(pNextNode->nodeDataModel());

        if (nullptr != pSceneModel || nullptr != pEndModel)
        {
          // splitter path name or scene name
          if (nullptr != pSplitterModel)
          {
            vpRet.push_back({pSplitterModel->TransitionLabel(static_cast<qint32>(i)), pNextNode});
          }
          else if(nullptr != pEndModel)
          {
            vpRet.push_back({"End", pNextNode});
          }
          else
          {
            vpRet.push_back({pSceneModel->SceneName(), pNextNode});
          }
        }
        else
        {
          ResolveNextPossibleNodes(pNextNode, vpRet);
        }
      }
    }
  }
  else
  {
    // check which maps lead to anything
    std::vector<quint32> viValidIndicees;
    for (quint32 i = 0; iOutPorts > i; i++)
    {
      auto pConnectionsMap =
          pNode->nodeState().connections(PortType::Out, static_cast<qint32>(i));
      if (pConnectionsMap.size() > 0)
      {
        viValidIndicees.push_back(i);
      }
    }

    long unsigned int seed =
      static_cast<long unsigned int>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    std::mt19937 generator(static_cast<long unsigned int>(seed));
    std::uniform_int_distribution<> dis(0, static_cast<qint32>(viValidIndicees.size() - 1));
    qint32 iGeneratedIndex = dis(generator);
    qDebug() << "Seed:" << seed << "casted:" << static_cast<long unsigned int>(seed);
    qDebug() << "Generated value:" << iGeneratedIndex << "from 0 -" << static_cast<qint32>(viValidIndicees.size() - 1);

    auto pConnectionsMap =
        pNode->nodeState().connections(PortType::Out,
                                       static_cast<qint32>(viValidIndicees[static_cast<quint32>(iGeneratedIndex)]));
    for (auto it = pConnectionsMap.begin(); pConnectionsMap.end() != it; ++it)
    {
      QtNodes::Node* pNextNode = it->second->getNode(PortType::In);
      CSceneNodeModel* pSceneModel =
        dynamic_cast<CSceneNodeModel*>(pNextNode->nodeDataModel());
      CEndNodeModel* pEndModel =
        dynamic_cast<CEndNodeModel*>(pNextNode->nodeDataModel());

      if (nullptr != pSceneModel || nullptr != pEndModel)
      {
        vpRet.push_back({pSplitterModel->TransitionLabel(static_cast<qint32>(iGeneratedIndex)), pNextNode});
      }
      else
      {
        ResolveNextPossibleNodes(pNextNode, vpRet);
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
bool CProjectRunner::ResolveNextScene()
{
  if (nullptr == m_pFlowScene || nullptr == m_pCurrentNode)
  {
    QString sError(tr("Internal error."));
    qWarning() << sError;
    emit SignalError(sError, QtMsgType::QtCriticalMsg);
    return false;
  }

  m_nodeMap.clear();
  std::vector<std::pair<QString, Node*>> vpNodes;
  ResolveNextPossibleNodes(m_pCurrentNode, vpNodes);
  if (vpNodes.size() > 0)
  {
    for (auto pNode : vpNodes)
    {
      CSceneNodeModel* pSceneModel =
        dynamic_cast<CSceneNodeModel*>(pNode.second->nodeDataModel());
      CEndNodeModel* pEndModel =
        dynamic_cast<CEndNodeModel*>(pNode.second->nodeDataModel());

      if (nullptr != pSceneModel)
      {
        qint32 iId = pSceneModel->SceneId();
        auto spDbManager = m_wpDbManager.lock();
        if (nullptr != spDbManager)
        {
          tspScene spScene = spDbManager->FindScene(m_spCurrentProject, iId);
          if (nullptr != spScene)
          {
            auto it = m_disabledScenes.find(pNode.first);
            if (m_disabledScenes.end() == it)
            {
              m_nodeMap.insert({pNode.first, pNode.second});
            }
          }
        }
      }
      else if (nullptr != pEndModel)
      {
        m_nodeMap["End"] = pNode.second;
      }
    }
  }
  else
  {
    QString sError(tr("More than one nodes attached to start."));
    qWarning() << sError;
    emit SignalError(sError, QtMsgType::QtCriticalMsg);
    return false;
  }

  return true;
}

//----------------------------------------------------------------------------------------
//
bool CProjectRunner::ResolveStart(const QString sStartScene)
{
  if (nullptr == m_pFlowScene)
  {
    QString sError(tr("Internal error."));
    qWarning() << sError;
    emit SignalError(sError, QtMsgType::QtCriticalMsg);
    return false;
  }

  bool bFound = false;
  QStringList vsFoundNames;
  auto vpNodes = m_pFlowScene->allNodes();
  for (auto pNode : vpNodes)
  {
    CSceneNodeModel* pSceneModel = dynamic_cast<CSceneNodeModel*>(pNode->nodeDataModel());
    CStartNodeModel* pStartModel = dynamic_cast<CStartNodeModel*>(pNode->nodeDataModel());
    if (nullptr != pSceneModel && !sStartScene.isEmpty())
    {
      if (pSceneModel->SceneName() == sStartScene)
      {
        vsFoundNames.push_back(pSceneModel->SceneName());
        bFound = true;
        m_pCurrentNode = pNode;
      }
    }
    if (nullptr != pStartModel)
    {
      if (!bFound)
      {
        vsFoundNames.push_back(pStartModel->Name());
        bFound = true;
        m_pCurrentNode = pNode;
      }
    }
  }

  if (!bFound)
  {
    if (vsFoundNames.size() > 0)
    {
      QString sError = QString(tr("Multiple entry points in project: %1 (%2)"))
                       .arg(vsFoundNames.size()).arg(vsFoundNames.join(", "));
      qWarning() << sError;
      emit SignalError(sError, QtMsgType::QtCriticalMsg);
      return false;
    }
    else
    {
      QString sError = QString(tr("No entry points in project."));
      qWarning() << sError;
      emit SignalError(sError, QtMsgType::QtCriticalMsg);
      return false;
    }
  }

  return bFound;
}

//----------------------------------------------------------------------------------------
//
void CProjectRunner::SlotNodeCreated(QtNodes::Node &n)
{
  auto spDbManager = m_wpDbManager.lock();
  CSceneNodeModel* pSceneModel = dynamic_cast<CSceneNodeModel*>(n.nodeDataModel());
  if (nullptr != pSceneModel && nullptr != spDbManager)
  {
    m_spCurrentProject->m_rwLock.lockForRead();
    qint32 iId = m_spCurrentProject->m_iId;
    m_spCurrentProject->m_rwLock.unlock();
    pSceneModel->SetProjectId(iId);
  }
}
