#include "SceneNodeResolver.h"
#include "Application.h"
#include "Systems/Nodes/EndNodeModel.h"
#include "Systems/Nodes/FlowScene.h"
#include "Systems/Nodes/FlowView.h"
#include "Systems/Nodes/PathMergerModel.h"
#include "Systems/Nodes/PathSplitterModel.h"
#include "Systems/Nodes/NodeGraphicsObjectProvider.h"
#include "Systems/Nodes/SceneNodeModel.h"
#include "Systems/Nodes/SceneTranstitionData.h"
#include "Systems/Nodes/StartNodeModel.h"

#include "Systems/DatabaseManager.h"
#include "Systems/Database/Project.h"
#include "Systems/Database/Resource.h"
#include "Systems/Database/Scene.h"

#include <nodes/DataModelRegistry>
#include <nodes/Node>
#include <nodes/NodeData>

#include <QDebug>
#include <QFile>

#include <random>
#include <chrono>

using QtNodes::Node;

namespace
{
  const char c_sEndNode[] = "~End";
}

//----------------------------------------------------------------------------------------
//
bool IsSelectableTransiation(Node const * const pNode)
{
  CPathSplitterModel* pSplitterModel =
      dynamic_cast<CPathSplitterModel*>(pNode->nodeDataModel());
  return nullptr != pSplitterModel &&
         pSplitterModel->TransitionType()._to_integral() == ESceneTransitionType::eSelection;
}

