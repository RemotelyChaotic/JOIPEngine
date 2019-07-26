#ifndef PROJECTSELECTIONSCREEN_H
#define PROJECTSELECTIONSCREEN_H

#include "IAppStateScreen.h"
#include <QWidget>
#include <memory>

namespace Ui {
  class CSceneSelectionScreen;
}

class CSceneSelectionScreen : public QWidget, public IAppStateScreen
{
  Q_OBJECT

public:
  explicit CSceneSelectionScreen(const std::shared_ptr<CWindowContext>& spWindowContext,
                                 QWidget* pParent = nullptr);
  ~CSceneSelectionScreen() override;

  void Initialize();

  void Load() override;
  void Unload() override;

protected slots:
  void on_pOpenExistingProjectButton_clicked();
  void on_pCancelButton_clicked();

private:
  std::unique_ptr<Ui::CSceneSelectionScreen> m_spUi;
};

#endif // PROJECTSELECTIONSCREEN_H
