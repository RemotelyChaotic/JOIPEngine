#ifndef EDITORPROJECTSETTINGSWIDGET_H
#define EDITORPROJECTSETTINGSWIDGET_H

#include "EditorWidgetBase.h"
#include "Enums.h"
#include "ui_EditorProjectSettingsWidget.h"
#include "ui_EditorActionBar.h"
#include <memory>

struct SSaveDataData;
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
  void on_pDefaultLayoutComboBox_currentIndexChanged(qint32 iIdx);
  void on_pToyCommandComboBox_currentIndexChanged(qint32 iIdx);
  void on_AddLayoutButton_clicked();
  void on_pDescribtionTextEdit_textChanged();
  void on_pFetishLineEdit_editingFinished();
  void on_FetishOverlayButton_clicked();
  void on_pAchievementLineEdit_editingFinished();
  void on_AddNewAchievement_clicked();
  void SlotChangeAchievementData(const SSaveDataData& oldData, const SSaveDataData& saveData);
  void SlotKinkChecked(const QModelIndex& index, bool bChecked);
  void SlotProjectRenamed(qint32 iId);
  void SlotRemoveAchievement(const SSaveDataData& saveData);
  void SlotRemoveKinkClicked();
  void SlotResourceAdded(qint32 iProjId, const QString& sName);
  void SlotResourceRemoved(qint32 iProjId, const QString& sName);
  void SlotResourceRenamed(qint32 iProjId, const QString& sOldName,const QString& sName);
  void SlotUndoForDescribtionAdded();

protected:
  void AddAchievement(const SSaveDataData& saveData, bool bEmitChanged);
  void AddKinks(QStringList vsKinks);
  void ClearAchievements();
  void OnActionBarAboutToChange() override;
  void OnActionBarChanged() override;

private:
  std::unique_ptr<CKinkSelectionOverlay>                      m_spKinkSelectionOverlay;
  std::shared_ptr<Ui::CEditorProjectSettingsWidget>           m_spUi;
  std::shared_ptr<CProjectSettingsTutorialStateSwitchHandler> m_spTutorialStateSwitchHandler;
  tspProject                                                  m_spCurrentProject;
};

#endif // EDITORPROJECTSETTINGSWIDGET_H
