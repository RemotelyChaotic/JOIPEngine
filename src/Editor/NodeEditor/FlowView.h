#ifndef FLOWVIEW_H
#define FLOWVIEW_H

#include <nodes/FlowView>

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
};

#endif // FLOWVIEW_H
