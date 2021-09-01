#ifndef CCOMMANDNODEEDITED_H
#define CCOMMANDNODEEDITED_H

#include <QJsonObject>
#include <QPointer>
#include <QUndoCommand>
#include <QUuid>

class CFlowScene;

class CCommandNodeEdited : public QUndoCommand
{
public:
  CCommandNodeEdited(QPointer<CFlowScene> pFlowScene,
                     const QUuid& id,
                     const QJsonObject& oldState,
                     const QJsonObject& newState,
                     QUndoCommand* pParent = nullptr);
  ~CCommandNodeEdited();

  void undo() override;
  void redo() override;

  int id() const override;
  bool mergeWith(const QUndoCommand* pOther) override;

protected:
  QPointer<CFlowScene> m_pFlowScene;
  bool                 m_bFirstInsertGuard;
  QUuid                m_nodeId;
  QJsonObject          m_oldState;
  QJsonObject          m_newState;

private:
  void DoUndoRedo(const QJsonObject& obj);
};

#endif // CCOMMANDNODEEDITED_H
