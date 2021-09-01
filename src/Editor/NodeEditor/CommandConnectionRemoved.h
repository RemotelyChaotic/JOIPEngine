#ifndef CCOMMANDCONNECTIONREMOVED_H
#define CCOMMANDCONNECTIONREMOVED_H

#include <QJsonObject>
#include <QPointer>
#include <QUndoCommand>
#include <QUuid>

class CFlowScene;


class CCommandConnectionRemoved : public QUndoCommand
{
public:
  CCommandConnectionRemoved(QPointer<CFlowScene> pFlowScene,
                            const QUuid& uuid,
                            const QJsonObject& serialized,
                            QUndoCommand* pParent = nullptr);
  ~CCommandConnectionRemoved();

  void undo() override;
  void redo() override;

  int id() const override;
  bool mergeWith(const QUndoCommand* pOther) override;

protected:
  QPointer<CFlowScene> m_pFlowScene;
  bool                 m_bFirstInsertGuard;
  QUuid                m_connId;
  QJsonObject          m_connection;
};

#endif // CCOMMANDCONNECTIONREMOVED_H
