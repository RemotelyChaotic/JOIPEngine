#ifndef EDITORPROJECTSETTINGSWIDGET_H
#define EDITORPROJECTSETTINGSWIDGET_H

#include "EditorWidgetBase.h"
#include "Enums.h"
#include "Systems/Resource.h"
#include "ui_EditorProjectSettingsWidget.h"
#include "ui_EditorActionBar.h"
#include <memory>

class CKinkSelectionOverlay;
class CProjectSettingsTutorialStateSwitchHandler;
namespace Ui {
  class CEditorProjectSettingsWidget;
}
struct SKink;
typedef std::shared_ptr<SKink>      tspKink;

class CEditorProjectSettingsWidget : public CEditorWidgetBase
{
  Q_OBJECT

public:
  explicit CEditorProjectSettingsWidget(QWidget *parent = nullptr);
  ~CEditorProjectSettingsWidget() override;

  void EditedProject() override {}
  void Initialize() override;
  void LoadProject(tspProject spProject) override;
  void UnloadProject() override;
  void SaveProject() override;
  void OnHidden() override {};
  void OnShown() override {};

protected slots:
  void on_pTitleLineEdit_editingFinished();
  void on_pProjectMajorVersion_valueChanged(qint32 iValue);
  void on_pProjectMinorVersion_valueChanged(qint32 iValue);
  void on_pProjectPatchVersion_valueChanged(qint32 iValue);
  void on_pSoundEmitterCount_valueChanged(qint32 iValue);
  void on_pFontComboBox_currentFontChanged(const QFont& font);
  void on_pDescribtionTextEdit_textChanged();
  void on_pFetishLineEdit_editingFinished();
  void on_FetishOverlayButton_clicked();
  void SlotKinkChecked(const QModelIndex& index, bool bChecked);
  void SlotProjectRenamed(qint32 iId);
  void SlotRemoveKinkClicked();
  void SlotUndoForDescribtionAdded();

protected:
  void AddKinks(QStringList vsKinks);
  void AddKinksToView(const std::vector<tspKink>& vspKinks);
  void ClearKinkTagView();
  void RemoveKinks(QStringList vsKinks);
  void OnActionBarAboutToChange() override;
  void OnActionBarChanged() override;

private:
  std::unique_ptr<CKinkSelectionOverlay>                      m_spKinkSelectionOverlay;
  std::shared_ptr<Ui::CEditorProjectSettingsWidget>           m_spUi;
  std::shared_ptr<CProjectSettingsTutorialStateSwitchHandler> m_spTutorialStateSwitchHandler;
  std::vector<tspKink>                                        m_vspKinks;
  tspProject                                                  m_spCurrentProject;
};

DECLARE_EDITORWIDGET(CEditorProjectSettingsWidget, EEditorWidget::eProjectSettings)

#endif // EDITORPROJECTSETTINGSWIDGET_H
