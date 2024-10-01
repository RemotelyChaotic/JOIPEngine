#ifndef CCOMMANDCHANGEPARAMETERS_H
#define CCOMMANDCHANGEPARAMETERS_H

#include "Systems/DialogTree.h"
#include "Systems/Project.h"
#include <QPointer>
#include <QUndoCommand>
#include <memory>

class CDialogEditorTreeModel;

class CCommandChangeParameters : public QUndoCommand
{
public:
  CCommandChangeParameters(const tspProject& spProject,
                           const QStringList& vsPath,
                           std::shared_ptr<CDialogNode> spNode,
                           QPointer<CDialogEditorTreeModel> pModel,
                           QUndoCommand* pParent = nullptr);
  ~CCommandChangeParameters();

  void undo() override;
  void redo() override;

  int id() const override;
  bool mergeWith(const QUndoCommand* pOther) override;

private:
  void DoUndoRedo(const QByteArray& arr, const QStringList& vsPath);

  tspProject m_spProject;
  QPointer<CDialogEditorTreeModel> m_pModel;
  QStringList m_vsPathNew;
  QStringList m_vsPathOld;
  QByteArray m_nodeNew;
  QByteArray m_nodeOld;
};

#endif // CCOMMANDCHANGEPARAMETERS_H
