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
  CFlowScene* Scene();

  void SetUndoStack(QPointer<QUndoStack> pUndoStack);
  QPointer<QUndoStack> UndoStack();

  void FitAllNodesInView();
  void OpenContextMenuAt(const QPoint& localPoint, const QPoint& createPoint = QPoint());

  bool IsReadOnly();
  void SetReadOnly(bool bReadOnly);

  void SetModelHiddenInContextMenu(const QString& sId, bool bHidden);
  bool IsModelHiddenInContextMenu(const QString& sId);

protected:
  void SlotClearSelectionTriggered();
  void SlotDeleteTriggered();

protected:
  void contextMenuEvent(QContextMenuEvent* pEvent) override;

private:
  bool                     m_bReadOnly;
  std::map<QString, bool>  m_contextMenuItemVisibility;
  QPointer<QUndoStack>     m_pUndoStack;
};

#endif // FLOWVIEW_H