//----------------------------------------------------------------------------------------
//
void ResolveNextPossibleNodes(qint32 iDepth, Node const * const pNode,
                              const QString& sDefaultTransitionName,
                              const std::set<QString>& disabledScenes,
                              std::vector<NodeResolveReslt>& vpRet,
                              std::shared_ptr<IResolverDebugger> spDebugger)
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
            if (nullptr != spDebugger)
            {
              spDebugger->PushNode(pNode, pNextNode,
                                   IResolverDebugger::NodeData{true, IsSelectableTransiation(pNextNode),
                                                               pSplitterModel->TransitionLabel(static_cast<qint32>(i)),
                                                               static_cast<qint32>(i)});
            }
          }
          // push end
          else if(nullptr != pEndModel)
          {
            vpRet.push_back(NodeResolveReslt{c_sEndNode, pNextNode, iDepth, false, QString()});
            if (nullptr != spDebugger)
            {
              spDebugger->PushNode(pNode, pNextNode,
                                   IResolverDebugger::NodeData{true, false,
                                                               c_sEndNode,
                                                               static_cast<qint32>(i)});
            }
          }
          // only push enabled scenes
          else if (disabledScenes.end() == disabledScenes.find(pSceneModel->SceneName()))
          {
            vpRet.push_back(NodeResolveReslt{sDefaultTransitionName.isEmpty() ?
                                                 pSceneModel->SceneName() : sDefaultTransitionName,
                                             pNextNode, iDepth, false, QString()});
            if (nullptr != spDebugger)
            {
              spDebugger->PushNode(pNode, pNextNode,
                                   IResolverDebugger::NodeData{true, false, pSceneModel->SceneName(),
                                                               static_cast<qint32>(i)});
            }
          }
          // for debugging purposes
          else
          {
            if (nullptr != spDebugger)
            {
              spDebugger->PushNode(pNode, pNextNode,
                                   IResolverDebugger::NodeData{false, false, pSceneModel->SceneName(),
                                                               static_cast<qint32>(i)});
            }
          }
        }
        // we have a merger or splitter next, iterate recursively
        else
        {
          if (nullptr != spDebugger)
          {
            QString sLabel;
            bool bSelection = false;
            if (nullptr == pNextNode)
            {
              sLabel = "Empty";
            }
            else if (nullptr != dynamic_cast<CPathSplitterModel*>(pNextNode->nodeDataModel()))
            {
              if (nullptr != pSplitterModel)
              {
                sLabel = pSplitterModel->TransitionLabel(static_cast<qint32>(i));
              }
              else
              {
                sLabel = "Splitter";
              }
              bSelection = IsSelectableTransiation(pNextNode);
            }
            else
            {
              sLabel = "Merger";
            }
            spDebugger->PushNode(pNode, pNextNode,
                                 IResolverDebugger::NodeData{nullptr != pNextNode, bSelection,
                                                             sLabel, static_cast<qint32>(i)});
          }

          QString sDefaultTransitionLabel = sDefaultTransitionName;
          if (nullptr != pSplitterModel)
          {
            sDefaultTransitionLabel = pSplitterModel->TransitionLabel(static_cast<qint32>(i));
          }

          std::vector<NodeResolveReslt> vpCachedRet;
          ResolveNextPossibleNodes(iDepth+1, pNextNode, sDefaultTransitionLabel,
                                   disabledScenes, vpCachedRet, spDebugger);

          // if we have a Splitter here, we migth need user resolvement
          if (nullptr != pSplitterModel)
          {
            std::optional<QString> optCustomTransition = pSplitterModel->CustomTransition();
            bool bPushThisNodeAsToResolve = false;
            for (auto& ret : vpCachedRet)
            {
              if (ret.m_bNeedsUserResolvement)
              {
                bPushThisNodeAsToResolve = true;
              }
            }
            if (bPushThisNodeAsToResolve)
            {
              vpRet.push_back(NodeResolveReslt{pSplitterModel->TransitionLabel(static_cast<qint32>(i)),
                                               pNextNode, iDepth,
                                               optCustomTransition.has_value(),
                                               optCustomTransition.value_or(QString())});
            }
            for (auto& ret : vpCachedRet)
            {
              if (!ret.m_bNeedsUserResolvement)
              {
                ret.m_iDepth--;
              }
              if (ret.m_iDepth == iDepth && !ret.m_bNeedsUserResolvement)
              {
                ret.m_bNeedsUserResolvement = optCustomTransition.has_value();
                ret.m_sResolvementData = optCustomTransition.value_or(QString());
              }
              if (nullptr != spDebugger)
              {
                auto pBlock = spDebugger->DataBlock(ret.m_pNode);
                assert(nullptr != pBlock);
                if (nullptr != pBlock)
                {
                  pBlock->bSelection = ret.m_bNeedsUserResolvement;
                }
              }
            }
          }
          else
          {
            for (auto& ret : vpCachedRet)
            {
              if (!ret.m_bNeedsUserResolvement)
              {
                ret.m_iDepth--;
              }
            }
          }
          vpRet.insert(vpRet.end(), vpCachedRet.begin(), vpCachedRet.end());
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
    //qDebug() << "Seed:" << seed << "casted:" << static_cast<long unsigned int>(seed);

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
              NodeResolveReslt{c_sEndNode, pNextNode, iDepth, false, QString()});
          if (nullptr != spDebugger)
          {
            spDebugger->PushNode(pNode, pNextNode,
                                 IResolverDebugger::NodeData{true, false, c_sEndNode,
                                                             iValidIndex});
          }
        }
        // we found a scene model, only insert if not disabled
        else if (nullptr != pSceneModel)
        {
          if (disabledScenes.end() == disabledScenes.find(pSceneModel->SceneName()))
          {
            vpCachedRet.push_back(
                NodeResolveReslt{pSplitterModel->TransitionLabel(iValidIndex),
                                 pNextNode, iDepth, false, QString()});
            if (nullptr != spDebugger)
            {
              spDebugger->PushNode(pNode, pNextNode,
                                   IResolverDebugger::NodeData{true, false,
                                                               pSplitterModel->TransitionLabel(iValidIndex),
                                                               iValidIndex});
            }
          }
          else
          {
            if (nullptr != spDebugger)
            {
              spDebugger->PushNode(pNode, pNextNode,
                                   IResolverDebugger::NodeData{false, false,
                                                               pSplitterModel->TransitionLabel(iValidIndex),
                                                               iValidIndex});
            }
          }
        }
        // we have a merger or splitter next, iterate recursively
        else
        {
          if (nullptr != spDebugger)
          {
            QString sLabel;
            bool bSelection = false;
            if (nullptr == pNextNode)
            {
              sLabel = "Empty";
            }
            else if (nullptr != dynamic_cast<CPathSplitterModel*>(pNextNode->nodeDataModel()))
            {
              if (nullptr != pSplitterModel)
              {
                sLabel = pSplitterModel->TransitionLabel(iValidIndex);
              }
              else
              {
                sLabel = "Splitter";
              }
              bSelection = IsSelectableTransiation(pNextNode);
            }
            else
            {
              sLabel = "Merger";
            }
            spDebugger->PushNode(pNode, pNextNode,
                                 IResolverDebugger::NodeData{nullptr != pNextNode, bSelection,
                                                             sLabel, iValidIndex});
          }
          QString sDefaultTransitionLabel = sDefaultTransitionName;
          if (nullptr != pSplitterModel)
          {
            sDefaultTransitionLabel = pSplitterModel->TransitionLabel(iValidIndex);
          }
          ResolveNextPossibleNodes(iDepth+1, pNextNode, sDefaultTransitionLabel, disabledScenes, vpCachedRet, spDebugger);
        }
      }

      if (!vpCachedRet.empty())
      {
        vResult2Pass.push_back({iValidIndex, vpCachedRet});
      }
    }

    // do we have results?
    if (!vResult2Pass.empty())
    {
      std::uniform_int_distribution<> dis(0, static_cast<qint32>(vResult2Pass.size() - 1));
      qint32 iGeneratedIndex = dis(generator);
      //qDebug() << "Generated value:" << iGeneratedIndex << "from 0 -" << static_cast<qint32>(vResult2Pass.size() - 1);

      auto vRolledRes = vResult2Pass[static_cast<size_t>(iGeneratedIndex)].second;
      for (auto& ret : vRolledRes)
      {
        ret.m_iDepth--;
      }
      vpRet.insert(vpRet.end(), vRolledRes.begin(), vRolledRes.end());

      if (nullptr != spDebugger)
      {
        auto vpChildDataBlocks = spDebugger->ChildBlocks(pNode);
        for (size_t iBlock = 0; vpChildDataBlocks.size() != iBlock; ++iBlock)
        {
          auto pBlock = vpChildDataBlocks[static_cast<size_t>(iBlock)];
          pBlock->bEnabled = pBlock->iPortIndex == vResult2Pass[static_cast<size_t>(iGeneratedIndex)].first;
        }
      }
    }
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
                  qint32 iResolve, const std::set<QString>& disabledScenes,
                  std::shared_ptr<IResolverDebugger> spDebugger)
{
  Q_UNUSED(disabledScenes)
  QStringList vsToResolve = vsNodes;

  std::optional<NodeResolveReslt> resolved = std::nullopt;

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
        // splitters and mergers are dummies here, so ignore
        CPathSplitterModel* pSplitterModel =
          dynamic_cast<CPathSplitterModel*>(node.m_pNode->nodeDataModel());
        CPathMergerModel* pMerger =
            dynamic_cast<CPathMergerModel*>(node.m_pNode->nodeDataModel());
        if (nullptr == pSplitterModel && nullptr == pMerger)
        {
          resolved = node;
        }

        if (nullptr != spDebugger)
        {
          spDebugger->ResolveTo(node.m_pNode);
        }
      }

      --iResolve;
    }

    vsToResolve.erase(vsToResolve.begin());
  }

  qint32 iNextDepth = INT_MAX;
  bool bNeedsResolvement = false;
  QString sResolvementData;
  for (const auto& res : resolveResult)
  {
    if (iNextDepth > res.m_iDepth)
    {
      iNextDepth = res.m_iDepth;
      bNeedsResolvement = res.m_bNeedsUserResolvement;
      sResolvementData = res.m_sResolvementData;
    }
  }

  if (resolved.has_value())
  {
    resolveResult.push_back(resolved.value());
    resolveResult.back().m_iDepth = iNextDepth;
    resolveResult.back().m_bNeedsUserResolvement = bNeedsResolvement;
    resolveResult.back().m_sResolvementData = sResolvementData;
  }
}

