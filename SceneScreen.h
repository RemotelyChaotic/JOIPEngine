#ifndef SCENESCREEN_H
#define SCENESCREEN_H

#include <QWidget>
#include <memory>

namespace Ui {
  class CSceneScreen;
}

class CSceneScreen : public QWidget
{
  Q_OBJECT

public:
  explicit CSceneScreen(QWidget *parent = nullptr);
  ~CSceneScreen();

  void Initialize();

private:
  std::unique_ptr<Ui::CSceneScreen> m_spUi;
  bool                              m_bInitialized;
};

#endif // SCENESCREEN_H
