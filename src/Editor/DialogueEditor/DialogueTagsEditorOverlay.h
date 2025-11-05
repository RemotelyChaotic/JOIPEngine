#ifndef DIALOGTAGSEDITOROVERLAY_H
#define DIALOGTAGSEDITOROVERLAY_H

#include "Systems/Database/Project.h"
#include "Widgets/OverlayBase.h"

#include <QPointer>
#include <memory>
#include <vector>

class CDatabaseManager;
class CDialogueEditorTreeModel;
class CTagCompleter;
class QPushButton;
class QStandardItemModel;
class QUndoStack;
namespace Ui {
  class CDialogueTagsEditorOverlay;
}

class CDialogueTagsEditorOverlay : public COverlayBase
{
  Q_OBJECT

public:
  explicit CDialogueTagsEditorOverlay(QWidget* pParent = nullptr);
  ~CDialogueTagsEditorOverlay();

  void SetPath(const QStringList& vsPath);
  void SetProject(const tspProject& spCurrentProject);
  void SetUndoStack(QPointer<QUndoStack> pUndoStack);
  void SetModel(QPointer<CDialogueEditorTreeModel> pModel);

public slots:
  void Climb() override;
  void Hide() override;
  void Show() override;
  void Resize() override;

signals:
  void SignalTagsChanged();

protected slots:
  void on_pLineEdit_editingFinished();
  void on_pDescriptionLineEdit_editingFinished();
  void on_pConfirmButton_clicked();
  void SlotRemoveTagClicked();
  void SlotTagAdded(const QString& sName);
  void SlotTagRemoved(const QString& sName);

private:
  void Initialize();
  void SortTags(std::vector<std::shared_ptr<SLockableTagData>>& vspTags);
  void TagAdded(QPushButton* pButton, const QString& sTag);
  void TagRemoved(const QStringList& vsTags);

  std::unique_ptr<Ui::CDialogueTagsEditorOverlay> m_spUi;
  tspProject                                    m_spCurrentProject;
  std::weak_ptr<CDatabaseManager>               m_wpDbManager;
  QPointer<QUndoStack>                          m_pUndoStack;
  QPointer<CDialogueEditorTreeModel>            m_pModel;
  QPointer<CTagCompleter>                       m_pCompleter;
  QPointer<QStandardItemModel>                  m_pCompleterModel;
  QStringList                                   m_vsPath;
};

#endif // DIALOGTAGSEDITOROVERLAY_H
