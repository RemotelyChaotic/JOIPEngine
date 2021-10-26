#ifndef PATHSPLITTERMODEL_H
#define PATHSPLITTERMODEL_H

#include "EditorNodeModelBase.h"
#include "Systems/Scene.h"
#include <QPointer>

class CPathSplitterModelWidget;
class CSceneTranstitionData;

using QtNodes::PortType;
using QtNodes::PortIndex;
using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeValidationState;

class CPathSplitterModel : public CEditorNodeModelBase
{
  Q_OBJECT

public:
  CPathSplitterModel();
   ~CPathSplitterModel() override {}

  ESceneTransitionType TransitionType() { return m_transitonType; }
  QString TransitionLabel(PortIndex port);

  static QString Name() { return staticCaption(); }
  static QString staticCaption() { return QStringLiteral("Path Splitter"); }

  QString caption() const override;
  bool captionVisible() const override;
  QString name() const override;

  QJsonObject save() const override;
  void restore(QJsonObject const& p) override;

  unsigned int nPorts(PortType portType) const override;
  NodeDataType dataType(PortType portType, PortIndex portIndex) const override;
  std::shared_ptr<NodeData> outData(PortIndex port) override;
  void setInData(std::shared_ptr<NodeData>, int) override;
  QWidget* embeddedWidget() override;
  NodeValidationState validationState() const override;
  QString validationMessage() const override;

  ConnectionPolicy portOutConnectionPolicy(PortIndex) const override { return ConnectionPolicy::One; }

protected:
  void OnUndoStackSet() override;

  std::weak_ptr<CSceneTranstitionData>                m_wpInData;
  std::shared_ptr<CSceneTranstitionData>              m_spOutData;
  QPointer<CPathSplitterModelWidget>                  m_pWidget;

  NodeValidationState m_modelValidationState;
  QString m_modelValidationError;

  std::vector<QString>          m_vsLabelNames;
  ESceneTransitionType          m_transitonType;

private slots:
  void SlotTransitionTypeChanged(qint32 iType);
  void SlotTransitionLabelChanged(PortIndex index, const QString& sLabelValue);
};

#endif // PATHSPLITTERMODEL_H
