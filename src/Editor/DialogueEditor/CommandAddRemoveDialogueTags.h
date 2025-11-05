#ifndef COMMANDADDREMOVEDIALOGTAGS_H
#define COMMANDADDREMOVEDIALOGTAGS_H

#include "Systems/DialogueTree.h"
#include "Systems/Database/Project.h"

#include <QStringList>
#include <QUndoCommand>
#include <functional>
#include <memory>

class CDatabaseManager;
class CDialogueEditorTreeModel;

class CCommandAddDialogueTag : public QUndoCommand
{
public:
  CCommandAddDialogueTag(const QString& sProject, const QStringList& vsPath,
                       std::shared_ptr<CDialogueNode> spNode,
                       const tspTag& spTag,
                       QPointer<CDialogueEditorTreeModel> pModel);
  ~CCommandAddDialogueTag();

  void undo() override;
  void redo() override;

  int id() const override;
  bool mergeWith(const QUndoCommand* pOther) override;

protected:
  QString                         m_sCurrentProject;
  std::weak_ptr<CDatabaseManager> m_wpDbManager;
  QPointer<CDialogueEditorTreeModel>m_pModel;
  tspTag                          m_spTag;
  QStringList                     m_vsPath;
};

//----------------------------------------------------------------------------------------
//
class CCommandRemoveDialogueTag : public QUndoCommand
{
public:
  CCommandRemoveDialogueTag(const QString& sProject, const QStringList& vsPath,
                          std::shared_ptr<CDialogueNode> spNode,
                          const tspTag& spTag,
                          QPointer<CDialogueEditorTreeModel> pModel);
  ~CCommandRemoveDialogueTag();

  void undo() override;
  void redo() override;

  int id() const override;
  bool mergeWith(const QUndoCommand* pOther) override;

protected:
  QString                         m_sCurrentProject;
  std::weak_ptr<CDatabaseManager> m_wpDbManager;
  QPointer<CDialogueEditorTreeModel>m_pModel;
  tspTag                          m_spTag;
  QStringList                     m_vsPath;
};

#endif // COMMANDADDREMOVEDIALOGTAGS_H
