#ifndef PATHMERGERMODEL_H
#define PATHMERGERMODEL_H

#include "EditorNodeModelBase.h"
#include <nodes/NodeDataModel>
#include <map>

class CSceneTranstitionData;

using QtNodes::PortType;
using QtNodes::PortIndex;
using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeValidationState;

class CPathMergerModel : public CEditorNodeModelBase
{
  Q_OBJECT
public:
  CPathMergerModel();
   ~CPathMergerModel() override {}

  static QString staticCaption() { return QStringLiteral("Path Merger"); }

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
  std::weak_ptr<CSceneTranstitionData>                    m_wpInData1;
  std::weak_ptr<CSceneTranstitionData>                    m_wpInData2;
  std::weak_ptr<CSceneTranstitionData>                    m_wpInData3;
  std::weak_ptr<CSceneTranstitionData>                    m_wpInData4;
  std::shared_ptr<CSceneTranstitionData>                  m_spOutData;

  NodeValidationState m_modelValidationState;
  QString m_modelValidationError;
};

#endif // PATHMERGERMODEL_H
