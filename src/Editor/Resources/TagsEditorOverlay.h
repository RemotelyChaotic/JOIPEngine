#ifndef TAGSEDITOROVERLAY_H
#define TAGSEDITOROVERLAY_H

#include "Systems/Project.h"
#include "Widgets/OverlayBase.h"

#include <QPointer>
#include <memory>
#include <vector>

class CDatabaseManager;
class CTagCompleter;
class QPushButton;
class QStandardItemModel;
class QUndoStack;
namespace Ui {
  class CTagsEditorOverlay;
}

class CTagsEditorOverlay : public COverlayBase
{
  Q_OBJECT

public:
  explicit CTagsEditorOverlay(QWidget* pParent = nullptr);
  ~CTagsEditorOverlay();

  void SetProject(const tspProject& spCurrentProject);
  void SetResource(const QString& sName);
  void SetUndoStack(QPointer<QUndoStack> pUndoStack);

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
  void SlotTagAdded(qint32 iProjId, const QString& sResource, const QString& sName);
  void SlotTagRemoved(qint32 iProjId, const QString& sResource, const QString& sName);

private:
  void Initialize();
  void SortTags(std::vector<std::shared_ptr<SLockableTagData>>& vspTags);
  void TagAdded(QPushButton* pButton, const QString& sTag);
  void TagRemoved(const QStringList& vsTags);

  std::unique_ptr<Ui::CTagsEditorOverlay> m_spUi;
  tspProject                              m_spCurrentProject;
  std::weak_ptr<CDatabaseManager>         m_wpDbManager;
  QPointer<QUndoStack>                    m_pUndoStack;
  QPointer<CTagCompleter>                 m_pCompleter;
  QPointer<QStandardItemModel>            m_pCompleterModel;
  QString                                 m_sResource;
};

#endif // TAGSEDITOROVERLAY_H
