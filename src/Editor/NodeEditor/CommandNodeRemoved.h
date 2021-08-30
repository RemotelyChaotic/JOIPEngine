#ifndef CCOMMANDNODEREMOVED_H
#define CCOMMANDNODEREMOVED_H

#include <QPointer>
#include <QPointF>
#include <QUndoCommand>
#include <QUuid>

class CFlowView;
namespace QtNodes {
  class FlowScene;
}

class CCommandNodesRemoved : public QUndoCommand
{
public:
  CCommandNodesRemoved(QPointer<CFlowView> pFlowView,
                       const std::vector<QUuid>& vIds,
                       QUndoCommand* pParent = nullptr);
  ~CCommandNodesRemoved();

  void undo() override;
  void redo() override;

  int id() const override;
  bool mergeWith(const QUndoCommand* pOther) override;

protected:
  QPointer<CFlowView>          m_pFlowView;
  QPointer<QtNodes::FlowScene> m_pScene;
  std::vector<QUuid>           m_vIds;
};

#endif // CCOMMANDNODEREMOVED_H
