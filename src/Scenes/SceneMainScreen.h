#ifndef SCENEMAINSCREEN_H
#define SCENEMAINSCREEN_H

#include <QWidget>
#include <memory>

class CDatabaseManager;
class CProjectRunner;
class CSettings;
namespace Ui {
  class CSceneMainScreen;
}
struct SProject;
typedef std::shared_ptr<SProject> tspProject;

class CSceneMainScreen : public QWidget
{
  Q_OBJECT

public:
  explicit CSceneMainScreen(QWidget* pParent = nullptr);
  ~CSceneMainScreen();

  void Initialize();
  void LoadProject(qint32 iId);
  void UnloadProject();

private:
  std::unique_ptr<Ui::CSceneMainScreen>                       m_spUi;
  std::unique_ptr<CProjectRunner>                             m_spProjectRunner;
  std::shared_ptr<CSettings>                                  m_spSettings;
  tspProject                                                  m_spCurrentProject;
  std::weak_ptr<CDatabaseManager>                             m_wpDbManager;
  bool                                                        m_bInitialized;
};

#endif // SCENEMAINSCREEN_H
