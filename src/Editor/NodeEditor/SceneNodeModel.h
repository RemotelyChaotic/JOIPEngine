#ifndef SCENENODEMODEL_H
#define SCENENODEMODEL_H

#include <nodes/NodeDataModel>

class CSceneTranstitionData;
class CSceneNodeModelWidget;

using QtNodes::PortType;
using QtNodes::PortIndex;
using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeDataModel;
using QtNodes::NodeValidationState;

class CSceneNodeModel : public NodeDataModel
{
  Q_OBJECT

public:
  CSceneNodeModel();
  ~CSceneNodeModel() override {}

public:
  QString caption() const override;
  bool captionVisible() const override { return true; }
  QString name() const override;

  QJsonObject save() const override;
  void restore(QJsonObject const& p) override;

  unsigned int nPorts(PortType portType) const override;
  NodeDataType dataType(PortType portType,
                        PortIndex portIndex) const override;
  std::shared_ptr<NodeData> outData(PortIndex port) override;
  void setInData(std::shared_ptr<NodeData> data, PortIndex portIndex) override;
  QWidget* embeddedWidget() override;
  NodeValidationState validationState() const override;
  QString validationMessage() const override;

  ConnectionPolicy portOutConnectionPolicy(PortIndex) const override { return ConnectionPolicy::Many; }

protected:
  std::weak_ptr<CSceneTranstitionData>                m_wpInData;
  std::shared_ptr<CSceneTranstitionData>              m_spOutData;
  CSceneNodeModelWidget*                              m_pWidget;

  NodeValidationState m_modelValidationState;
  QString m_modelValidationError;

protected slots:
  void SlotNumberOfOutputsChanged(qint32 iValue);
};

#endif // SCENENODEMODEL_H
