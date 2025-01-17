#ifndef CDIALOGEDITORDELEGATE_H
#define CDIALOGEDITORDELEGATE_H

#include "Systems/Project.h"

#include "Widgets/HtmlViewDelegate.h"

#include <QPointer>
#include <QTreeView>
#include <QUndoStack>

class CEditorModel;

class CDialogEditorDelegate : public CHtmlViewDelegate
{
public:
  CDialogEditorDelegate(QTreeView* pTree);
  ~CDialogEditorDelegate() override;

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

#endif // CDIALOGEDITORDELEGATE_H
