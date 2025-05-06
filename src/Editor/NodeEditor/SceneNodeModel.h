#ifndef SCENENODEMODEL_H
#define SCENENODEMODEL_H

#include "EditorNodeModelBase.h"
#include "Systems/Project.h"
#include "Systems/Scene.h"
#include <memory>

class CDatabaseManager;
class CSceneTranstitionData;
class CSceneNodeModelWidget;

class QAbstractItemModel;

using QtNodes::PortType;
using QtNodes::PortIndex;
using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeValidationState;

class CSceneNodeModel : public CEditorNodeModelBase
{
  Q_OBJECT

public:
  CSceneNodeModel();
  ~CSceneNodeModel() override {}

public:
  virtual void SetProjectId(qint32 iId);
  qint32 ProjectId();
  qint32 SceneId();

  virtual void SetSceneName(const QString& sScene);
  QString SceneName() { return m_sSceneName; }

  void SetResourceItemModel(QAbstractItemModel* pModel);

  static QString Name() { return staticCaption(); }
  static QString staticCaption() { return QStringLiteral("Scene"); }

  QString caption() const override;
  bool captionVisible() const override { return true; }
  QString name() const override;

  QJsonObject save() const override;
  void restore(QJsonObject const& p) override;
  void UndoRestore(QJsonObject const& p) override;

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
  void SignalAddScriptFileRequested(const QString&);
  void SignalAddLayoutFileRequested(const QString&);

protected slots:
  void SlotNameChanged(const QString& sName);
  void SlotLayoutChanged(const QString& sName);
  void SlotScriptChanged(const QString& sName);
  void SlotSceneDataChanged(qint32 iProjId, qint32 iSceneId);
  void SlotSceneRenamed(qint32 iProjId, qint32 iSceneId);
  void SlotResourceAdded(qint32 iProjId, const QString& sName);
  void SlotResourceRenamed(qint32 iProjId, const QString& sOldName, const QString& sName);
  void SlotResourceRemoved(qint32 iProjId, const QString& sName);
  void SlotTitleResourceChanged(const QString& sOld, const QString& sNew);

protected:
  virtual void ProjectSetImpl() {}
  virtual void ResourceItemModelSetImpl(QAbstractItemModel* pModel){}
  virtual void SlotNameChangedImpl(const QString&) {}
  virtual void SlotLayoutChangedImpl(const QString&) {}
  virtual void SlotScriptChangedImpl(const QString&) {}
  virtual void SlotSceneRenamedImpl(const QString&) {}
  virtual void SlotResourceAddedImpl(const QString&, EResourceType) {}
  virtual void SlotResourceRenamedImpl(const QString&, const QString&, EResourceType) {}
  virtual void SlotResourceRemovedImpl(const QString&, EResourceType) {}
  virtual void SlotTitleResourceChangedImpl(const QString& sOld, const QString& sNew) {}

  std::weak_ptr<CDatabaseManager>                     m_wpDbManager;
  std::weak_ptr<CSceneTranstitionData>                m_wpInData;
  std::shared_ptr<CSceneTranstitionData>              m_spOutData;
  tspProject                                          m_spProject;
  tspScene                                            m_spScene;
  bool                                                m_bOutConnected;

  NodeValidationState m_modelValidationState;
  QString             m_modelValidationError;
  QString             m_sSceneName;
  QString             m_sOldSceneName;
  QString             m_sScript;
  QString             m_sLayout;
  QString             m_sTitle;
  bool                m_bIsInUndoOperation = false;
};

//----------------------------------------------------------------------------------------
//
class CSceneNodeModelWithWidget : public CSceneNodeModel
{
  Q_OBJECT

public:
  CSceneNodeModelWithWidget();
  ~CSceneNodeModelWithWidget() override;

  void SetProjectId(qint32 iId) override;
  void SetSceneName(const QString& sScene) override;

  void restore(QJsonObject const& p) override;

  QWidget* embeddedWidget() override;

protected:
  void OnUndoStackSet() override;
  void ProjectSetImpl() override;
  void ResourceItemModelSetImpl(QAbstractItemModel* pModel) override;
  void SlotNameChangedImpl(const QString& sName) override;
  void SlotLayoutChangedImpl(const QString& sName) override;
  void SlotScriptChangedImpl(const QString& sName) override;
  void SlotSceneRenamedImpl(const QString& sName) override;
  void SlotResourceAddedImpl(const QString& sName, EResourceType type) override;
  void SlotResourceRenamedImpl(const QString& sOldName, const QString& sName, EResourceType type) override;
  void SlotResourceRemovedImpl(const QString& sName, EResourceType type) override;
  void SlotTitleResourceChangedImpl(const QString& sOld, const QString& sNew) override;

  QPointer<CSceneNodeModelWidget>                   m_pWidget;
};

#endif // SCENENODEMODEL_H
