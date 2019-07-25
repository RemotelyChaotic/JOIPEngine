#ifndef SETTINGSSCREEN_H
#define SETTINGSSCREEN_H

#include "IAppStateScreen.h"
#include <QWidget>
#include <memory>

class CSettings;
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

  void Initialize();

  void Load() override;
  void Unload() override;

protected slots:
  void on_pResolutionComboBox_currentIndexChanged(qint32 iIndex);
  void on_pFolderLineEdit_editingFinished();
  void on_pBrowseButton_clicked();
  void on_pBackButton_clicked();

private:  
  std::unique_ptr<Ui::CSettingsScreen> m_spUi;
  std::shared_ptr<CSettings>           m_spSettings;
};

#endif // SETTINGSSCREEN_H