//----------------------------------------------------------------------------------------
//
IResolverDebugger::IResolverDebugger() {}
IResolverDebugger::~IResolverDebugger() = default;

//----------------------------------------------------------------------------------------
//
CSceneNodeResolver::CSceneNodeResolver(std::shared_ptr<QtNodes::DataModelRegistry> spRegistry,
                                       QObject* pParent) :
  QObject (pParent),
  m_spNodeModelRegistry(spRegistry),
  m_spCurrentProject(nullptr),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>()),
  m_resolveResult(),
  m_nodeMap(),
  m_disabledScenes(),
  m_pFlowScene(nullptr),
  m_pCurrentNode(nullptr)
{

}

CSceneNodeResolver::~CSceneNodeResolver()
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
bool CSceneNodeResolver::MightBeRegexScene(const QString& sName)
{
  return sName.contains('+') || sName.contains('*') || sName.contains('|') || sName.contains('{') ||
         sName.contains('}') || sName.contains('[') || sName.contains(']');
}

//----------------------------------------------------------------------------------------
//
void CSceneNodeResolver::AttatchDebugger(const std::weak_ptr<IResolverDebugger>& wpDebugger)
{
  m_wpDebugger = wpDebugger;
}

//----------------------------------------------------------------------------------------
//
void CSceneNodeResolver::LoadProject(tspProject spProject, const QUuid& nodeId)
{
  bool bOk = Setup(spProject, nodeId);
  QString sError;
  if (!bOk)
  {
    return;
  }

  CSceneNodeModel* pNodeDataModel = dynamic_cast<CSceneNodeModel*>(m_pCurrentNode->nodeDataModel());
  if (!nodeId.isNull() && nullptr != pNodeDataModel)
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
void CSceneNodeResolver::LoadProject(tspProject spProject, const QString& sStartScene)
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
void CSceneNodeResolver::LoadProject(tspProject spProject, const tspScene& spStartScene)
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
void CSceneNodeResolver::UnloadProject()
{
  if (auto spDebugger = m_wpDebugger.lock())
  {
    spDebugger->SetCurrentNode(nullptr);
  }
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
tspScene CSceneNodeResolver::CurrentScene() const
{
  return m_spCurrentScene;
}

//----------------------------------------------------------------------------------------
//
void CSceneNodeResolver::DisableScene(const QString& sScene)
{
  m_disabledScenes.insert(sScene);
}

//----------------------------------------------------------------------------------------
//
void CSceneNodeResolver::EnableScene(const QString& sScene)
{
  auto it = m_disabledScenes.find(sScene);
  if (m_disabledScenes.end() != it)
  {
    m_disabledScenes.erase(it);
  }
}

//----------------------------------------------------------------------------------------
//
bool CSceneNodeResolver::IsSceneEnabled(const QString& sScene) const
{
  auto it = m_disabledScenes.find(sScene);
  return m_disabledScenes.end() == it;
}

//----------------------------------------------------------------------------------------
//
tspScene CSceneNodeResolver::NextScene(const QString sName, bool* bEnd)
{
  // we found a scene, so clear resolve cache
  m_resolveResult.clear();

  if (nullptr != bEnd)
  {
    *bEnd = false;
  }

  auto it = m_nodeMap.find(sName);
  if (m_nodeMap.end() != it)
  {
    CSceneNodeModel* pSceneModel =
      dynamic_cast<CSceneNodeModel*>(it->second->nodeDataModel());
    CEndNodeModel* pEndNodeModel =
        dynamic_cast<CEndNodeModel*>(it->second->nodeDataModel());

    if (nullptr != pSceneModel)
    {
      m_pCurrentNode = it->second;
      if (auto spDebugger = m_wpDebugger.lock())
      {
        spDebugger->SetCurrentNode(m_pCurrentNode);
      }

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
    else if (nullptr != pEndNodeModel && nullptr != bEnd)
    {
      *bEnd = true;
      if (nullptr != pEndNodeModel)
      {
        m_pCurrentNode = it->second;
        if (auto spDebugger = m_wpDebugger.lock())
        {
          spDebugger->SetCurrentNode(m_pCurrentNode);
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
QStringList CSceneNodeResolver::PossibleScenes(std::optional<QString>* unresolvedData)
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
void CSceneNodeResolver::ResolveFindScenes(const QString sName)
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
void CSceneNodeResolver::ResolvePossibleScenes(const QStringList vsNames, qint32 iIndex)
{
  ResolveNodes(m_resolveResult, vsNames, iIndex, m_disabledScenes, m_wpDebugger.lock());
  m_nodeMap.clear();
  GenerateNodesFromResolved();
}

//----------------------------------------------------------------------------------------
//
void CSceneNodeResolver::ResolveScenes()
{
  ResolveNextScene();
}

//----------------------------------------------------------------------------------------
//
void CSceneNodeResolver::Error(const QString& sError, QtMsgType type)
{
  qWarning() << sError;
  if (auto spDebugger = m_wpDebugger.lock())
  {
    spDebugger->Error(sError, type);
  }
  emit SignalError(sError, type);
}

//----------------------------------------------------------------------------------------
//
bool CSceneNodeResolver::GenerateNodesFromResolved()
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
        m_nodeMap[c_sEndNode] = pNode.m_pNode;
      }
    }
  }
  else
  {
    Error(tr("Next node not found."), QtMsgType::QtCriticalMsg);
    return false;
  }
  return true;
}

//----------------------------------------------------------------------------------------
//
bool CSceneNodeResolver::Setup(tspProject spProject, const std::variant<QString, QUuid>& start)
{
  if (nullptr != m_spCurrentProject)
  {
    assert(false);
    Error(tr("Old Project was not unloaded before loading project."), QtMsgType::QtCriticalMsg);
  }

  m_spCurrentProject = spProject;

  if (nullptr != m_pFlowScene)
  {
    m_pFlowScene->clearScene();
    delete m_pFlowScene;
  }
  m_pFlowScene = new CFlowScene(m_spNodeModelRegistry,
                                std::make_shared<CDefaultGraphicsObjectProvider>());
  connect(m_pFlowScene, &CFlowScene::nodeCreated, this, &CSceneNodeResolver::SlotNodeCreated);

  bool bOk = LoadFlowScene();
  assert(bOk && "Could not load Flow scene. Why????");
  if (!bOk)
  {
    Error(tr("Could not load Flow scene."), QtMsgType::QtCriticalMsg);
    return false;
  }

  bOk = ResolveStart(start);
  assert(bOk && "Starting scene could not be resolved.");
  if (!bOk)
  {
    QString sError;
    if (std::holds_alternative<QString>(start))
    {
      sError = QString(tr("Starting scene '%1' could not be resolved."))
                   .arg(std::get<QString>(start));
    }
    else if (std::holds_alternative<QUuid>(start))
    {
      sError = QString(tr("Starting scene '%1' could not be resolved."))
                   .arg(std::get<QUuid>(start).toString());
    }
    Error(sError, QtMsgType::QtCriticalMsg);
    return false;
  }

  return bOk;
}

//----------------------------------------------------------------------------------------
//
bool CSceneNodeResolver::LoadFlowScene()
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
          Error(tr("Could not open scene model file: %1.").arg(modelFile.errorString()), QtMsgType::QtWarningMsg);
          return false;
        }
      }
    }
    else
    {
      Error(tr("Could not open scene model file: scene not found."), QtMsgType::QtWarningMsg);
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
bool CSceneNodeResolver::ResolveNextScene()
{
  if (nullptr == m_pFlowScene || nullptr == m_pCurrentNode)
  {
    Error(tr("Internal error."), QtMsgType::QtCriticalMsg);
    return false;
  }

  m_nodeMap.clear();
  ResolveNextPossibleNodes(0, m_pCurrentNode, QString(), m_disabledScenes, m_resolveResult, m_wpDebugger.lock());
  return GenerateNodesFromResolved();
}

//----------------------------------------------------------------------------------------
//
bool CSceneNodeResolver::ResolveStart(const std::variant<QString, QUuid>& start)
{
  if (nullptr == m_pFlowScene)
  {
    Error(tr("Internal error."), QtMsgType::QtCriticalMsg);
    return false;
  }

  bool bIsEmpty = false;
  if (std::holds_alternative<QString>(start))
  {
    bIsEmpty = std::get<QString>(start).isEmpty();
  }
  else if (std::holds_alternative<QUuid>(start))
  {
    bIsEmpty = std::get<QUuid>(start).isNull();
  }

  bool bFound = false;
  QStringList vsFoundNames;
  auto vpNodes = m_pFlowScene->allNodes();
  if (std::holds_alternative<QString>(start))
  {
    for (auto pNode : vpNodes)
    {
      CSceneNodeModel* pSceneModel = dynamic_cast<CSceneNodeModel*>(pNode->nodeDataModel());
      if (nullptr != pSceneModel && !bIsEmpty)
      {
        if (pSceneModel->SceneName() == std::get<QString>(start))
        {
          vsFoundNames.push_back(pSceneModel->SceneName());
          bFound = true;
          m_pCurrentNode = pNode;
          if (auto spDebugger = m_wpDebugger.lock())
          {
            spDebugger->SetCurrentNode(m_pCurrentNode);
          }
        }
      }
    }
    if (!bFound)
    {
      for (auto pNode : vpNodes)
      {
        CStartNodeModel* pStartModel = dynamic_cast<CStartNodeModel*>(pNode->nodeDataModel());
        if (nullptr != pStartModel)
        {
          if (!bFound)
          {
            vsFoundNames.push_back(pStartModel->Name());
            bFound = true;
            m_pCurrentNode = pNode;
            if (auto spDebugger = m_wpDebugger.lock())
            {
              spDebugger->SetCurrentNode(m_pCurrentNode);
            }
          }
        }
      }
    }
  }
  else if (std::holds_alternative<QUuid>(start))
  {
    for (auto pNode : vpNodes)
    {
      if (pNode->id() == std::get<QUuid>(start))
      {
        vsFoundNames.push_back(pNode->id().toString());
        bFound = true;
        m_pCurrentNode = pNode;
        if (auto spDebugger = m_wpDebugger.lock())
        {
          spDebugger->SetCurrentNode(m_pCurrentNode);
        }
      }
    }
  }

  if (!bFound)
  {
    if (vsFoundNames.size() > 0)
    {
      Error(tr("Multiple entry points in project: %1 (%2)")
              .arg(vsFoundNames.size()).arg(vsFoundNames.join(", ")), QtMsgType::QtCriticalMsg);
      return false;
    }
    else
    {
      Error(tr("No entry points in project."), QtMsgType::QtCriticalMsg);
      return false;
    }
  }

  return bFound;
}

//----------------------------------------------------------------------------------------
//
void CSceneNodeResolver::SlotNodeCreated(QtNodes::Node &n)
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
