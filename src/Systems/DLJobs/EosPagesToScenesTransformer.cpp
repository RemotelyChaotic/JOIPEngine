#include "EosPagesToScenesTransformer.h"
#include "Application.h"

#include "Editor/NodeEditor/EndNodeModel.h"
#include "Editor/NodeEditor/FlowScene.h"
#include "Editor/NodeEditor/PathMergerModel.h"
#include "Editor/NodeEditor/PathSplitterModel.h"
#include "Editor/NodeEditor/NodeEditorRegistry.h"
#include "Editor/NodeEditor/SceneNodeModel.h"
#include "Editor/NodeEditor/SceneTranstitionData.h"
#include "Editor/NodeEditor/StartNodeModel.h"
#include "Systems/DatabaseManager.h"
#include "Systems/Project.h"

#include <nodes/Node>
#include <QGraphicsObject>
#include <QJsonArray>

namespace
{
  const QString c_sPagesKeyWord = "pages";
  const QString c_sInitKeyWord = "init";

  const QString c_sEvalKeyWord = "eval";
  const QString c_sScriptKeyWord = "script";
  const QString c_sCommandsKeyWord = "commands";

  const double c_dNodeOffset = 200.0;
}

//----------------------------------------------------------------------------------------
//
CEosPagesToScenesTransformer::CEosPagesToScenesTransformer(const QJsonDocument& script) :
  m_vPages(),
  m_spScene(std::make_unique<CFlowScene>(CNodeEditorRegistry::RegisterDataModelsWithoutUi(), nullptr)),
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
    CSceneNodeModel* pSceneModel =
        dynamic_cast<CSceneNodeModel*>(pSceneNode->nodeDataModel());
    auto spDbManager = CApplication::Instance()->System<CDatabaseManager>().lock();
    if (nullptr != pSceneModel && nullptr != spDbManager)
    {
      pSceneModel->SetProjectId(iProjectId);
      qint32 iSceneId = pSceneModel->SceneId();
      spScene = spDbManager->FindScene(spProject, iSceneId);
      pSceneModel->SetSceneName(page.m_sName);
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

  // create init "page"
  if (root.end() != initIt && initIt.value().isString())
  {
    QString sScript = initIt.value().toString();
    QJsonObject evalObj
    {
      {
        c_sCommandsKeyWord,
        QJsonArray{ {c_sEvalKeyWord, QJsonObject { {c_sScriptKeyWord, sScript} }} }
      }
    };
    m_vPages.push_back({c_sInitKeyWord, QJsonDocument(evalObj)});
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
      m_vPages.push_back({sPageKey, QJsonDocument(objPage)});
    }
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
