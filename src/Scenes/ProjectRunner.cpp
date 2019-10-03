#include "ProjectRunner.h"
#include "Application.h"
#include "Backend/DatabaseManager.h"
#include "Backend/Project.h"
#include "Backend/Resource.h"
#include "Backend/Scene.h"
#include "Editor/NodeEditor/EndNodeModel.h"
#include "Editor/NodeEditor/FlowView.h"
#include "Editor/NodeEditor/PathMergerModel.h"
#include "Editor/NodeEditor/PathSplitterModel.h"
#include "Editor/NodeEditor/SceneNodeModel.h"
#include "Editor/NodeEditor/SceneTranstitionData.h"
#include "Editor/NodeEditor/StartNodeModel.h"

#include <nodes/DataModelRegistry>
#include <nodes/FlowScene>
#include <nodes/Node>
#include <nodes/NodeData>

#include <QDebug>
#include <QFile>

using QtNodes::DataModelRegistry;
using QtNodes::FlowScene;
using QtNodes::Node;

namespace
{
  std::shared_ptr<DataModelRegistry> RegisterDataModels()
  {
    auto ret = std::make_shared<DataModelRegistry>();
    ret->registerModel<CStartNodeModel>("Control");
    ret->registerModel<CSceneNodeModel>("Scene");
    ret->registerModel<CEndNodeModel>("Control");
    ret->registerModel<CPathMergerModel>("Path");
    ret->registerModel<CPathSplitterModel>("Path");
    return ret;
  }
}

//----------------------------------------------------------------------------------------
//
CProjectRunner::CProjectRunner(QObject* pParent) :
  QObject (pParent),
  m_spCurrentProject(nullptr),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>()),
  m_nodeMap(),
  m_pFlowScene(nullptr),
  m_pCurrentNode(nullptr)
{
  m_pFlowScene = new FlowScene(RegisterDataModels());

  connect(m_pFlowScene, &FlowScene::nodeCreated,
          this, &CProjectRunner::SlotNodeCreated);

  std::random_device rd;            //Will be used to obtain a seed for the random number engine
  m_generator = std::mt19937(rd()); //Standard mersenne_twister_engine seeded with rd()
}

CProjectRunner::~CProjectRunner()
{
  UnloadProject();
  //delete m_pFlowScene;
}

//----------------------------------------------------------------------------------------
//
void CProjectRunner::LoadProject(tspProject spProject)
{
  if (nullptr != m_spCurrentProject)
  {
    qWarning() << "Old Project was not unloaded before loading project.";
  }

  m_spCurrentProject = spProject;

  bool bOk = LoadFlowScene();
  if (!bOk) { return; }

  bOk = ResolveStart();
  if (!bOk) { return; }

  bOk = ResolveNextScene();
  if (!bOk) { return; }
}

//----------------------------------------------------------------------------------------
//
void CProjectRunner::UnloadProject()
{
  m_pCurrentNode = nullptr;
  m_spCurrentProject = nullptr;
  m_nodeMap.clear();
  m_pFlowScene->clearScene();
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
        return spScene;
      }
    }
    else
    {
      return nullptr;
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

      auto spResource = spDbManager->FindResource(m_spCurrentProject, sModelName);
      if (nullptr != spResource)
      {
        QReadLocker resourceLocker(&spResource->m_rwLock);
        QUrl path = spResource->m_sPath;
        resourceLocker.unlock();

        QString sPath = ResourceUrlToAbsolutePath(path, PhysicalProjectName(m_spCurrentProject));
        QFile modelFile(sPath);
        if (modelFile.open(QIODevice::ReadOnly))
        {
          QByteArray arr = modelFile.readAll();
          m_pFlowScene->loadFromMemory(arr);
        }
        else
        {
          QString sError(tr("Could not open save scene model file."));
          qWarning() << sError;
          emit SignalError(sError, QtMsgType::QtWarningMsg);
          return false;
        }
      }
    }
    else
    {
      QString sError(tr("Could not open save scene model file."));
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

    std::uniform_int_distribution<> dis(0, static_cast<qint32>(viValidIndicees.size() - 1));

    qint32 iGeneratedIndex = dis(m_generator);
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
            m_nodeMap.insert({pNode.first, pNode.second});
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
bool CProjectRunner::ResolveStart()
{
  if (nullptr == m_pFlowScene)
  {
    QString sError(tr("Internal error."));
    qWarning() << sError;
    emit SignalError(sError, QtMsgType::QtCriticalMsg);
    return false;
  }

  bool bFound = false;
  auto vpNodes = m_pFlowScene->allNodes();
  for (auto pNode : vpNodes)
  {
    CStartNodeModel* pStartModel = dynamic_cast<CStartNodeModel*>(pNode->nodeDataModel());
    if (nullptr != pStartModel)
    {
      if (!bFound)
      {
        bFound = true;
        m_pCurrentNode = pNode;
      }
      else
      {
        QString sError(tr("Multiple entry points in project."));
        qWarning() << sError;
        emit SignalError(sError, QtMsgType::QtCriticalMsg);
        return false;
      }
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
