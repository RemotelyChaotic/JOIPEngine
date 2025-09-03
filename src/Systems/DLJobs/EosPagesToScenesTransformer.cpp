#include "EosPagesToScenesTransformer.h"
#include "Application.h"

#include "Systems/Nodes/EndNodeModel.h"
#include "Systems/Nodes/FlowScene.h"
#include "Systems/Nodes/PathMergerModel.h"
#include "Systems/Nodes/PathSplitterModel.h"
#include "Systems/Nodes/NodeEditorRegistryBase.h"
#include "Systems/Nodes/NodegraphicsObjectProvider.h"
#include "Systems/Nodes/SceneNodeModel.h"
#include "Systems/Nodes/SceneTranstitionData.h"
#include "Systems/Nodes/StartNodeModel.h"
#include "Systems/DatabaseManager.h"
#include "Systems/Project.h"

#include <nodes/Node>
#include <QGraphicsObject>
#include <QJsonArray>

namespace
{
  const QString c_sPagesKeyWord = "pages";
  const QString c_sInitKeyWord = "init";

  const QString c_sStartPageKeyWord = "start";

  const QString c_sCommandsKeyWord = "commands";
  const QString c_sEvalKeyWord = "eval";
  const QString c_sGotoKeyWord = "goto";
  const QString c_sScriptKeyWord = "script";
  const QString c_sTargetKeyWord = "target";

  const double c_dNodeOffset = 500.0;
}

//----------------------------------------------------------------------------------------
//
CEosPagesToScenesTransformer::CEosPagesToScenesTransformer(const QJsonDocument& script) :
  m_vPages(),
  m_spScene(std::make_unique<CFlowScene>(
          CNodeEditorRegistryBase::RegisterDataModelsWithoutUi(),
          std::make_shared<CDefaultGraphicsObjectProvider>(), nullptr)),
  m_script(script),
  m_startId(),
  m_endId()
{
  m_startId = CreateNodeAndAdd(CStartNodeModel::Name(), QPointF(0, 0));
  m_lastNode = m_startId;
  m_endId = CreateNodeAndAdd(CEndNodeModel::Name(), QPointF(c_dNodeOffset, 0));
}

CEosPagesToScenesTransformer::~CEosPagesToScenesTransformer()
{

}

//----------------------------------------------------------------------------------------
//
tspScene CEosPagesToScenesTransformer::AddPageToScene(const qint32 iProjectId,
                                                      tspProject spProject,
                                                      const SPageScene& page)
{
  tspScene spScene = nullptr;

  // get previous end position
  QPointF point(0.0, 0.0);
  QtNodes::Node* pPreviousNode = GetNode(m_lastNode);
  QtNodes::Node* pEndNode = GetNode(m_endId);
  if (nullptr != pEndNode)
  {
    point = pEndNode->nodeGraphicsObject().pos();
  }

  // create nodes
  QUuid idSplitter = CreateNodeAndAdd(CPathSplitterModel::Name(), point);
  point += QPointF(0.0, c_dNodeOffset);
  m_lastNode = CreateNodeAndAdd(CSceneNodeModel::Name(), point);
  point += QPointF(c_dNodeOffset, 0.0);

  QtNodes::Node* pSplitterNode = GetNode(idSplitter);
  QtNodes::Node* pSceneNode = GetNode(m_lastNode);
  if (nullptr != pPreviousNode && nullptr != pSplitterNode &&
      nullptr != pSceneNode && nullptr != pEndNode)
  {
    // connect nodes
    m_spScene->createConnection(*pSplitterNode, 0, *pPreviousNode, 0);
    m_spScene->createConnection(*pSceneNode, 0, *pSplitterNode, 0);

    // set properties
    CPathSplitterModel* pSplitter =
        dynamic_cast<CPathSplitterModel*>(pSplitterNode->nodeDataModel());
    if (nullptr != pSplitter)
    {
      pSplitter->SetTransitionLabel(0, page.m_sName);
    }

    auto spDbManager = CApplication::Instance()->System<CDatabaseManager>().lock();
    if (nullptr != spDbManager)
    {
      CSceneNodeModel* pSceneModel =
          dynamic_cast<CSceneNodeModel*>(pSceneNode->nodeDataModel());
      if (nullptr != pSceneModel)
      {
        pSceneModel->SetProjectId(iProjectId);
        qint32 iSceneId = pSceneModel->SceneId();
        spScene = spDbManager->FindScene(spProject, iSceneId);
        pSceneModel->SetSceneName(page.m_sName);
      }
      CPathSplitterModel* pSplitterModel =
          dynamic_cast<CPathSplitterModel*>(pSceneNode->nodeDataModel());
      if (nullptr != pSplitterModel)
      {
        pSceneModel->SetProjectId(iProjectId);
      }
    }
  }

  // set new end position
  if (nullptr != pEndNode)
  {
    pEndNode->nodeGraphicsObject().setPos(point);
  }

  return spScene;
}

