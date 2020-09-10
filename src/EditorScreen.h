#ifndef EDITORSCREEN_H
#define EDITORSCREEN_H

#include "IAppStateScreen.h"
#include <QWidget>
#include <memory>

namespace Ui {
  class CEditorScreen;
}

class CEditorScreen : public QWidget, public IAppStateScreen
{
  Q_OBJECT
  Q_INTERFACES(IAppStateScreen)

public:
  explicit CEditorScreen(const std::shared_ptr<CWindowContext>& spWindowContext,
                         QWidget* pParent = nullptr);
  ~CEditorScreen() override;

  void Initialize() override;
  void Load() override;
  void Unload() override;

signals:
  void UnloadFinished() override;

private slots:
  void SlotNewClicked(const QString& sNewProjectName, bool bTutorial);
  void SlotOpenClicked(qint32 iId);
  void SlotCancelClicked();
  void SlotExitClicked();

private:
  std::unique_ptr<Ui::CEditorScreen> m_spUi;
};

#endif // EDITORSCREEN_H
