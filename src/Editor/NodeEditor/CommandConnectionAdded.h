#ifndef CCOMMANDCONNECTIONADDED_H
#define CCOMMANDCONNECTIONADDED_H

#include <QJsonObject>
#include <QPointer>
#include <QUndoCommand>
#include <QUuid>

class CFlowScene;

class CCommandConnectionAdded : public QUndoCommand
{
public:
  CCommandConnectionAdded(QPointer<CFlowScene> pFlowScene,
                          const QUuid& uuid,
                          const QJsonObject& serialized,
                          QUndoCommand* pParent = nullptr);
  ~CCommandConnectionAdded();

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

#endif // CCOMMANDCONNECTIONADDED_H
