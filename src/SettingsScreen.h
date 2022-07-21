#ifndef SETTINGSSCREEN_H
#define SETTINGSSCREEN_H

#include "IAppStateScreen.h"
#include <QPointer>
#include <QWidget>
#include <memory>

class CHelpButtonOverlay;
class CSettings;
class QKeySequence;
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
  void on_pFontComboBox_currentFontChanged(const QFont& font);
  void on_pStyleComboBox_currentIndexChanged(qint32 iIndex);
  void on_pEditorLayoutComboBox_currentIndexChanged(qint32 iIndex);
  void on_pFolderLineEdit_editingFinished();
  void on_BrowseButton_clicked();
  void on_pMuteCheckBox_stateChanged(qint32 iState);
  void on_pVolumeSlider_sliderReleased();
  void on_pPauseWhenNotActiveCheckBox_stateChanged(qint32 iState);
  void on_pOfflineModeCheckBox_stateChanged(qint32 iState);
  void on_pShowPushNotificationsCeckBox_stateChanged(qint32 iState);
  void on_pStyleHotoadCheckBox_stateChanged(qint32 iState);
  void on_pBackButton_clicked();
  void SlotKeySequenceChanged(const QKeySequence& keySequence);

private:
  std::unique_ptr<Ui::CSettingsScreen> m_spUi;
  std::shared_ptr<CSettings>           m_spSettings;
  QPointer<CHelpButtonOverlay>         m_pHelpButtonOverlay;
};

#endif // SETTINGSSCREEN_H
