#ifndef EDITORMAINSCREEN_H
#define EDITORMAINSCREEN_H

#include "EditorModel.h"
#include "EditorWidgetTypes.h"
#include "ui_EditorMainScreen.h" // we need to include this in the header for the tutorial
#include "ui_EditorActionBar.h"  // state switch handler to be able to access the ui
#include "EditorWidgets/EditorWidgetBase.h"
#include <QWidget>
#include <map>
#include <memory>

class CDatabaseManager;
class CEditorLayoutViewProvider;
class CEditorTutorialOverlay;
class CWindowContext;
namespace Ui {
  class CEditorMainScreen;
}
struct SProject;
typedef std::shared_ptr<SProject> tspProject;

class CEditorMainScreen : public QWidget
{
  Q_OBJECT
  friend class CEditorLayoutViewProvider;

public:
  explicit CEditorMainScreen(QWidget* pParent = nullptr);
  ~CEditorMainScreen();

  void Initialize(const std::shared_ptr<CWindowContext>& spWindowContext);
  void InitNewProject(const QString& sNewProjectName, bool bTutorial);
  void LoadProject(qint32 iId);
  void UnloadProject();

signals:
  void SignalExitClicked();
  void SignalUnloadFinished();

protected slots:
  void SlotExitCalled();
  void SlotExitClicked(bool bClick);
  void SlotExportClicked(bool bClick);
  void SlotHelpClicked(bool bClick);
  void SlotProjectEdited();
  void SlotProjectExportStarted();
  void SlotProjectExportError(CEditorModel::EExportError error, const QString& sErrorString);
  void SlotProjectExportFinished();
  void SlotProjectNameEditingFinished();
  void SlotProjectRenamed(qint32 iId);
  void SlotSaveClicked(bool bClick);
  void SlotUnloadFinished();

private:
  void CreateLayout();
  void ProjectLoaded(bool bNewProject);
  void RemoveLayout();
  void SetModificaitonFlag(bool bModified);

  std::unique_ptr<CEditorModel>                               m_spEditorModel;
  std::shared_ptr<CEditorLayoutViewProvider>                  m_spViewProvider;
  std::shared_ptr<Ui::CEditorMainScreen>                      m_spUi;
  std::shared_ptr<CWindowContext>                             m_spWindowContext;
  std::vector<QPointer<QAction>>                              m_vpKeyBindingActions;
  QPointer<CEditorLayoutBase>                                 m_pLayout;
  QPointer<CEditorTutorialOverlay>                            m_pTutorialOverlay;
  std::map<EEditorWidget, QPointer<CEditorWidgetBase>>        m_spWidgetsMap;
  tspProject                                                  m_spCurrentProject;
  std::weak_ptr<CDatabaseManager>                             m_wpDbManager;
  bool                                                        m_bInitialized;
  bool                                                        m_bProjectModified;
};

#endif // EDITORMAINSCREEN_H
