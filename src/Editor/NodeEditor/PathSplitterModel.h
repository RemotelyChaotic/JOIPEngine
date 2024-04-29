#ifndef PATHSPLITTERMODEL_H
#define PATHSPLITTERMODEL_H

#include "EditorNodeModelBase.h"
#include "Systems/Project.h"
#include "Systems/Scene.h"
#include <QPointer>

#include <optional>

class CDatabaseManager;
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

  std::optional<QString> CustomTransition() const;
  virtual void SetProjectId(qint32 iId);
  qint32 ProjectId();
  ESceneTransitionType TransitionType() { return m_transitonType; }
  QString TransitionLabel(PortIndex port);
  void SetTransitionLabel(PortIndex index, const QString& sLabelValue);

  static QString Name() { return staticCaption(); }
  static QString staticCaption() { return QStringLiteral("Path Splitter"); }

  QString caption() const override;
  bool captionVisible() const override;
  QString name() const override;

  QJsonObject save() const override;
  void restore(QJsonObject const& p) override;
  void UndoRestore(QJsonObject const& p) override;

  unsigned int nPorts(PortType portType) const override;
  NodeDataType dataType(PortType portType, PortIndex portIndex) const override;
  std::shared_ptr<NodeData> outData(PortIndex port) override;
  void setInData(std::shared_ptr<NodeData>, int) override;
  QWidget* embeddedWidget() override;
  NodeValidationState validationState() const override;
  QString validationMessage() const override;

  ConnectionPolicy portOutConnectionPolicy(PortIndex) const override { return ConnectionPolicy::One; }

signals:
  void SignalAddLayoutFileRequested(const QString& sCustomInitContent);

protected slots:
  void SlotResourceAdded(qint32 iProjId, const QString& sName);
  void SlotResourceRenamed(qint32 iProjId, const QString& sOldName, const QString& sName);
  void SlotResourceRemoved(qint32 iProjId, const QString& sName);
  void SlotCustomTransitionChanged(bool bEnabled, const QString& sResource);
  void SlotTransitionTypeChanged(qint32 iType);
  void SlotTransitionLabelChanged(PortIndex index, const QString& sLabelValue);

protected:
  virtual void OnProjectSetImpl() {}
  virtual void SlotCustomTransitionChangedImpl(bool, const QString&) {}
  virtual void SlotResourceAddedImpl(const QString&, EResourceType) {}
  virtual void SlotResourceRenamedImpl(const QString& sOldName, const QString& sName, EResourceType) {}
  virtual void SlotResourceRemovedImpl(const QString&, EResourceType) {}

  std::weak_ptr<CSceneTranstitionData>                m_wpInData;
  std::shared_ptr<CSceneTranstitionData>              m_spOutData;

  NodeValidationState m_modelValidationState;
  QString m_modelValidationError;

  tspProject                       m_spProject;
  std::weak_ptr<CDatabaseManager>  m_wpDbManager;
  std::vector<QString>             m_vsLabelNames;
  ESceneTransitionType             m_transitonType;
  bool                             m_bCustomLayoutEnabled = false;
  QString                          m_sCustomLayout;
  bool                             m_bIsInUndoOperation = false;
};

//----------------------------------------------------------------------------------------
//
class CPathSplitterModelWithWidget : public CPathSplitterModel
{
  Q_OBJECT

public:
  CPathSplitterModelWithWidget();
  ~CPathSplitterModelWithWidget() override;

  void SetProjectId(qint32 iId) override;

  void restore(QJsonObject const& p) override;

  QWidget* embeddedWidget() override;

protected:
  void OnProjectSetImpl() override;
  void OnUndoStackSet() override;
  void SlotCustomTransitionChangedImpl(bool bEnabled, const QString& sResource) override;
  void SlotResourceAddedImpl(const QString& sName, EResourceType type) override;
  void SlotResourceRenamedImpl(const QString& sOldName, const QString& sName, EResourceType type) override;
  void SlotResourceRemovedImpl(const QString& sName, EResourceType type) override;

  QPointer<CPathSplitterModelWidget>                  m_pWidget;
};

#endif // PATHSPLITTERMODEL_H
