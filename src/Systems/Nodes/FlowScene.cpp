#include "FlowScene.h"
#include "NodeGraphicsObjectProvider.h"

#include <QJsonArray>
#include <QJsonObject>

#include <nodes/Node>
#include <QKeyEvent>

using QtNodes::FlowScene;

//----------------------------------------------------------------------------------------
//
CFlowScene::CFlowScene(std::shared_ptr<QtNodes::DataModelRegistry> spRegistry,
                       std::shared_ptr<CNodeGraphicsObjectProvider> spGraphicsObjectProvider,
                       QObject* pParent) :
  QtNodes::FlowScene(spRegistry, pParent),
  m_spGraphicsObjectProvider(spGraphicsObjectProvider),
  m_bLoading(false)
{
}

CFlowScene::~CFlowScene() {}


//----------------------------------------------------------------------------------------
//
QtNodes::Node& CFlowScene::createNode(std::unique_ptr<QtNodes::NodeDataModel>&& dataModel)
{
  // see FlowScene.cpp -> createNode
  auto node = std::make_unique<QtNodes::Node>(std::move(dataModel));
  auto ngo  = m_spGraphicsObjectProvider->createObject(*this, *node);

  node->setGraphicsObject(std::move(ngo));

  auto nodePtr = node.get();
  _nodes[node->id()] = std::move(node);

  connect(nodePtr, &QtNodes::Node::connectionRemoved, this, &FlowScene::deleteConnection);

  Q_EMIT nodeCreated(*nodePtr);
  return *nodePtr;
}

//----------------------------------------------------------------------------------------
//
void CFlowScene::loadFromMemory(const QByteArray& data)
{
  m_bLoading = true;
  const QJsonObject jsonDocument = QJsonDocument::fromJson(data).object();
  loadFromObject(jsonDocument);
  m_bLoading = false;

  for (const auto& [uid, spNode] : nodes())
  {
    Q_UNUSED(uid)
    NodeCreatedImpl(*spNode);
  }
}

//----------------------------------------------------------------------------------------
//
void CFlowScene::loadFromObject(const QJsonObject& data)
{
  // see FlowScene.cpp: loadFromObject
  QJsonArray nodesJsonArray = data["nodes"].toArray();

  for (QJsonValueRef node : nodesJsonArray)
  {
    restoreNode(node.toObject());
  }

  QJsonArray connectionJsonArray = data["connections"].toArray();

  for (QJsonValueRef connection : connectionJsonArray)
  {
    restoreConnection(connection.toObject());
  }
}

//----------------------------------------------------------------------------------------
//
QtNodes::Node& CFlowScene::restoreNode(QJsonObject const& nodeJson)
{
  QString modelName = nodeJson["model"].toObject()["name"].toString();

  auto dataModel = registry().create(modelName);

  if (!dataModel)
    throw std::logic_error(std::string("No registered model with name ") +
                           modelName.toLocal8Bit().data());

  // restore data model before the node, to be able to restore dynamic ports.
  dataModel->restore(nodeJson["model"].toObject());

  // create a node with uuid taken from json
  auto node = std::make_unique<QtNodes::Node>(std::move(dataModel),
                                              QUuid(nodeJson["id"].toString()));

  // create node graphics object
  auto ngo  = m_spGraphicsObjectProvider->createObject(*this, *node);
  QJsonObject positionJson = nodeJson["position"].toObject();
  QPointF point(positionJson["x"].toDouble(), positionJson["y"].toDouble());
  ngo->setPos(point);

  node->setGraphicsObject(std::move(ngo));

  auto nodePtr = node.get();
  _nodes[node->id()] = std::move(node);

  connect(nodePtr, &QtNodes::Node::connectionRemoved, this, &FlowScene::deleteConnection);

  Q_EMIT nodePlaced(*nodePtr);
  Q_EMIT nodeCreated(*nodePtr);
  return *nodePtr;
}
