#ifndef STARTNODEMODEL_H
#define STARTNODEMODEL_H

#include <nodes/NodeDataModel>

class CSceneTranstitionData;

using QtNodes::PortType;
using QtNodes::PortIndex;
using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeDataModel;
using QtNodes::NodeValidationState;

class CStartNodeModel : public NodeDataModel
{
  Q_OBJECT
public:
  CStartNodeModel();
  ~CStartNodeModel() override {}

  QString caption() const override;
  bool captionVisible() const override;
  QString name() const override;

  QJsonObject save() const override;
  void restore(QJsonObject const& p) override;

  unsigned int nPorts(PortType portType) const override;
  NodeDataType dataType(PortType portType, PortIndex portIndex) const override;
  std::shared_ptr<NodeData> outData(PortIndex port) override;
  void setInData(std::shared_ptr<NodeData>, int) override { }
  QWidget* embeddedWidget() override { return nullptr; }

  ConnectionPolicy portOutConnectionPolicy(PortIndex) const override { return ConnectionPolicy::One; }

protected:
  std::shared_ptr<CSceneTranstitionData> m_spTransition;
};

#endif // STARTNODEMODEL_H
