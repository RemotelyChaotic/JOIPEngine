#ifndef COMMANDINSERTEOSCOMMAND_H
#define COMMANDINSERTEOSCOMMAND_H

#include "EosScriptModelItem.h"

#include <QPointer>
#include <QUndoCommand>
#include <memory>

class CEosScriptModel;

class CCommandInsertEosCommand : public QUndoCommand
{
public:
  CCommandInsertEosCommand(QPointer<CEosScriptModel> pModel,
                           const SItemIndexPath& pathCurrent,
                           QString sType,
                           const tInstructionMapValue& args,
                           QUndoCommand* pParent = nullptr);
  ~CCommandInsertEosCommand();

  void undo() override;
  void redo() override;

  int id() const override;
  bool mergeWith(const QUndoCommand* pOther) override;

protected:
  QPointer<CEosScriptModel> m_pModel;
  SItemIndexPath m_pathCurrent;
  SItemIndexPath m_pathInserted;
  QString m_sType;
  tInstructionMapValue m_args;
};

#endif // COMMANDINSERTEOSCOMMAND_H
