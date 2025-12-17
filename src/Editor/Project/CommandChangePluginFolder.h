#ifndef CCOMMANDCHANGEPLUGINFOLDER_H
#define CCOMMANDCHANGEPLUGINFOLDER_H

#include "Systems/Database/Project.h"
#include <QLineEdit>
#include <QPointer>
#include <QUndoCommand>

class CCommandChangePluginFolder  : public QUndoCommand
{
public:
  CCommandChangePluginFolder(QPointer<QLineEdit> pLineEdit,
                     const std::function<void(void)>& fnOnUndoRedo,
                     QUndoCommand* pParent = nullptr);
  ~CCommandChangePluginFolder();

  void undo() override;
  void redo() override;

  int id() const override;
  bool mergeWith(const QUndoCommand* pOther) override;

protected:
  QPointer<QLineEdit> m_pLineEdit;
  std::function<void(void)> m_fnOnUndoRedo;
  QString                 m_sOriginalValue;
  QString                 m_sNewValue;
};

#endif // CCOMMANDCHANGEPLUGINFOLDER_H
