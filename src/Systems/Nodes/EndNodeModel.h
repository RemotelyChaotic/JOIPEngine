#ifndef ENDNODEMODEL_H
#define ENDNODEMODEL_H

#include "NodeModelBase.h"

class CSceneTranstitionData;

using QtNodes::PortType;
using QtNodes::PortIndex;
using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeDataModel;
using QtNodes::NodeValidationState;


class CEndNodeModel : public CNodeModelBase
{
  Q_OBJECT
public:
  CEndNodeModel();
   ~CEndNodeModel() override {}

  static QString Name() { return staticCaption(); }
  static QString staticCaption() { return QStringLiteral("Exit Point"); }

  QString caption() const override;
  bool captionVisible() const override;
  QString name() const override;

  QJsonObject save() const override;
  void restore(QJsonObject const& p) override;

  unsigned int nPorts(PortType portType) const override;
  NodeDataType dataType(PortType portType, PortIndex portIndex) const override;
  std::shared_ptr<NodeData> outData(PortIndex port) override;
  void setInData(std::shared_ptr<NodeData>, int) override;
  QWidget* embeddedWidget() override { return nullptr; }
  NodeValidationState validationState() const override;
  QString validationMessage() const override;

  ConnectionPolicy portOutConnectionPolicy(PortIndex) const override { return ConnectionPolicy::One; }

protected:
  std::weak_ptr<CSceneTranstitionData>                m_wpInData;

  NodeValidationState m_modelValidationState;
  QString m_modelValidationError;
};

#endif // ENDNODEMODEL_H
