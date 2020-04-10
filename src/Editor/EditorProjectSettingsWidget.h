#ifndef EDITORPROJECTSETTINGSWIDGET_H
#define EDITORPROJECTSETTINGSWIDGET_H

#include "EditorWidgetBase.h"
#include "Enums.h"
#include "Systems/Resource.h"
#include <memory>

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
  void SlotProjectRenamed(qint32 iId);

protected:
  void OnActionBarAboutToChange() override;
  void OnActionBarChanged() override;

private:
  std::unique_ptr<Ui::CEditorProjectSettingsWidget> m_spUi;
  tspProject                                        m_spCurrentProject;
  bool                                              m_bProjectHasBeenEdited;
};

#endif // EDITORPROJECTSETTINGSWIDGET_H
