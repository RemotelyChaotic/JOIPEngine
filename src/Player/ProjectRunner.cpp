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
void ResolveNextPossibleNodes(qint32 iDepth, Node* pNode, const std::set<QString>& disabledScenes,
                              std::vector<NodeResolveReslt>& vpRet)
{
  if (nullptr == pNode) { return; }

  quint32 iOutPorts = pNode->nodeDataModel()->nPorts(PortType::Out);

  CPathSplitterModel* pSplitterModel =
    dynamic_cast<CPathSplitterModel*>(pNode->nodeDataModel());

  // selection transition, merger or a scene of sorts
  if (pSplitterModel == nullptr ||
      pSplitterModel->TransitionType()._to_integral() == ESceneTransitionType::eSelection)
  {
    // iterate out-ports
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

        // we have a scene or end
        if (nullptr != pSceneModel || nullptr != pEndModel)
        {
          // splitter path name or scene name
          // push resolver
          if (nullptr != pSplitterModel)
          {
            std::optional<QString> optCustomTransition = pSplitterModel->CustomTransition();
            vpRet.push_back(
                  NodeResolveReslt{pSplitterModel->TransitionLabel(static_cast<qint32>(i)),
                                   pNextNode, iDepth, optCustomTransition.has_value(),
                                   optCustomTransition.value_or(QString())});
          }
          // push end
          else if(nullptr != pEndModel)
          {
            vpRet.push_back(NodeResolveReslt{"End", pNextNode, iDepth, false, QString()});
          }
          // only push enabled scenes
          else if (disabledScenes.end() == disabledScenes.find(pSceneModel->SceneName()))
          {
            vpRet.push_back(NodeResolveReslt{pSceneModel->SceneName(), pNextNode, iDepth, false, QString()});
          }
        }
        // we have a merger
        else
        {
          ResolveNextPossibleNodes(iDepth+1, pNextNode, disabledScenes, vpRet);
        }
      }
    }
  }
  // random selection
  else
  {
    // pre-check which ports lead to anything
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
    qDebug() << "Seed:" << seed << "casted:" << static_cast<long unsigned int>(seed);

    // Do a second pass and check the following nodes. If empty, remove from the possible list.
    std::vector<std::pair<qint32, std::vector<NodeResolveReslt>>> vResult2Pass;
    for (qint32 iValidIndex : viValidIndicees)
    {
      std::vector<NodeResolveReslt> vpCachedRet;
      auto pConnectionsMap =
          pNode->nodeState().connections(PortType::Out, iValidIndex);
      for (auto it = pConnectionsMap.begin(); pConnectionsMap.end() != it; ++it)
      {
        QtNodes::Node* pNextNode = it->second->getNode(PortType::In);
        CSceneNodeModel* pSceneModel =
          dynamic_cast<CSceneNodeModel*>(pNextNode->nodeDataModel());
        CEndNodeModel* pEndModel =
          dynamic_cast<CEndNodeModel*>(pNextNode->nodeDataModel());

        // we found an end model
        if (nullptr != pEndModel)
        {
          vpCachedRet.push_back(
              NodeResolveReslt{"End", pNextNode, iDepth, false, QString()});
        }
        // we found a scene model, only insert if not disabled
        else if (nullptr != pSceneModel)
        {
          if (disabledScenes.end() == disabledScenes.find(pSceneModel->SceneName()))
          {
            vpCachedRet.push_back(
                NodeResolveReslt{pSplitterModel->TransitionLabel(iValidIndex),
                                 pNextNode, iDepth, false, QString()});
          }
        }
        // might be a merger or splitter, iterate recursively
        else
        {
          ResolveNextPossibleNodes(iDepth+1, pNextNode, disabledScenes, vpCachedRet);
        }
      }

      if (!vpCachedRet.empty())
      {
        vResult2Pass.push_back({iValidIndex, vpCachedRet});
      }
    }

    std::uniform_int_distribution<> dis(0, static_cast<qint32>(vResult2Pass.size() - 1));
    qint32 iGeneratedIndex = dis(generator);
    qDebug() << "Generated value:" << iGeneratedIndex << "from 0 -" << static_cast<qint32>(vResult2Pass.size() - 1);

    auto vRolledRes = vResult2Pass[static_cast<size_t>(iGeneratedIndex)].second;
    vpRet.insert(vpRet.end(), vRolledRes.begin(), vRolledRes.end());
  }
}

