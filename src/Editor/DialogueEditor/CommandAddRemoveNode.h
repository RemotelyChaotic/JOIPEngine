#ifndef CCOMMANDADDREMOVENODE_H
#define CCOMMANDADDREMOVENODE_H

#include "Systems/DialogueTree.h"
#include "Systems/Project.h"
#include <QPointer>
#include <QUndoCommand>
#include <memory>

class CDialogueEditorTreeModel;

class CCommandAddDialogueNode : public QUndoCommand
{
public:
  CCommandAddDialogueNode(const tspProject& spProject,
                        const QStringList& vsPath,
                        qint32 iIndex,
                        std::shared_ptr<CDialogueNode> spNode,
                        QPointer<CDialogueEditorTreeModel> pModel,
                        QUndoCommand* pParent = nullptr);
  ~CCommandAddDialogueNode();

  void undo() override;
  void redo() override;

  int id() const override;
  bool mergeWith(const QUndoCommand* pOther) override;

protected:
  tspProject m_spProject;
  QPointer<CDialogueEditorTreeModel> m_pModel;
  QStringList m_vsPath;
  QByteArray m_node;
  qint32 m_iPos = -1;
};

//----------------------------------------------------------------------------------------
//
class CCommandRemoveDialogueNode : public QUndoCommand
{
public:
  CCommandRemoveDialogueNode(const tspProject& spProject,
                           const QStringList& vsPath,
                           QPointer<CDialogueEditorTreeModel> pModel,
                           QUndoCommand* pParent = nullptr);
  ~CCommandRemoveDialogueNode();

  void undo() override;
  void redo() override;

  int id() const override;
  bool mergeWith(const QUndoCommand* pOther) override;

protected:
  tspProject m_spProject;
  QPointer<CDialogueEditorTreeModel> m_pModel;
  QStringList m_vsPath;
  QByteArray m_node;
  qint32 m_iPos = -1;
};

#endif // CCOMMANDADDREMOVENODE_H
