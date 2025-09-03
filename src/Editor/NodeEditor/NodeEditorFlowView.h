#ifndef CNODEEDITORFLOWVIEW_H
#define CNODEEDITORFLOWVIEW_H

#include "Systems/Nodes/FlowView.h"

#include <QUndoStack>

class CNodeEditorFlowScene;

class CNodeEditorFlowView : public CFlowView
{
  Q_OBJECT

public:
  CNodeEditorFlowView(QWidget* pParent = nullptr);
  CNodeEditorFlowView(CNodeEditorFlowScene* pScene, QWidget* pParent = nullptr);

  CNodeEditorFlowView(const CNodeEditorFlowView&) = delete;
  CNodeEditorFlowView operator=(const CNodeEditorFlowView&) = delete;

  ~CNodeEditorFlowView() override;

  void SetSceneImpl(CFlowScene* pScene) override;

  void SetUndoStack(QPointer<QUndoStack> pUndoStack);
  QPointer<QUndoStack> UndoStack();

  void OpenContextMenuAt(const QPoint& localPoint, const QPoint& createPoint = QPoint()) override;

protected slots:
  void SlotClearSelectionTriggered() override;
  void SlotDeleteTriggered() override;

protected:
  void keyPressEvent(QKeyEvent *event) override;

  void NodeAboutToBeCreated(const QString& modelName, const QPoint& localPoint) override;

private:
  QPointer<QUndoStack>     m_pUndoStack;
};

#endif // CNODEEDITORFLOWVIEW_H
