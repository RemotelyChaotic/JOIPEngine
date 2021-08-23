#ifndef EDITORMODEL_H
#define EDITORMODEL_H

#include <QObject>
#include <QPointer>
#include <QProcess>
#include <memory>

class CDatabaseManager;
class CKinkTreeModel;
class CResourceTreeItemModel;
class CScriptEditorModel;
class CSettings;
class ITutorialStateSwitchHandler;
namespace QtNodes {
  class FlowScene;
  class Node;
}
class QUndoStack;
struct SScene;
struct SProject;
typedef std::shared_ptr<SProject> tspProject;
typedef std::shared_ptr<SScene> tspScene;

class CEditorModel : public QObject
{
  Q_OBJECT
public:
  explicit CEditorModel(QWidget* pParent = nullptr);
  ~CEditorModel();

  enum class EExportError : qint32 {
    eWriteFailed,
    eCleanupFailed,
    eProcessError
  };
  Q_ENUM(EExportError)

  const tspProject& CurrentProject() const;
  QtNodes::FlowScene* FlowSceneModel() const;
  bool IsReadOnly() const;
  CKinkTreeModel* KinkTreeModel() const;
  CResourceTreeItemModel* ResourceTreeModel() const;
  CScriptEditorModel* ScriptEditorModel() const;
  QUndoStack* UndoStack() const;

  void AddFilesToProjectResources(QPointer<QWidget> pParentForDialog,
                                  const QStringList& vsFiles);
  void AddNewScriptFileToScene(QPointer<QWidget> pParentForDialog,
                               tspScene spScene);

  void AddTutorialStateSwitchHandler(std::weak_ptr<ITutorialStateSwitchHandler> wpSwitcher);
  void NextTutorialState();
  void NextResetTutorialState();

  void InitNewProject(const QString& sNewProjectName, bool bTutorial);
  void LoadProject(qint32 iId);
  QString RenameProject(const QString& sNewProjectName);
  void SaveProject();
  void UnloadProject();
  void SerializeProject();
  void ExportProject();

public slots:
  void SlotNodeCreated(QtNodes::Node &n);
  void SlotNodeDeleted(QtNodes::Node &n);

signals:
  void SignalProjectEdited();
  void SignalProjectExportStarted();
  void SignalProjectExportError(EExportError error, const QString& sErrorString);
  void SignalProjectExportFinished();

private slots:
  void SlotAddNewScriptFileToScene();
  void SlotExportErrorOccurred(QProcess::ProcessError error);
  void SlotExportFinished(int exitCode, QProcess::ExitStatus exitStatus);
  void SlotExportStarted();
  void SlotExportStateChanged(QProcess::ProcessState newState);


private:
  std::unique_ptr<CKinkTreeModel>                             m_spKinkTreeModel;
  std::unique_ptr<CResourceTreeItemModel>                     m_spResourceTreeModel;
  std::unique_ptr<CScriptEditorModel>                         m_spScriptEditorModel;
  std::unique_ptr<QtNodes::FlowScene>                         m_spFlowSceneModel;
  std::unique_ptr<QUndoStack>                                 m_spUndoStack;
  std::unique_ptr<QProcess>                                   m_spExportProcess;
  std::shared_ptr<CSettings>                                  m_spSettings;
  tspProject                                                  m_spCurrentProject;
  std::weak_ptr<CDatabaseManager>                             m_wpDbManager;
  std::vector<std::weak_ptr<ITutorialStateSwitchHandler>>     m_vwpTutorialStateSwitchHandlers;
  QPointer<QWidget>                                           m_pParentWidget;
  bool                                                        m_bInitializingNewProject;
  bool                                                        m_bReadOnly;
};

#endif // EDITORMODEL_H
