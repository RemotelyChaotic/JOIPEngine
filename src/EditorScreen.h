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

public:
  explicit CEditorScreen(const std::shared_ptr<CWindowContext>& spWindowContext,
                         QWidget* pParent = nullptr);
  ~CEditorScreen() override;

  void Initialize();

  void Load() override;
  void Unload() override;

private slots:
  void SlotNewClicked(const QString& sNewProjectName);
  void SlotOpenClicked(qint32 iId);
  void SlotCancelClicked();
  void SlotExitClicked();

private:
  std::unique_ptr<Ui::CEditorScreen> m_spUi;
};

#endif // EDITORSCREEN_H
