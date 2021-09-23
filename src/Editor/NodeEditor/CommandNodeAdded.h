#ifndef CCOMMANDNODEADDED_H
#define CCOMMANDNODEADDED_H

#include <QJsonObject>
#include <QPointer>
#include <QPointF>
#include <QUndoCommand>
#include <QUuid>

class CFlowScene;
class CFlowView;

class CCommandNodeAdded : public QUndoCommand
{
public:
  CCommandNodeAdded(QPointer<CFlowView> pFlowView,
                    const QString& sModelName,
                    const QPointF& addPoint,
                    QPointer<QUndoStack> pUndoStack,
                    QUndoCommand* pParent = nullptr);
  CCommandNodeAdded(QPointer<CFlowView> pFlowView,
                    QUuid nodeId,
                    QPointer<QUndoStack> pUndoStack,
                    QUndoCommand* pParent = nullptr);
  ~CCommandNodeAdded();

  void undo() override;
  void redo() override;

  int id() const override;
  bool mergeWith(const QUndoCommand* pOther) override;

protected:
  QPointer<CFlowView>          m_pFlowView;
  QPointer<CFlowScene>         m_pScene;
  QPointer<QUndoStack>         m_pUndoStack;
  QString                      m_sModelName;
  QPointF                      m_addPoint;
  QUuid                        m_nodeId;
  QJsonObject                  m_node;
  bool                         m_bFirstRedo;
};

#endif // CCOMMANDNODEADDED_H
