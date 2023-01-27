#ifndef COMMANDTOGGLEDEOSCOMMAND_H
#define COMMANDTOGGLEDEOSCOMMAND_H

#include "EosScriptModelItem.h"

#include <QPointer>
#include <QUndoCommand>
#include <memory>

class CEosScriptModel;

class CCommandToggledEosCommand : public QUndoCommand
{
public:
  CCommandToggledEosCommand(QPointer<CEosScriptModel> pModel,
                            const SItemIndexPath& path,
                            bool bOldValue,
                            bool bNewValue,
                            QUndoCommand* pParent = nullptr);
  ~CCommandToggledEosCommand();

  void undo() override;
  void redo() override;

  int id() const override;
  bool mergeWith(const QUndoCommand* pOther) override;

protected:
  QPointer<CEosScriptModel> m_pModel;
  SItemIndexPath m_path;
  bool m_bOldValue;
  bool m_bNewValue;
};

#endif // COMMANDTOGGLEDEOSCOMMAND_H
