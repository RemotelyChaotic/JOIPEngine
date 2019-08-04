#ifndef EDITORMAINSCREEN_H
#define EDITORMAINSCREEN_H

#include "enum.h"
#include "EditorWidgetBase.h"
#include <QWidget>
#include <map>
#include <memory>

BETTER_ENUM(EEditorWidget, qint32,
            eResourceWidget = 0,
            eResourceDisplay = 1);

class CDatabaseManager;
class CEditorResourceDisplayWidget;
class CEditorResourceWidget;
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

protected slots:
  void on_pLeftComboBox_currentIndexChanged(qint32 iIndex);
  void on_pRightComboBox_currentIndexChanged(qint32 iIndex);

private:
  template<class T> T* GetWidget();

  std::unique_ptr<Ui::CEditorMainScreen>                      m_spUi;
  std::map<EEditorWidget, std::unique_ptr<CEditorWidgetBase>> m_spWidgetsMap;
  tspProject                                                  m_spCurrentProject;
  std::weak_ptr<CDatabaseManager>                             m_wpDbManager;
  bool                                                        m_bInitialized;
};

template<> CEditorResourceWidget* CEditorMainScreen::GetWidget<CEditorResourceWidget>();
template<> CEditorResourceDisplayWidget* CEditorMainScreen::GetWidget<CEditorResourceDisplayWidget>();

#endif // EDITORMAINSCREEN_H
