#ifndef CCOMMANDNODEREMOVED_H
#define CCOMMANDNODEREMOVED_H

#include <QPointer>
#include <QPointF>
#include <QUndoCommand>
#include <QUuid>
#include <vector>

class CFlowScene;
class CFlowView;

class CCommandNodesRemoved : public QUndoCommand
{
public:
  CCommandNodesRemoved(QPointer<CFlowView> pFlowView,
                       const std::vector<QUuid>& vIds,
                       QPointer<QUndoStack> pUndoStack,
                       QUndoCommand* pParent = nullptr);
  ~CCommandNodesRemoved();

  void undo() override;
  void redo() override;

  int id() const override;
  bool mergeWith(const QUndoCommand* pOther) override;

protected:
  QPointer<CFlowView>          m_pFlowView;
  QPointer<CFlowScene>         m_pScene;
  QPointer<QUndoStack>         m_pUndoStack;
  std::vector<QUuid>           m_vIds;
  std::map<QUuid, QJsonObject> m_nodesSaved;
};

#endif // CCOMMANDNODEREMOVED_H
