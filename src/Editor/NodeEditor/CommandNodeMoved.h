#ifndef CCOMMANDNODEMOVED_H
#define CCOMMANDNODEMOVED_H

#include <QPointer>
#include <QUndoCommand>
#include <QUuid>

class CFlowScene;
class CFlowView;

class CCommandNodeMoved : public QUndoCommand
{
public:
  CCommandNodeMoved(QPointer<CFlowView> pFlowView,
                    const QUuid& id,
                    const QPointF& from,
                    const QPointF& to,
                    QUndoCommand* pParent = nullptr);
  ~CCommandNodeMoved();

  void undo() override;
  void redo() override;

  int id() const override;
  bool mergeWith(const QUndoCommand* pOther) override;

protected:
  QPointer<CFlowView>          m_pFlowView;
  QPointer<CFlowScene>         m_pScene;
  QUuid                        m_nodeId;
  QPointF                      m_from;
  QPointF                      m_to;

private:
  void DoUndoRedo(const QPointF& point);
};

#endif // CCOMMANDNODEMOVED_H
