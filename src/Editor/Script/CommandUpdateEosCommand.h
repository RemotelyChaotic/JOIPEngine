#ifndef COMMANDUPDATEEOSCOMMAND_H
#define COMMANDUPDATEEOSCOMMAND_H

#include "Editor/Script/EosScriptEditorView.h"
#include "EosScriptModelItem.h"

#include <QPointer>
#include <QUndoCommand>
#include <memory>

class CEosScriptModel;

class CCommandUpdateEosCommand : public QUndoCommand
{
public:
  CCommandUpdateEosCommand(QPointer<CEosScriptModel> pModel,
                           QPointer<CEosScriptEditorView> pView,
                           const SItemIndexPath& pathCurrent,
                           const tInstructionMapValue& argsOld,
                           const tInstructionMapValue& argsNew,
                           bool bInvalidate,
                           QUndoCommand* pParent = nullptr);
  ~CCommandUpdateEosCommand();

  void undo() override;
  void redo() override;

  int id() const override;
  bool mergeWith(const QUndoCommand* pOther) override;

protected:
  void DoWithArgs(const tInstructionMapValue& args);

  QPointer<CEosScriptModel> m_pModel;
  QPointer<CEosScriptEditorView> m_pView;
  SItemIndexPath m_pathCurrent;
  tInstructionMapValue m_argsOld;
  tInstructionMapValue m_argsNew;
  bool m_bInvalidate;
};

#endif // COMMANDUPDATEEOSCOMMAND_H
