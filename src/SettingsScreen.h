#ifndef SETTINGSSCREEN_H
#define SETTINGSSCREEN_H

#include "IAppStateScreen.h"
#include <QWidget>
#include <memory>

class CSettings;
class QKeySequence;
namespace Ui {
  class CSettingsScreen;
}

class CSettingsScreen : public QWidget, public IAppStateScreen
{
  Q_OBJECT

public:
  explicit CSettingsScreen(const std::shared_ptr<CWindowContext>& spWindowContext,
                           QWidget* pParent = nullptr);
  ~CSettingsScreen() override;

  void Initialize() override;
  void Load() override;
  void Unload() override;

protected slots:
  void on_pFullscreenCheckBox_stateChanged(qint32 iState);
  void on_pResolutionComboBox_currentIndexChanged(qint32 iIndex);
  void on_pFontComboBox_currentFontChanged(const QFont& font);
  void on_pStyleComboBox_currentIndexChanged(qint32 iIndex);
  void on_pFolderLineEdit_editingFinished();
  void on_pBrowseButton_clicked();
  void on_pMuteCheckBox_stateChanged(qint32 iState);
  void on_pVolumeSlider_sliderReleased();
  void on_pOfflineModeCheckBox_stateChanged(qint32 iState);
  void on_pBackButton_clicked();
  void SlotKeySequenceChanged(const QKeySequence& keySequence);

private:
  std::unique_ptr<Ui::CSettingsScreen> m_spUi;
  std::shared_ptr<CSettings>           m_spSettings;
};

#endif // SETTINGSSCREEN_H
