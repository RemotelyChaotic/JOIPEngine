#ifndef EDITORMAINSCREEN_H
#define EDITORMAINSCREEN_H

#include "enum.h"
#include "EditorWidgetBase.h"
#include <QWidget>
#include <map>
#include <memory>

BETTER_ENUM(EEditorWidget, qint32,
            eResourceWidget = 0,
            eResourceDisplay = 1,
            eSceneNodeWidget = 2,
            eSceneCodeEditorWidget = 3);

class CDatabaseManager;
class CEditorCodeWidget;
class CEditorResourceDisplayWidget;
class CEditorResourceWidget;
class CEditorSceneNodeWidget;
class CResourceTreeItemModel;
namespace Ui {
  class CEditorMainScreen;
}
class QComboBox;
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

signals:
  void SignalExitClicked();

protected slots:
  void on_pLeftComboBox_currentIndexChanged(qint32 iIndex);
  void on_pRightComboBox_currentIndexChanged(qint32 iIndex);
  void SlotDisplayResource(const QString& sName);
  void SlotExitClicked(bool bClick);
  void SlotProjectEdited();
  void SlotProjectNameEditingFinished();
  void SlotSaveClicked(bool bClick);

private:
  void ChangeIndex(QComboBox* pComboBox, QWidget* pContainer,
                   CEditorActionBar* pActionBar, qint32 iIndex);
  void SetModificaitonFlag(bool bModified);
  template<class T> T* GetWidget();

  std::unique_ptr<Ui::CEditorMainScreen>                      m_spUi;
  std::unique_ptr<CResourceTreeItemModel>                     m_spResourceTreeModel;
  std::map<EEditorWidget, QPointer<CEditorWidgetBase>>        m_spWidgetsMap;
  tspProject                                                  m_spCurrentProject;
  std::weak_ptr<CDatabaseManager>                             m_wpDbManager;
  bool                                                        m_bInitialized;
  bool                                                        m_bInitializingNewProject;
  bool                                                        m_bProjectModified;
  qint32                                                      m_iLastLeftIndex;
  qint32                                                      m_iLastRightIndex;
};

template<> CEditorResourceWidget* CEditorMainScreen::GetWidget<CEditorResourceWidget>();
template<> CEditorResourceDisplayWidget* CEditorMainScreen::GetWidget<CEditorResourceDisplayWidget>();
template<> CEditorSceneNodeWidget* CEditorMainScreen::GetWidget<CEditorSceneNodeWidget>();
template<> CEditorCodeWidget* CEditorMainScreen::GetWidget<CEditorCodeWidget>();

#endif // EDITORMAINSCREEN_H
