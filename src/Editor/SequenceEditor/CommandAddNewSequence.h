#ifndef CCOMMANDADDNEWSEQUENCE_H
#define CCOMMANDADDNEWSEQUENCE_H

#include "Systems/Project.h"

#include <QUndoCommand>

#include <memory>

class CDatabaseManager;

class CCommandAddNewSequence : public QUndoCommand
{
public:
  CCommandAddNewSequence(const tspProject& spProject,
                         QPointer<QWidget> pParentForDialog,
                         QUndoCommand* pParent = nullptr);
  ~CCommandAddNewSequence();

  void undo() override;
  void redo() override;

  int id() const override;
  bool mergeWith(const QUndoCommand* pOther) override;

protected:
  tspResource                     m_addedResource;
  tspProject                      m_spCurrentProject;
  std::weak_ptr<CDatabaseManager> m_wpDbManager;
  QPointer<QWidget>               m_pParentForDialog;
};

#endif // CCOMMANDADDNEWSEQUENCE_H
