#ifndef SETTINGSSCREEN_H
#define SETTINGSSCREEN_H

#include "IAppStateScreen.h"
#include <QPointer>
#include <QWidget>
#include <memory>

class CHelpButtonOverlay;
class CSettings;
class QKeySequence;
namespace QtAV { class AVPlayer; }
namespace Ui {
  class CSettingsScreen;
}

class CSettingsScreen : public QWidget, public IAppStateScreen
{
  Q_OBJECT
  Q_INTERFACES(IAppStateScreen)

public:
  explicit CSettingsScreen(const std::shared_ptr<CWindowContext>& spWindowContext,
                           QWidget* pParent = nullptr);
  ~CSettingsScreen() override;

  bool CloseApplication() override { return false; }
  void Initialize() override;
  void Load() override;
  void Unload() override;

signals:
  void UnloadFinished() override;

protected slots:
  void on_pWindowModeComboBox_currentIndexChanged(qint32 iIndex);
  void on_pResolutionComboBox_currentIndexChanged(qint32 iIndex);
  void on_pAntialiasingCheckBox_toggled(bool bState);
  void on_pDropShadowCheckBox_toggled(bool bState);
  void on_pMipMapCheckBox_toggled(bool bState);
  void on_pSmoothingCheckBox_toggled(bool bState);
  void on_pFontComboBox_currentFontChanged(const QFont& font);
  void on_pStyleComboBox_currentIndexChanged(qint32 iIndex);
  void on_pEditorLayoutComboBox_currentIndexChanged(qint32 iIndex);
  void on_pHideTextBoxSpinBox_valueChanged(qint32 iValue);
  void on_pCaseSensitiveCheckBox_toggled(bool bState);
  void on_pCodeFontComboBox_currentFontChanged(const QFont& font);
  void on_pShowWhiteSpaceCheckBox_toggled(bool bState);
  void on_pCodeThemeComboBox_currentIndexChanged(qint32 iIndex);
  void on_pFolderLineEdit_editingFinished();
  void on_BrowseButton_clicked();
  void on_pMuteCheckBox_stateChanged(qint32 iState);
  void on_pVolumeSlider_sliderReleased();
  void on_pMetronomeVolumeSlider_sliderReleased();
  void on_pMetronomeSFXComboBox_currentIndexChanged(qint32 iIndex);
  void on_pDominantHandComboBox_currentIndexChanged(qint32 iIndex);
  void on_pPauseWhenNotActiveCheckBox_stateChanged(qint32 iState);
  void on_pOfflineModeCheckBox_stateChanged(qint32 iState);
  void on_pShowPushNotificationsCeckBox_stateChanged(qint32 iState);
  void on_pConnectOnStartupCheckBox_toggled(bool bState);
  void on_pVibrateCmdCheckBox_toggled(bool bState);
  void on_pLinearCmdCheckBox_toggled(bool bState);
  void on_pRotateCmdCheckBox_toggled(bool bState);
  void on_pConnectPushButton_clicked();
  void on_pStyleHotoadCheckBox_stateChanged(qint32 iState);
  void on_pBackButton_clicked();
  void SlotKeySequenceChanged(const QKeySequence& keySequence);
  void SlotMetronomeSfxItemHovered(qint32 iIndex);
  void SlotSoundPlaybackFinished();

private:
  std::unique_ptr<Ui::CSettingsScreen> m_spUi;
  std::unique_ptr<QtAV::AVPlayer>      m_spPlayer;
  std::shared_ptr<CSettings>           m_spSettings;
  QPointer<CHelpButtonOverlay>         m_pHelpButtonOverlay;
  QStringList                          m_vsSoundsToPlay;
};

#endif // SETTINGSSCREEN_H
