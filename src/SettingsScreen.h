#ifndef SETTINGSSCREEN_H
#define SETTINGSSCREEN_H

#include <QWidget>
#include <memory>

namespace Ui {
  class CSettingsScreen;
}

class CSettingsScreen : public QWidget
{
  Q_OBJECT

public:
  explicit CSettingsScreen(QWidget *parent = nullptr);
  ~CSettingsScreen();

  void Initialize();

private:
  std::unique_ptr<Ui::CSettingsScreen> m_spUi;
  bool                                 m_bInitialized;
};

#endif // SETTINGSSCREEN_H
