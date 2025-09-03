#ifndef FLOWVIEW_H
#define FLOWVIEW_H

#include <nodes/FlowView>
#include <QPointer>
#include <QUndoStack>
#include <map>

class CFlowScene;

class CFlowView : public QtNodes::FlowView
{
  Q_OBJECT

public:
  CFlowView(QWidget* pParent = nullptr);
  CFlowView(CFlowScene* pScene, QWidget* pParent = nullptr);

  CFlowView(const CFlowView&) = delete;
  CFlowView operator=(const CFlowView&) = delete;

  ~CFlowView() override;

  void SetScene(CFlowScene* pScene);
  virtual void SetSceneImpl(CFlowScene* pScene) {}
  CFlowScene* Scene();

  void FitAllNodesInView();
  virtual void OpenContextMenuAt(const QPoint& localPoint, const QPoint& createPoint = QPoint());

  bool IsReadOnly();
  void SetReadOnly(bool bReadOnly);

  void SetModelHiddenInContextMenu(const QString& sId, bool bHidden);
  bool IsModelHiddenInContextMenu(const QString& sId);

protected slots:
  virtual void SlotClearSelectionTriggered();
  virtual void SlotDeleteTriggered();

protected:
  void contextMenuEvent(QContextMenuEvent* pEvent) override;

  virtual void NodeAboutToBeCreated(const QString& modelName, const QPoint& createPoint){}

  bool                     m_bReadOnly;
  std::map<QString, bool>  m_contextMenuItemVisibility;
};

#endif // FLOWVIEW_H
