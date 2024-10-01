#ifndef CCOMMANDADDREMOVENODE_H
#define CCOMMANDADDREMOVENODE_H

#include "Systems/DialogTree.h"
#include "Systems/Project.h"
#include <QPointer>
#include <QUndoCommand>
#include <memory>

class CDialogEditorTreeModel;

class CCommandAddDialogNode : public QUndoCommand
{
public:
  CCommandAddDialogNode(const tspProject& spProject,
                        const QStringList& vsPath,
                        qint32 iIndex,
                        std::shared_ptr<CDialogNode> spNode,
                        QPointer<CDialogEditorTreeModel> pModel,
                        QUndoCommand* pParent = nullptr);
  ~CCommandAddDialogNode();

  void undo() override;
  void redo() override;

  int id() const override;
  bool mergeWith(const QUndoCommand* pOther) override;

protected:
  tspProject m_spProject;
  QPointer<CDialogEditorTreeModel> m_pModel;
  QStringList m_vsPath;
  QByteArray m_node;
  qint32 m_iPos = -1;
};

//----------------------------------------------------------------------------------------
//
class CCommandRemoveDialogNode : public QUndoCommand
{
public:
  CCommandRemoveDialogNode(const tspProject& spProject,
                           const QStringList& vsPath,
                           QPointer<CDialogEditorTreeModel> pModel,
                           QUndoCommand* pParent = nullptr);
  ~CCommandRemoveDialogNode();

  void undo() override;
  void redo() override;

  int id() const override;
  bool mergeWith(const QUndoCommand* pOther) override;

protected:
  tspProject m_spProject;
  QPointer<CDialogEditorTreeModel> m_pModel;
  QStringList m_vsPath;
  QByteArray m_node;
  qint32 m_iPos = -1;
};

#endif // CCOMMANDADDREMOVENODE_H
