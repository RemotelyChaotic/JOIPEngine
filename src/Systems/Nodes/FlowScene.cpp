#include "FlowScene.h"
#include "CommandConnectionAdded.h"
#include "CommandConnectionRemoved.h"
#include "CommandNodeAdded.h"
#include "CommandNodeMoved.h"
#include "EditorNodeModelBase.h"
#include "FlowView.h"
#include "NodeGraphicsObjectProvider.h"

#include "Editor/EditorWidgetTypes.h"

#include <QJsonArray>
#include <QJsonObject>

#include <nodes/Node>
#include <QKeyEvent>

using QtNodes::FlowScene;

const char* CFlowScene::c_sUndoRedoingOperation = "UndoRedoingOperation";

//----------------------------------------------------------------------------------------
//
CFlowScene::CFlowScene(std::shared_ptr<QtNodes::DataModelRegistry> spRegistry,
                       std::shared_ptr<CNodeGraphicsObjectProvider> spGraphicsObjectProvider,
                       QObject* pParent) :
  QtNodes::FlowScene(spRegistry, pParent),
  m_spGraphicsObjectProvider(spGraphicsObjectProvider),
  m_pUndoStack(nullptr),
  m_bUndoRedoOperationInProgress(false),
  m_bLoading(false)
{
  connect(this, &FlowScene::connectionCreated, this, &CFlowScene::SlotConnectionCreated, Qt::DirectConnection);
  connect(this, &FlowScene::connectionDeleted, this, &CFlowScene::SlotConnectionDeleted, Qt::DirectConnection);
  connect(this, &FlowScene::nodeCreated, this, &CFlowScene::SlotNodeCreated, Qt::DirectConnection);
  connect(this, &FlowScene::nodePlaced, this, &CFlowScene::SlotNodePlaced, Qt::DirectConnection);
  connect(this, &FlowScene::nodeMoved, this, &CFlowScene::SlotNodeMoved, Qt::DirectConnection);
}

CFlowScene::~CFlowScene() {}

//----------------------------------------------------------------------------------------
//
void CFlowScene::SetUndoStack(QPointer<QUndoStack> pUndoStack)
{
  if (nullptr != m_pUndoStack) { delete m_pUndoStack; }
  m_pUndoStack = pUndoStack;
}

//----------------------------------------------------------------------------------------
//
QPointer<QUndoStack> CFlowScene::UndoStack()
{
  return m_pUndoStack;
}

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
    if (auto pTypeUndoAware = dynamic_cast<CEditorNodeModelBase*>(spNode->nodeDataModel()))
    {
      pTypeUndoAware->SetUndoStack(m_pUndoStack);
      pTypeUndoAware->SetNodeContext(spNode->id(), this);
    }
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

//----------------------------------------------------------------------------------------
//
void CFlowScene::SlotConnectionCreated(QtNodes::Connection const &c)
{
  if (nullptr != UndoStack() &&
      !m_bUndoRedoOperationInProgress &&
      !m_bLoading)
  {
    UndoStack()->push(new CCommandConnectionAdded(this, c.id(), c.save()));
  }
}

//----------------------------------------------------------------------------------------
//
void CFlowScene::SlotConnectionDeleted(QtNodes::Connection const &c)
{
  if (nullptr != UndoStack() &&
      !m_bUndoRedoOperationInProgress &&
      !m_bLoading)
  {
    UndoStack()->push(new CCommandConnectionRemoved(this, c.id(), c.save()));
  }
}

//----------------------------------------------------------------------------------------
//
void CFlowScene::SlotNodeCreated(QtNodes::Node& node)
{
  if (m_bLoading) { return; }
  if (auto pTypeUndoAware = dynamic_cast<CEditorNodeModelBase*>(node.nodeDataModel()))
  {
    pTypeUndoAware->SetUndoStack(m_pUndoStack);
    pTypeUndoAware->SetNodeContext(node.id(), this);
  }
}

//----------------------------------------------------------------------------------------
//
void CFlowScene::SlotNodePlaced(QtNodes::Node& node)
{
  node.nodeGraphicsObject().setProperty(editor::c_sPropertyOldValue, node.nodeGraphicsObject().pos());
}

//----------------------------------------------------------------------------------------
//
void CFlowScene::SlotNodeMoved(QtNodes::Node& node, const QPointF& newPosition)
{
  if (!m_bLoading && !m_bUndoRedoOperationInProgress)
  {
    QPointF oldPos =
        node.nodeGraphicsObject().property(editor::c_sPropertyOldValue).value<QPointF>();
    if (oldPos.x() != newPosition.x() && oldPos.y() != newPosition.y() &&
        nullptr != UndoStack() && views().size() > 0)
    {
      for (auto pView : views())
      {
        if (CFlowView* pViewCasted = dynamic_cast<CFlowView*>(pView))
        {
          UndoStack()->push(new CCommandNodeMoved(pViewCasted, node.id(),
                                                  oldPos, newPosition));
        }
      }
    }
  }
}
