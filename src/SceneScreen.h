#ifndef SCENESCREEN_H
#define SCENESCREEN_H

#include "IAppStateScreen.h"
#include <QWidget>
#include <memory>

namespace Ui {
  class CSceneScreen;
}

class CSceneScreen : public QWidget, public IAppStateScreen
{
  Q_OBJECT

public:
  explicit CSceneScreen(const std::shared_ptr<CWindowContext>& spWindowContext,
                        QWidget* pParent = nullptr);
  ~CSceneScreen() override;

  void Initialize();

  void Load() override;
  void Unload() override;

private:
  std::unique_ptr<Ui::CSceneScreen> m_spUi;
};

#endif // SCENESCREEN_H
