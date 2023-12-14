#ifndef EDITORMODEL_H
#define EDITORMODEL_H

#include "EditorJobs/EditorExportJob.h"

#include "Systems/DatabaseInterface/ResourceData.h"

#include <QObject>
#include <QPointer>
#include <QProcess>
#include <QString>
#include <map>
#include <memory>
#include <vector>

class CDatabaseManager;
class CFlowScene;
class CEditorJobWorker;
class CKinkTreeModel;
class CResourceTreeItemModel;
class CEditorEditableFileModel;
class CSettings;
class CThreadedSystem;
class IEditorJobStateListener;
class IEditorToolBox;
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

  const tspProject& CurrentProject() const;
  CFlowScene* FlowSceneModel() const;
  bool IsReadOnly() const;
  CEditorJobWorker* JobWorker() const;
  CKinkTreeModel* KinkTreeModel() const;
  CResourceTreeItemModel* ResourceTreeModel() const;
  CEditorEditableFileModel* EditableFileModel() const;
  QString ScriptTypeFilterForNewScripts() const;
  QUndoStack* UndoStack() const;

  void AddNewFileToScene(QPointer<QWidget> pParentForDialog,
                         tspScene spScene,
                         EResourceType type);

  void AddTutorialStateSwitchHandler(std::weak_ptr<ITutorialStateSwitchHandler> wpSwitcher);
  void NextTutorialState();
  void NextResetTutorialState();

  void AddEditorJobStateListener(const QString& sJobType, IEditorJobStateListener* pListener);

  void AddEditorToolbox(const QString& sTool, IEditorToolBox* pTool);
  const std::map<QString, IEditorToolBox*>& EditorToolboxes() const;

  void InitNewProject(const QString& sNewProjectName, bool bTutorial);
  void LoadProject(qint32 iId);
  QString RenameProject(const QString& sNewProjectName);
  void SaveProject();
  void UnloadProject();
  void SerializeProject();
  void ExportProject(CEditorExportJob::EExportFormat format);
  void SetScriptTypeFilterForNewScripts(const QString& sFilter);

public slots:
  void SlotNodeCreated(QtNodes::Node &n);
  void SlotNodeDeleted(QtNodes::Node &n);

signals:
  void SignalProjectEdited();
  void SignalProjectPropertiesEdited();

private slots:
  void SlotAddNewScriptFileToScene();
  void SlotAddNewLayoutFileToScene();
  void SlotJobFinished(qint32 iId, QString type);
  void SlotJobStarted(qint32 iId, QString type);
  void SlotJobMessage(qint32 iId, QString type, QString sMsg);
  void SlotJobProgressChanged(qint32 iId, QString type, qint32 iProgress);

private:
  std::unique_ptr<CKinkTreeModel>                             m_spKinkTreeModel;
  std::unique_ptr<CEditorEditableFileModel>                   m_spEditableFileModel;
  std::unique_ptr<CFlowScene>                                 m_spFlowSceneModel;
  std::unique_ptr<QUndoStack>                                 m_spUndoStack;
  std::unique_ptr<CResourceTreeItemModel>                     m_spResourceTreeModel;
  std::shared_ptr<CSettings>                                  m_spSettings;
  std::shared_ptr<CThreadedSystem>                            m_spJobWorkerSystem;
  tspProject                                                  m_spCurrentProject;
  std::weak_ptr<CDatabaseManager>                             m_wpDbManager;
  std::vector<std::weak_ptr<ITutorialStateSwitchHandler>>     m_vwpTutorialStateSwitchHandlers;
  std::map<QString, std::vector<IEditorJobStateListener*>>    m_vpEditorJobStateListeners;
  std::map<QString, IEditorToolBox*>                          m_editorTools;
  QPointer<QWidget>                                           m_pParentWidget;
  QString                                                     m_sScriptTypesFilter;
  bool                                                        m_bInitializingNewProject;
  bool                                                        m_bReadOnly;
};

#endif // EDITORMODEL_H
