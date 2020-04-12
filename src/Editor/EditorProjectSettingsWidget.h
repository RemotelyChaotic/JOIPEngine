#ifndef EDITORPROJECTSETTINGSWIDGET_H
#define EDITORPROJECTSETTINGSWIDGET_H

#include "EditorWidgetBase.h"
#include "Enums.h"
#include "Systems/Resource.h"
#include <memory>

class CKinkSelectionOverlay;
namespace Ui {
  class CEditorProjectSettingsWidget;
}

class CEditorProjectSettingsWidget : public CEditorWidgetBase
{
  Q_OBJECT

public:
  explicit CEditorProjectSettingsWidget(QWidget *parent = nullptr);
  ~CEditorProjectSettingsWidget() override;

  void EditedProject() override;
  void Initialize() override;
  void LoadProject(tspProject spProject) override;
  void UnloadProject() override;
  void SaveProject() override;

protected slots:
  void on_pTitleLineEdit_editingFinished();
  void on_pProjectMajorVersion_valueChanged(qint32 iValue);
  void on_pProjectMinorVersion_valueChanged(qint32 iValue);
  void on_pProjectPatchVersion_valueChanged(qint32 iValue);
  void on_pDescribtionTextEdit_textChanged();
  void SlotAddKinksClicked();
  void SlotKinkOverlayClosed(bool bAccepted);
  void SlotProjectRenamed(qint32 iId);
  void SlotRemoveKinksClicked();

protected:
  void OnActionBarAboutToChange() override;
  void OnActionBarChanged() override;

private:
  std::unique_ptr<Ui::CEditorProjectSettingsWidget> m_spUi;
  std::unique_ptr<CKinkSelectionOverlay>            m_spKinkSelectionOverlay;
  tspProject                                        m_spCurrentProject;
  bool                                              m_bProjectHasBeenEdited;
};

#endif // EDITORPROJECTSETTINGSWIDGET_H
