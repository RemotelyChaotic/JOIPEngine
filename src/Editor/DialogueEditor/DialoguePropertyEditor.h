#ifndef DIALOGPROPERTYEDITOR_H
#define DIALOGPROPERTYEDITOR_H

#include "Systems/DialogueTree.h"
#include "Systems/Project.h"

#include "Widgets/OverlayBase.h"

#include <memory>

class CDatabaseManager;
class CResourceTreeItemModel;
namespace Ui {
  class CDialoguePropertyEditor;
}

class CDialoguePropertyEditor : public COverlayBase
{
  Q_OBJECT

public:
  explicit CDialoguePropertyEditor(QWidget* pParent = nullptr);
  ~CDialoguePropertyEditor();

  void Initialize(CResourceTreeItemModel* pResourceTreeModel);
  void SetNode(QStringList vsPath, const std::shared_ptr<CDialogueNode>& spNode);
  void LoadProject(tspProject spProject);
  void UnloadProject();

signals:
  void SignalDialogChanged(QStringList vsPath, const std::shared_ptr<CDialogueNode>& spNode);

public slots:
  void Resize() override;
  void Climb() override;

protected slots:
  void on_pNameLineEdit_editingFinished();
  void on_pFileComboBox_currentIndexChanged(qint32 iIdx);
  void on_pConditionLineEdit_editingFinished();
  void on_pSkippableCheckBox_toggled(bool bChecked);
  void on_pAutoCheckBox_toggled(bool bChecked);
  void on_pWaitTimeEdit_timeChanged(const QTime &time);
  void on_pResourceLineEdit_editingFinished();
  void on_pTextEdit_textChanged();
  void on_CloseButton_clicked();
  void on_pFilter_SignalFilterChanged(const QString& sText);
  void on_pConfirmButton_clicked();
  void on_pCancelButton_clicked();
  void SlotCurrentChanged(const QModelIndex &current, const QModelIndex &previous);

private:
  std::unique_ptr<Ui::CDialoguePropertyEditor> m_spUi;
  tspProject                                 m_spCurrentProject;
  std::shared_ptr<CDialogueNode>             m_spNode;
  std::weak_ptr<CDatabaseManager>            m_wpDbManager;
  QStringList                                m_vsPath;
  QSize                                      m_preferredSize;
  bool                                       m_bInitialized = false;
};

#endif // DIALOGPROPERTYEDITOR_H
