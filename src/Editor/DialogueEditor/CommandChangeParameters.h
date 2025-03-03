#ifndef CCOMMANDCHANGEDialoguePARAMETERS_H
#define CCOMMANDCHANGEDialoguePARAMETERS_H

#include "Systems/DialogueTree.h"
#include "Systems/Project.h"
#include <QPointer>
#include <QUndoCommand>
#include <memory>

class CDialogueEditorTreeModel;

class CCommandChangeDialogueParameters : public QUndoCommand
{
public:
  CCommandChangeDialogueParameters(const tspProject& spProject,
                           const QStringList& vsPath,
                           std::shared_ptr<CDialogueNode> spNode,
                           QPointer<CDialogueEditorTreeModel> pModel,
                           QUndoCommand* pParent = nullptr);
  ~CCommandChangeDialogueParameters();

  void undo() override;
  void redo() override;

  int id() const override;
  bool mergeWith(const QUndoCommand* pOther) override;

private:
  void DoUndoRedo(const QByteArray& arr, const QStringList& vsPath);

  tspProject m_spProject;
  QPointer<CDialogueEditorTreeModel> m_pModel;
  QStringList m_vsPathNew;
  QStringList m_vsPathOld;
  QByteArray m_nodeNew;
  QByteArray m_nodeOld;
};

#endif // CCOMMANDCHANGEDialoguePARAMETERS_H
