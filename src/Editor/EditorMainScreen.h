#ifndef EDITORMAINSCREEN_H
#define EDITORMAINSCREEN_H

#include "EditorWidgetBase.h"
#include "EditorModel.h"
#include "EditorWidgetTypes.h"
#include <QWidget>
#include <map>
#include <memory>

class CDatabaseManager;
class CEditorCodeWidget;
class CEditorProjectSettingsWidget;
class CEditorResourceDisplayWidget;
class CEditorResourceWidget;
class CEditorSceneNodeWidget;
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
  void SlotExportClicked(bool bClick);
  void SlotHelpClicked(bool bClick);
  void SlotKeyBindingsChanged();
  void SlotProjectEdited();
  void SlotProjectExportStarted();
  void SlotProjectExportError(CEditorModel::EExportError error, const QString& sErrorString);
  void SlotProjectExportFinished();
  void SlotProjectNameEditingFinished();
  void SlotProjectRenamed(qint32 iId);
  void SlotSaveClicked(bool bClick);

private:
  void ChangeIndex(QComboBox* pComboBox, QWidget* pContainer,
                   CEditorActionBar* pActionBar, qint32 iIndex);
  void ProjectLoaded(bool bNewProject);
  void SetModificaitonFlag(bool bModified);
  template<class T> T* GetWidget();

  std::unique_ptr<Ui::CEditorMainScreen>                      m_spUi;
  std::unique_ptr<CEditorModel>                               m_spEditorModel;
  std::vector<QPointer<QAction>>                              m_vpKeyBindingActions;
  std::map<EEditorWidget, QPointer<CEditorWidgetBase>>        m_spWidgetsMap;
  tspProject                                                  m_spCurrentProject;
  std::weak_ptr<CDatabaseManager>                             m_wpDbManager;
  bool                                                        m_bInitialized;
  bool                                                        m_bProjectModified;
  qint32                                                      m_iLastLeftIndex;
  qint32                                                      m_iLastRightIndex;
};

template<> CEditorResourceWidget* CEditorMainScreen::GetWidget<CEditorResourceWidget>();
template<> CEditorResourceDisplayWidget* CEditorMainScreen::GetWidget<CEditorResourceDisplayWidget>();
template<> CEditorProjectSettingsWidget* CEditorMainScreen::GetWidget<CEditorProjectSettingsWidget>();
template<> CEditorSceneNodeWidget* CEditorMainScreen::GetWidget<CEditorSceneNodeWidget>();
template<> CEditorCodeWidget* CEditorMainScreen::GetWidget<CEditorCodeWidget>();

#endif // EDITORMAINSCREEN_H