//----------------------------------------------------------------------------------------
//
QStringList GetFirstUnresolvedNodes(std::vector<NodeResolveReslt>& resolveResult,
                                    std::optional<QString>* unresolvedData)
{
  if (nullptr != unresolvedData)
  {
    *unresolvedData = std::nullopt;
  }

  QStringList vsUnresolved;
  qint32 iLowestDepthOfUnresolved = INT_MAX;

  // find the unresolved nodes with the lowest depth
  for (const auto& node : resolveResult)
  {
    if (iLowestDepthOfUnresolved > node.m_iDepth && node.m_bNeedsUserResolvement)
    {
      iLowestDepthOfUnresolved = node.m_iDepth;
      vsUnresolved.clear();
    }

    if (iLowestDepthOfUnresolved == node.m_iDepth && node.m_bNeedsUserResolvement)
    {
      vsUnresolved.push_back(node.m_sLabel);
      if (nullptr != unresolvedData)
      {
        *unresolvedData = node.m_sResolvementData;
      }
    }
  }

  return vsUnresolved;
}

//----------------------------------------------------------------------------------------
//
void ResolveNodes(std::vector<NodeResolveReslt>& resolveResult, const QStringList& vsNodes,
                  qint32 iResolve, const std::set<QString>& disabledScenes)
{
  QStringList vsToResolve = vsNodes;

  // find the unresolved nodes with the lowest depth
  while (vsToResolve.size() > 0)
  {
    auto it = std::find_if(resolveResult.begin(), resolveResult.end(),
                           [s = vsToResolve.front()](const NodeResolveReslt& node) {
      return node.m_sLabel == s;
    });
    if (resolveResult.end() != it)
    {
      NodeResolveReslt node = *it;
      resolveResult.erase(it);

      if (0 == iResolve)
      {
        CSceneNodeModel* pSceneModel =
          dynamic_cast<CSceneNodeModel*>(node.m_pNode->nodeDataModel());
        CEndNodeModel* pEndModel =
          dynamic_cast<CEndNodeModel*>(node.m_pNode->nodeDataModel());
        if (nullptr != pSceneModel)
        {
          resolveResult.push_back(NodeResolveReslt{pSceneModel->SceneName(), node.m_pNode,
                                                   node.m_iDepth+1, false, QString()});
        }
        else if (nullptr != pEndModel)
        {
          resolveResult.push_back(NodeResolveReslt{"End", node.m_pNode, node.m_iDepth+1,
                                                   false, QString()});
        }
        else
        {
          ResolveNextPossibleNodes(node.m_iDepth+1, node.m_pNode, disabledScenes, resolveResult);
        }
      }

      --iResolve;
    }

    vsToResolve.erase(vsToResolve.begin());
  }
}


