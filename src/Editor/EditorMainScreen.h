#ifndef EDITORMAINSCREEN_H
#define EDITORMAINSCREEN_H

#include <QWidget>
#include <memory>

class CDatabaseManager;
namespace Ui {
  class CEditorMainScreen;
}
struct SProject;
typedef std::shared_ptr<SProject> tspProject;

class CEditorMainScreen : public QWidget
{
  Q_OBJECT

public:
  explicit CEditorMainScreen(QWidget* pParent = nullptr);
  ~CEditorMainScreen();

  void Initialize();
  void InitNewProject(const QString& sNewProjectName);
  void LoadProject(qint32 iId);
  void UnloadProject();

private:
  std::unique_ptr<Ui::CEditorMainScreen> m_spUi;
  tspProject                             m_spCurrentProject;
  std::weak_ptr<CDatabaseManager>        m_wpDbManager;
  bool                                   m_bInitialized;
};

#endif // EDITORMAINSCREEN_H
