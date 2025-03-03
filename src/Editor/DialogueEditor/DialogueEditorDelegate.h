#ifndef CDIALOGueEDITORDELEGATE_H
#define CDIALOGueEDITORDELEGATE_H

#include "Systems/Project.h"

#include "Widgets/HtmlViewDelegate.h"

#include <QPointer>
#include <QTreeView>
#include <QUndoStack>

class CEditorModel;

class CDialogueEditorDelegate : public CHtmlViewDelegate
{
public:
  CDialogueEditorDelegate(QTreeView* pTree);
  ~CDialogueEditorDelegate() override;

  void SetCurrentProject(const tspProject& spProject);
  void SetReadOnly(bool bReadOnly);
  void SetUndoStack(QPointer<QUndoStack> pUndo);

  QWidget* createEditor(QWidget* pParent,
                        const QStyleOptionViewItem& option,
                        const QModelIndex& index) const override;

  void setEditorData(QWidget* pEditor, const QModelIndex& index) const override;
  void setModelData(QWidget* pEditor,
                    QAbstractItemModel* pModel,
                    const QModelIndex& index) const override;

private:
  tspProject           m_spProject;
  QPointer<QTreeView>  m_pParent;
  QPointer<CEditorModel> m_pEditorModel;
  QPointer<QUndoStack> m_pUndo;
  bool m_bReadOnly = false;
};

#endif // CDIALOGueEDITORDELEGATE_H
