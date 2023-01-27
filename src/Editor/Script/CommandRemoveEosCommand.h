#ifndef COMMANDREMOVEEOSCOMMAND_H
#define COMMANDREMOVEEOSCOMMAND_H

#include "EosScriptModelItem.h"

#include <QPointer>
#include <QUndoCommand>
#include <memory>

class CEosScriptModel;

class CCommandRemoveEosCommand : public QUndoCommand
{
public:
  CCommandRemoveEosCommand(QPointer<CEosScriptModel> pModel,
                           const SItemIndexPath& path,
                           QUndoCommand* pParent = nullptr);
  ~CCommandRemoveEosCommand();

  void undo() override;
  void redo() override;

  int id() const override;
  bool mergeWith(const QUndoCommand* pOther) override;

protected:
  QPointer<CEosScriptModel> m_pModel;
  SItemIndexPath m_pathParent;
  SItemIndexPath m_pathNodeParent;
  SItemIndexPath m_pathRemoved;
  qint32 m_iIndex;
  qint32 m_iChildIndex;
  EosScriptModelItem m_parentType;
  QString m_sType;
  QString m_sChildGroup;
  tInstructionMapValue m_args;
};

#endif // COMMANDREMOVEEOSCOMMAND_H
