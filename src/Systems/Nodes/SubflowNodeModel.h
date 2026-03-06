#ifndef CSUBFLOWNODEMODEL_H
#define CSUBFLOWNODEMODEL_H

#include "NodeModelBase.h"
#include "Systems/Database/Project.h"

class CDatabaseManager;
class CSceneTranstitionData;
class CSubflowNodeModelWidget;

class QAbstractItemModel;

using QtNodes::PortType;
using QtNodes::PortIndex;
using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeValidationState;

class CSubflowNodeModel : public CNodeModelBase
{
  Q_OBJECT

public:
  CSubflowNodeModel();
  ~CSubflowNodeModel() override {}

  virtual void SetProjectId(qint32 iId);
  qint32 ProjectId();

  QString FlowName() const { return m_sFlow; }

  virtual void SetNodeName(const QString& sName);
  QString NodeName() { return m_sNodeName; }

  static QString Name() { return staticCaption(); }
  static QString staticCaption() { return QStringLiteral("Subflow"); }

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
  void SignalAddNodeFileRequested();

protected slots:
  void SlotFlowChanged(const QString& sName);
  void SlotNameChanged(const QString& sName);
  void SlotResourceAdded(qint32 iProjId, const QString& sName);
  void SlotResourceRenamed(qint32 iProjId, const QString& sOldName, const QString& sName);
  void SlotResourceRemoved(qint32 iProjId, const QString& sName);

protected:
  virtual void ProjectSetImpl() {}
  virtual void SlotFlowChangedImpl(const QString&);
  virtual void SlotNameChangedImpl(const QString&);
  virtual void SlotResourceAddedImpl(const QString&, EResourceType) {}
  virtual void SlotResourceRenamedImpl(const QString&, const QString&, EResourceType) {}
  virtual void SlotResourceRemovedImpl(const QString&, EResourceType) {}

  std::weak_ptr<CDatabaseManager>                     m_wpDbManager;
  std::weak_ptr<CSceneTranstitionData>                m_wpInData;
  std::shared_ptr<CSceneTranstitionData>              m_spOutData;
  tspProject                                          m_spProject;
  bool                                                m_bOutConnected;

  NodeValidationState m_modelValidationState;
  QString             m_modelValidationError;

  QString             m_sNodeName;
  QString             m_sFlow;
};

//----------------------------------------------------------------------------------------
//
class CSubflowNodeModelWithWidget : public CSubflowNodeModel
{
  Q_OBJECT

public:
  CSubflowNodeModelWithWidget();
  ~CSubflowNodeModelWithWidget() override;

  void SetProjectId(qint32 iId) override;

  void restore(QJsonObject const& p) override;

  QWidget* embeddedWidget() override;

protected:
  void ProjectSetImpl() override;
  void SlotFlowChangedImpl(const QString& sName) override;
  void SlotNameChangedImpl(const QString&) override;
  void SlotResourceAddedImpl(const QString&, EResourceType) override;
  void SlotResourceRenamedImpl(const QString&, const QString&, EResourceType) override;
  void SlotResourceRemovedImpl(const QString&, EResourceType) override;

  QPointer<CSubflowNodeModelWidget>                   m_pWidget;
};

#endif // CSUBFLOWNODEMODEL_H
