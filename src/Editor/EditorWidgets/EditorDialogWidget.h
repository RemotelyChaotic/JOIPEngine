#ifndef EDITORDIALOGWIDGET_H
#define EDITORDIALOGWIDGET_H

#include "EditorWidgetBase.h"
#include "ui_EditorDialogWidget.h"
#include "ui_EditorActionBar.h"
#include <QPointer>
#include <memory>

class CDialogEditorSortFilterProxyModel;
class CDialogEditorTreeModel;
class CDialogPropertyEditor;
class CDialogTagsEditorOverlay;
struct CDialogNode;
namespace Ui {
  class CEditorDialogWidget;
}

class CEditorDialogWidget : public CEditorWidgetBase
{
  Q_OBJECT

public:
  explicit CEditorDialogWidget(QWidget *parent = nullptr);
  ~CEditorDialogWidget();

  void EditedProject() override {}
  void Initialize() override;
  void LoadProject(tspProject spCurrentProject) override;
  void UnloadProject() override;
  void SaveProject() override;
  void OnHidden() override {}
  void OnShown() override {}

protected:
  void OnActionBarAboutToChange() override;
  void OnActionBarChanged() override;
  bool eventFilter(QObject* pObj, QEvent* pEvt) override;
  void dragEnterEvent(QDragEnterEvent* pEvent) override;
  void dropEvent(QDropEvent* pEvent) override;

protected slots:
  void SlotAddDialogClicked();
  void SlotAddDialogFragmentClicked();
  void SlotAddDialogcategoryClicked();
  void SlotRemoveDialogClicked();
  void SlotEditDialogClicked();
  void SlotDialogChanged(QStringList vsPath, const std::shared_ptr<CDialogNode>& spNode);
  void SlotEditDialogTagsClicked();
  void SlotExpandAllNodes();
  void SlotFilterChanged(const QString& sText);
  void SlotCopy();
  void SlotPaste();

private:
  void ShowContextMenu(CDialogEditorTreeModel* pModel, const QModelIndex& idx,
                       const QPoint& globalPos);

  std::unique_ptr<Ui::CEditorDialogWidget>                    m_spUi;
  std::unique_ptr<CDialogPropertyEditor>                      m_spPropertiesOverlay;
  std::unique_ptr<CDialogTagsEditorOverlay>                   m_spTagOverlay;
  tspProject                                                  m_spCurrentProject;
  QPointer<CDialogEditorSortFilterProxyModel>                 m_pProxy;
  QPointer<QAction>                                           m_pCopyAction;
  QPointer<QAction>                                           m_pPasteAction;
};

#endif // EDITORDIALOGWIDGET_H
