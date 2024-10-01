#ifndef DIALOGTAGSEDITOROVERLAY_H
#define DIALOGTAGSEDITOROVERLAY_H

#include "Systems/Project.h"
#include "Widgets/OverlayBase.h"

#include <QPointer>
#include <memory>
#include <vector>

class CDatabaseManager;
class CDialogEditorTreeModel;
class CTagCompleter;
class QPushButton;
class QStandardItemModel;
class QUndoStack;
namespace Ui {
  class CDialogTagsEditorOverlay;
}

class CDialogTagsEditorOverlay : public COverlayBase
{
  Q_OBJECT

public:
  explicit CDialogTagsEditorOverlay(QWidget* pParent = nullptr);
  ~CDialogTagsEditorOverlay();

  void SetPath(const QStringList& vsPath);
  void SetProject(const tspProject& spCurrentProject);
  void SetUndoStack(QPointer<QUndoStack> pUndoStack);
  void SetModel(QPointer<CDialogEditorTreeModel> pModel);

public slots:
  void Climb() override;
  void Hide() override;
  void Show() override;
  void Resize() override;

signals:
  void SignalTagsChanged();

protected slots:
  void on_pLineEdit_editingFinished();
  void on_pDescribtionLineEdit_editingFinished();
  void on_pConfirmButton_clicked();
  void SlotRemoveTagClicked();
  void SlotTagAdded(const QString& sName);
  void SlotTagRemoved(const QString& sName);

private:
  void Initialize();
  void SortTags(std::vector<std::shared_ptr<SLockableTagData>>& vspTags);
  void TagAdded(QPushButton* pButton, const QString& sTag);
  void TagRemoved(const QStringList& vsTags);

  std::unique_ptr<Ui::CDialogTagsEditorOverlay> m_spUi;
  tspProject                                    m_spCurrentProject;
  std::weak_ptr<CDatabaseManager>               m_wpDbManager;
  QPointer<QUndoStack>                          m_pUndoStack;
  QPointer<CDialogEditorTreeModel>              m_pModel;
  QPointer<CTagCompleter>                       m_pCompleter;
  QPointer<QStandardItemModel>                  m_pCompleterModel;
  QStringList                                   m_vsPath;
};

#endif // DIALOGTAGSEDITOROVERLAY_H
