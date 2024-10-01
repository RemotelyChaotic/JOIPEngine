#ifndef COMMANDADDREMOVEDIALOGTAGS_H
#define COMMANDADDREMOVEDIALOGTAGS_H

#include "Systems/DialogTree.h"
#include "Systems/Project.h"

#include <QStringList>
#include <QUndoCommand>
#include <functional>
#include <memory>

class CDatabaseManager;
class CDialogEditorTreeModel;

class CCommandAddDialogTag : public QUndoCommand
{
public:
  CCommandAddDialogTag(const QString& sProject, const QStringList& vsPath,
                       std::shared_ptr<CDialogNode> spNode,
                       const tspTag& spTag,
                       QPointer<CDialogEditorTreeModel> pModel);
  ~CCommandAddDialogTag();

  void undo() override;
  void redo() override;

  int id() const override;
  bool mergeWith(const QUndoCommand* pOther) override;

protected:
  QString                         m_sCurrentProject;
  std::weak_ptr<CDatabaseManager> m_wpDbManager;
  QPointer<CDialogEditorTreeModel>m_pModel;
  tspTag                          m_spTag;
  QStringList                     m_vsPath;
};

//----------------------------------------------------------------------------------------
//
class CCommandRemoveDialogTag : public QUndoCommand
{
public:
  CCommandRemoveDialogTag(const QString& sProject, const QStringList& vsPath,
                          std::shared_ptr<CDialogNode> spNode,
                          const tspTag& spTag,
                          QPointer<CDialogEditorTreeModel> pModel);
  ~CCommandRemoveDialogTag();

  void undo() override;
  void redo() override;

  int id() const override;
  bool mergeWith(const QUndoCommand* pOther) override;

protected:
  QString                         m_sCurrentProject;
  std::weak_ptr<CDatabaseManager> m_wpDbManager;
  QPointer<CDialogEditorTreeModel>m_pModel;
  tspTag                          m_spTag;
  QStringList                     m_vsPath;
};

#endif // COMMANDADDREMOVEDIALOGTAGS_H
