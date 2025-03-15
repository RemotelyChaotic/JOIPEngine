#ifndef EDITORCHOICESCREEN_H
#define EDITORCHOICESCREEN_H

#include <QWidget>
#include <memory>

namespace Ui {
  class CEditorChoiceScreen;
}
class CDatabaseManager;

class CEditorChoiceScreen : public QWidget
{
  Q_OBJECT

public:
  explicit CEditorChoiceScreen(QWidget* pParent = nullptr);
  ~CEditorChoiceScreen();

  void Initialize();
  void Load();
  void Unload();

signals:
  void SignalNewClicked(const QString& sNewProjectName, bool bTutorial);
  void SignalOpenClicked(qint32 iId);
  void SignalCancelClicked();
  void SignalUnloadFinished();

protected slots:
  void on_pNewProjectButton_clicked();
  void on_pOpenProjectButton_clicked();
  void on_pCancelButton_clicked();

  void on_pProjectNameLineEdit_textChanged(const QString &text);
  void on_pCreateProjectButton_clicked();

  void on_pOpenExistingProjectButton_clicked();

private:
  std::unique_ptr<Ui::CEditorChoiceScreen> m_spUi;
  std::weak_ptr<CDatabaseManager>          m_wpDbManager;
  bool                                     m_bInitialized;
};

#endif // EDITORCHOICESCREEN_H
