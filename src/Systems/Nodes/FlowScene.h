#ifndef CFLOWSCENE_H
#define CFLOWSCENE_H

#include <nodes/FlowScene>
#include <nodes/Node>

#include <type_traits>

class CNodeGraphicsObjectProvider;

class CFlowScene : public QtNodes::FlowScene
{
public:
  CFlowScene(std::shared_ptr<QtNodes::DataModelRegistry> spRegistry,
             std::shared_ptr<CNodeGraphicsObjectProvider> spGraphicsObjectProvider,
             QObject* pParent = Q_NULLPTR);
  ~CFlowScene() override;

  // shadow
  QtNodes::Node& createNode(std::unique_ptr<QtNodes::NodeDataModel>&& dataModel);
  void loadFromMemory(const QByteArray& data);
  void loadFromObject(const QJsonObject& data);
  QtNodes::Node& restoreNode(QJsonObject const& nodeJson);

protected:
  virtual void NodeCreatedImpl(QtNodes::Node&){}

  std::shared_ptr<CNodeGraphicsObjectProvider> m_spGraphicsObjectProvider;
  bool                     m_bLoading;
};

#endif // CFLOWSCENE_H
