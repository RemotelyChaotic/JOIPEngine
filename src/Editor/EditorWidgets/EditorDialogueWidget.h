#ifndef EDITORDIALOGWIDGET_H
#define EDITORDIALOGWIDGET_H

#include "EditorWidgetBase.h"
#include "ui_EditorDialogueWidget.h"
#include "ui_EditorActionBar.h"
#include <QPointer>
#include <memory>

class CDialogueEditorSortFilterProxyModel;
class CDialogueEditorTreeModel;
class CDialoguePropertyEditor;
class CDialogueTagsEditorOverlay;
struct CDialogueNode;
namespace Ui {
  class CEditorDialogueWidget;
}

class CEditorDialogueWidget : public CEditorWidgetBase
{
  Q_OBJECT

public:
  explicit CEditorDialogueWidget(QWidget *parent = nullptr);
  ~CEditorDialogueWidget();

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
  void SlotAddDialogueClicked();
  void SlotAddDialogueFragmentClicked();
  void SlotAddDialogueCategoryClicked();
  void SlotRemoveDialogueClicked();
  void SlotEditDialogueClicked();
  void SlotDialogueChanged(QStringList vsPath, const std::shared_ptr<CDialogueNode>& spNode);
  void SlotEditDialogueTagsClicked();
  void SlotExpandAllNodes();
  void SlotFilterChanged(const QString& sText);
  void SlotCopy();
  void SlotPaste();

private:
  void ShowContextMenu(CDialogueEditorTreeModel* pModel, const QModelIndex& idx,
                       const QPoint& globalPos);

  std::unique_ptr<Ui::CEditorDialogueWidget>                    m_spUi;
  std::unique_ptr<CDialoguePropertyEditor>                      m_spPropertiesOverlay;
  std::unique_ptr<CDialogueTagsEditorOverlay>                   m_spTagOverlay;
  tspProject                                                  m_spCurrentProject;
  QPointer<CDialogueEditorSortFilterProxyModel>                 m_pProxy;
  QPointer<QAction>                                           m_pCopyAction;
  QPointer<QAction>                                           m_pPasteAction;
};

#endif // EDITORDIALOGWIDGET_H
