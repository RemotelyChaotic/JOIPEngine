#ifndef STARTNODEMODEL_H
#define STARTNODEMODEL_H

#include "EditorNodeModelBase.h"

class CSceneTranstitionData;

using QtNodes::PortType;
using QtNodes::PortIndex;
using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeValidationState;

class CStartNodeModel : public CEditorNodeModelBase
{
  Q_OBJECT
public:
  CStartNodeModel();
  ~CStartNodeModel() override {}

  static QString Name() { return staticCaption(); }
  static QString staticCaption() { return QStringLiteral("Entry Point"); }

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
  NodeValidationState validationState() const override;
  QString validationMessage() const override;

  void outputConnectionCreated(QtNodes::Connection const&) override;
  void outputConnectionDeleted(QtNodes::Connection const&) override;

  ConnectionPolicy portOutConnectionPolicy(PortIndex) const override { return ConnectionPolicy::One; }

protected:
  std::shared_ptr<CSceneTranstitionData> m_spTransition;
  NodeValidationState                    m_modelValidationState;
  QString                                m_modelValidationError;
};

#endif // STARTNODEMODEL_H
