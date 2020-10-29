#ifndef SCENENODEMODEL_H
#define SCENENODEMODEL_H

#include "Systems/Project.h"
#include "Systems/Scene.h"

#include <nodes/NodeDataModel>
#include <memory>

class CDatabaseManager;
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
  void SetProjectId(qint32 iId);
  qint32 ProjectId();
  qint32 SceneId();
  QString SceneName() { return m_sSceneName; }

  static QString staticCaption() { return QStringLiteral("Scene"); }

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

  void outputConnectionCreated(QtNodes::Connection const&) override;
  void outputConnectionDeleted(QtNodes::Connection const&) override;

  ConnectionPolicy portOutConnectionPolicy(PortIndex) const override { return ConnectionPolicy::One; }

signals:
  void SignalAddScriptFileRequested();

protected:
  std::weak_ptr<CDatabaseManager>                     m_wpDbManager;
  std::weak_ptr<CSceneTranstitionData>                m_wpInData;
  std::shared_ptr<CSceneTranstitionData>              m_spOutData;
  tspProject                                          m_spProject;
  tspScene                                            m_spScene;
  CSceneNodeModelWidget*                              m_pWidget;

  NodeValidationState m_modelValidationState;
  QString             m_modelValidationError;
  QString             m_sSceneName;
  QString             m_sOldSceneName;

protected slots:
  void SlotNameChanged(const QString& sName);
  void SlotSceneRenamed(qint32 iProjId, qint32 iSceneId);
  void SlotResourceAdded(qint32 iProjId, const QString& sName);
  void SlotResourceRemoved(qint32 iProjId, const QString& sName);
};

#endif // SCENENODEMODEL_H
