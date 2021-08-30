#ifndef CCOMMANDNODEADDED_H
#define CCOMMANDNODEADDED_H

#include <QJsonObject>
#include <QPointer>
#include <QPointF>
#include <QUndoCommand>
#include <QUuid>

class CFlowView;
namespace QtNodes {
  class FlowScene;
}

class CCommandNodeAdded : public QUndoCommand
{
public:
  CCommandNodeAdded(QPointer<CFlowView> pFlowView,
                    const QString& sModelName,
                    const QPoint& addPoint,
                    QUndoCommand* pParent = nullptr);
  ~CCommandNodeAdded();

  void undo() override;
  void redo() override;

  int id() const override;
  bool mergeWith(const QUndoCommand* pOther) override;

protected:
  QPointer<CFlowView>          m_pFlowView;
  QPointer<QtNodes::FlowScene> m_pScene;
  QString                      m_sModelName;
  QPoint                       m_addPoint;
  QUuid                        m_nodeId;
  QJsonObject                  m_node;
};

#endif // CCOMMANDNODEADDED_H
