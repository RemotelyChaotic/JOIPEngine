#ifndef CCOMMANDREMOVERESOURCE_H
#define CCOMMANDREMOVERESOURCE_H

#include "Systems/Project.h"
#include <QPointer>
#include <QUndoCommand>

class CDatabaseManager;

class CCommandRemoveResource : public QUndoCommand
{
public:
  CCommandRemoveResource(const tspProject& spProject,
                         const QStringList& vsFiles,
                         QUndoCommand* pParent = nullptr);
  ~CCommandRemoveResource();

  void undo() override;
  void redo() override;

  int id() const override;
  bool mergeWith(const QUndoCommand* pOther) override;

protected:
  tspResourceMap     m_removedResources;
  tspProject         m_spCurrentProject;
  std::weak_ptr<CDatabaseManager> m_wpDbManager;
  QStringList        m_vsResources;
};

#endif // CCOMMANDREMOVERESOURCE_H