//----------------------------------------------------------------------------------------
//
bool CEosPagesToScenesTransformer::CollectScenes(QString* psError)
{
  QJsonObject root = m_script.object();

  auto initIt = root.find(c_sInitKeyWord); // optional
  auto pagesIt = root.find(c_sPagesKeyWord);

  if (root.end() == pagesIt)
  {
    if (nullptr != psError) { *psError = "JSON does not caontain some required modes."; }
    return false;
  }

  if (!pagesIt.value().isObject())
  {
    if (nullptr != psError) { *psError = QString("\"%1\" node is not an object.").arg(c_sPagesKeyWord); }
    return false;
  }

  // gather pages
  QJsonObject pagesObj = pagesIt.value().toObject();
  for (auto it = pagesObj.constBegin(); pagesObj.constEnd() != it; ++it)
  {
    QJsonValue pageObjVal = it.value();
    const QString sPageKey = it.key();
    if (pageObjVal.isArray())
    {
      QJsonArray arrPage = pageObjVal.toArray();
      QJsonObject objPage
      {
          {c_sCommandsKeyWord, arrPage}
      };
      if (c_sStartPageKeyWord == sPageKey)
      {
        m_vPages.insert(m_vPages.begin(), {sPageKey, QJsonDocument(objPage)});
      }
      else
      {
        m_vPages.push_back({sPageKey, QJsonDocument(objPage)});
      }
    }
  }

  // create init "page"
  if (root.end() != initIt && initIt.value().isString())
  {
    QString sScript = initIt.value().toString();
    QJsonObject evalObj
    {
      {
        c_sCommandsKeyWord,
        QJsonArray{ { QJsonObject{{c_sEvalKeyWord, QJsonObject { {c_sScriptKeyWord, sScript} }}},
                      QJsonObject{{c_sGotoKeyWord, QJsonObject { {c_sTargetKeyWord, c_sStartPageKeyWord} }}}} }
      }
    };
    m_vPages.insert(m_vPages.begin(), {c_sInitKeyWord, QJsonDocument(evalObj)});
  }

  return true;
}

//----------------------------------------------------------------------------------------
//
QByteArray CEosPagesToScenesTransformer::CompileScenes()
{
  // finish connections
  QtNodes::Node* pSceneNode = GetNode(m_lastNode);
  QtNodes::Node* pEndNode = GetNode(m_endId);
  if (nullptr != pEndNode && nullptr != pSceneNode)
  {
    m_spScene->createConnection(*pEndNode, 0, *pSceneNode, 0);
  }
  return m_spScene->saveToMemory();
}

//----------------------------------------------------------------------------------------
//
QUuid CEosPagesToScenesTransformer::CreateNodeAndAdd(const QString sName, QPointF pos)
{
  auto type = m_spScene->registry().create(sName);

  if (type)
  {
    auto& node = m_spScene->createNode(std::move(type));

    node.nodeGraphicsObject().setPos(pos);

    m_spScene->nodePlaced(node);

    return node.id();
  }

  return QUuid();
}

//----------------------------------------------------------------------------------------
//
QtNodes::Node* CEosPagesToScenesTransformer::GetNode(QUuid uuid)
{
  const auto& map = m_spScene->nodes();

  const auto& it = map.find(uuid);

  if (map.end() != it)
  {
    return it->second.get();
  }

  return nullptr;
}
