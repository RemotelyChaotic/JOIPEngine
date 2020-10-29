#ifndef FLOWVIEW_H
#define FLOWVIEW_H

#include <nodes/FlowView>
#include <map>

class CFlowView : public QtNodes::FlowView
{
  Q_OBJECT

public:
  CFlowView(QWidget* pParent = nullptr);
  CFlowView(QtNodes::FlowScene* pScene, QWidget* pParent = nullptr);

  CFlowView(const CFlowView&) = delete;
  CFlowView operator=(const CFlowView&) = delete;

  ~CFlowView() override;

  void OpenContextMenuAt(const QPoint& localPoint, const QPoint& createPoint = QPoint());

  bool IsReadOnly();
  void SetReadOnly(bool bReadOnly);

  void SetModelHiddenInContextMenu(const QString& sId, bool bHidden);
  bool IsModelHiddenInContextMenu(const QString& sId);

  using QtNodes::FlowView::scene;

protected:
  void contextMenuEvent(QContextMenuEvent* pEvent) override;

private:
  bool                     m_bReadOnly;
  std::map<QString, bool>  m_contextMenuItemVisibility;
};

#endif // FLOWVIEW_H