//----------------------------------------------------------------------------------------
//
CProjectRunner::CProjectRunner(QObject* pParent) :
  QObject (pParent),
  m_spCurrentProject(nullptr),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>()),
  m_resolveResult(),
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
  bool bOk = Setup(spProject, sStartScene);
  QString sError;
  if (!bOk)
  {
    return;
  }

  CSceneNodeModel* pNodeDataModel = dynamic_cast<CSceneNodeModel*>(m_pCurrentNode->nodeDataModel());
  if (!sStartScene.isEmpty() && nullptr != pNodeDataModel)
  {
    m_nodeMap.clear();
    m_resolveResult.clear();
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
void CProjectRunner::LoadProject(tspProject spProject, tspScene spStartScene)
{
  bool bOk = Setup(spProject, QString());
  QString sError;

  if (!bOk)
  {
    return;
  }

  m_nodeMap.clear();
  m_resolveResult.clear();

  if (nullptr == spStartScene)
  {
    sError = tr("Injected scene is null.");
    qWarning() << sError;
    emit SignalError(sError, QtMsgType::QtCriticalMsg);
    return;
  }

  m_spInjectedScene = spStartScene;
}

//----------------------------------------------------------------------------------------
//
void CProjectRunner::UnloadProject()
{
  m_pCurrentNode = nullptr;
  m_spCurrentProject = nullptr;
  m_spInjectedScene = nullptr;
  m_spCurrentScene = nullptr;
  m_nodeMap.clear();
  m_resolveResult.clear();
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
  // we found a scene, so clear resolve cache
  m_resolveResult.clear();

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
  else if (nullptr != m_spInjectedScene)
  {
    QReadLocker l(&m_spInjectedScene->m_rwLock);
    if (m_spInjectedScene->m_sName == sName)
    {
      if (!ResolveStart(QString()))
      {
        return nullptr;
      }
      m_spCurrentScene = m_spInjectedScene;
      return m_spInjectedScene;
    }
  }

  return nullptr;
}

//----------------------------------------------------------------------------------------
//
QStringList CProjectRunner::PossibleScenes(std::optional<QString>* unresolvedData)
{
  QStringList vsUnresolved =
      GetFirstUnresolvedNodes(m_resolveResult, unresolvedData);
  if (!vsUnresolved.isEmpty())
  {
    return vsUnresolved;
  }

  if (nullptr != unresolvedData) { *unresolvedData = std::nullopt; }
  QStringList out;
  for (auto it = m_nodeMap.begin(); m_nodeMap.end() != it; ++it)
  {
    out << it->first;
  }
  if (nullptr != m_spInjectedScene)
  {
    QReadLocker locker(&m_spInjectedScene->m_rwLock);
    out << m_spInjectedScene->m_sName;
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
void CProjectRunner::ResolvePossibleScenes(const QStringList vsNames, qint32 iIndex)
{
  ResolveNodes(m_resolveResult, vsNames, iIndex, m_disabledScenes);
  m_nodeMap.clear();
  GenerateNodesFromResolved();
}

//----------------------------------------------------------------------------------------
//
void CProjectRunner::ResolveScenes()
{
  ResolveNextScene();
}

//----------------------------------------------------------------------------------------
//
bool CProjectRunner::GenerateNodesFromResolved()
{
  if (m_resolveResult.size() > 0)
  {
    for (auto pNode : m_resolveResult)
    {
      CSceneNodeModel* pSceneModel =
        dynamic_cast<CSceneNodeModel*>(pNode.m_pNode->nodeDataModel());
      CEndNodeModel* pEndModel =
        dynamic_cast<CEndNodeModel*>(pNode.m_pNode->nodeDataModel());

      if (nullptr != pSceneModel)
      {
        qint32 iId = pSceneModel->SceneId();
        auto spDbManager = m_wpDbManager.lock();
        if (nullptr != spDbManager)
        {
          tspScene spScene = spDbManager->FindScene(m_spCurrentProject, iId);
          if (nullptr != spScene)
          {
            auto it = m_disabledScenes.find(pNode.m_sLabel);
            if (m_disabledScenes.end() == it)
            {
              m_nodeMap.insert({pNode.m_sLabel, pNode.m_pNode});
            }
          }
        }
      }
      else if (nullptr != pEndModel)
      {
        m_nodeMap["End"] = pNode.m_pNode;
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
bool CProjectRunner::Setup(tspProject spProject, const QString sStartScene)
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
    return false;
  }

  bOk = ResolveStart(sStartScene);
  assert(bOk && "Starting scene could not be resolved.");
  if (!bOk)
  {
    sError = QString(tr("Starting scene '%1' could not be resolved.")).arg(sStartScene);
    qWarning() << sError;
    emit SignalError(sError, QtMsgType::QtCriticalMsg);
    return false;
  }

  return bOk;
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
  ResolveNextPossibleNodes(0, m_pCurrentNode, m_disabledScenes, m_resolveResult);
  return GenerateNodesFromResolved();
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
  if (nullptr != spDbManager)
  {
    m_spCurrentProject->m_rwLock.lockForRead();
    qint32 iId = m_spCurrentProject->m_iId;
    m_spCurrentProject->m_rwLock.unlock();

    CSceneNodeModel* pSceneModel = dynamic_cast<CSceneNodeModel*>(n.nodeDataModel());
    if (nullptr != pSceneModel)
    {
      pSceneModel->SetProjectId(iId);
    }
    CPathSplitterModel* pPathSplitterModel = dynamic_cast<CPathSplitterModel*>(n.nodeDataModel());
    if (nullptr != pPathSplitterModel)
    {
      pPathSplitterModel->SetProjectId(iId);
    }
  }
}
