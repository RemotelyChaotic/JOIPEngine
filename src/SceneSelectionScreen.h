#ifndef PROJECTSELECTIONSCREEN_H
#define PROJECTSELECTIONSCREEN_H

#include <QWidget>
#include <memory>

namespace Ui {
  class CSceneSelectionScreen;
}

class CSceneSelectionScreen : public QWidget
{
  Q_OBJECT

public:
  explicit CSceneSelectionScreen(QWidget *parent = nullptr);
  ~CSceneSelectionScreen();

  void Initialize();

private:
  std::unique_ptr<Ui::CSceneSelectionScreen> m_spUi;
  bool                                       m_bInitialized;
};

#endif // PROJECTSELECTIONSCREEN_H
