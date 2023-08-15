#ifndef EDITORMODEL_H
#define EDITORMODEL_H

#include <QObject>
#include <QPointer>
#include <QProcess>
#include <memory>

class CDatabaseManager;
class CFlowScene;
class CEditorJobWorker;
class CKinkTreeModel;
class CResourceTreeItemModel;
class CScriptEditorModel;
class CSettings;
class CThreadedSystem;
class ITutorialStateSwitchHandler;
namespace QtNodes {
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
  CFlowScene* FlowSceneModel() const;
  bool IsReadOnly() const;
  CEditorJobWorker* JobWorker() const;
  CKinkTreeModel* KinkTreeModel() const;
  CResourceTreeItemModel* ResourceTreeModel() const;
  CScriptEditorModel* ScriptEditorModel() const;
  QString ScriptTypeFilterForNewScripts() const;
  QUndoStack* UndoStack() const;

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
  void SetScriptTypeFilterForNewScripts(const QString& sFilter);

public slots:
  void SlotNodeCreated(QtNodes::Node &n);
  void SlotNodeDeleted(QtNodes::Node &n);

signals:
  void SignalJobMessage(qint32 iId, QString sMsg);
  void SignalJobFinished(qint32 iId);
  void SignalJobStarted(qint32 iId);
  void SignalJobProgressChanged(qint32 iId, qint32 iProgress);
  void SignalProjectEdited();

private slots:
  void SlotAddNewScriptFileToScene();

private:
  std::unique_ptr<CKinkTreeModel>                             m_spKinkTreeModel;
  std::unique_ptr<CScriptEditorModel>                         m_spScriptEditorModel;
  std::unique_ptr<CFlowScene>                                 m_spFlowSceneModel;
  std::unique_ptr<QUndoStack>                                 m_spUndoStack;
  std::unique_ptr<CResourceTreeItemModel>                     m_spResourceTreeModel;
  std::shared_ptr<CSettings>                                  m_spSettings;
  std::shared_ptr<CThreadedSystem>                            m_spJobWorkerSystem;
  tspProject                                                  m_spCurrentProject;
  std::weak_ptr<CDatabaseManager>                             m_wpDbManager;
  std::vector<std::weak_ptr<ITutorialStateSwitchHandler>>     m_vwpTutorialStateSwitchHandlers;
  QPointer<QWidget>                                           m_pParentWidget;
  QString                                                     m_sScriptTypesFilter;
  bool                                                        m_bInitializingNewProject;
  bool                                                        m_bReadOnly;
};

#endif // EDITORMODEL_H
