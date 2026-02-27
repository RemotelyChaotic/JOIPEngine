#ifndef CCOMMANDADDNEWFLOW_H
#define CCOMMANDADDNEWFLOW_H

#include "Systems/Database/Project.h"

#include <QUndoCommand>

#include <memory>

class CDatabaseManager;

class CCommandAddNewFlow : public QUndoCommand
{
public:
  CCommandAddNewFlow(const tspProject& spProject,
                     QPointer<QWidget> pParentForDialog,
                     QUndoCommand* pParent = nullptr);

  ~CCommandAddNewFlow();

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

#endif // CCOMMANDADDNEWFLOW_H
