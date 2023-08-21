#include "EditorModel.h"
#include "Application.h"
#include "EditorJobWorker.h"

#include "EditorJobs/IEditorJobStateListener.h"

#include "NodeEditor/EndNodeModel.h"
#include "NodeEditor/FlowScene.h"
#include "NodeEditor/PathMergerModel.h"
#include "NodeEditor/PathSplitterModel.h"
#include "NodeEditor/NodeEditorRegistry.h"
#include "NodeEditor/SceneNodeModel.h"
#include "NodeEditor/SceneTranstitionData.h"
#include "NodeEditor/StartNodeModel.h"

#include "Project/KinkTreeModel.h"
#include "Resources/ResourceTreeItemModel.h"
#include "Script/ScriptEditorModel.h"
#include "Systems/DatabaseManager.h"
#include "Systems/PhysFs/PhysFsFileEngine.h"
#include "Systems/Project.h"
#include "Tutorial/ITutorialStateSwitchHandler.h"

#include <nodes/ConnectionStyle>
#include <nodes/FlowScene>
#include <nodes/Node>
#include <nodes/NodeData>
#include <nodes/NodeDataModel>

#include <QDebug>
#include <QFileDialog>
#include <QTextStream>
#include <QUndoStack>
#include <QWidget>

using QtNodes::Node;

//----------------------------------------------------------------------------------------
//
CEditorModel::CEditorModel(QWidget* pParent) :
  QObject(nullptr),
  m_spKinkTreeModel(std::make_unique<CKinkTreeModel>()),
  m_spScriptEditorModel(std::make_unique<CScriptEditorModel>(pParent)),
  m_spFlowSceneModel(std::make_unique<CFlowScene>(CNodeEditorRegistry::RegisterDataModels(), nullptr)),
  m_spUndoStack(std::make_unique<QUndoStack>()),
  m_spResourceTreeModel(std::make_unique<CResourceTreeItemModel>(m_spUndoStack.get())),
  m_spSettings(CApplication::Instance()->Settings()),
  m_spJobWorkerSystem(std::make_shared<CThreadedSystem>("EditorJobWorker")),
  m_spCurrentProject(nullptr),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>()),
  m_vwpTutorialStateSwitchHandlers(),
  m_pParentWidget(pParent),
  m_sScriptTypesFilter(".*"),
  m_bInitializingNewProject(false),
  m_bReadOnly(false)
{
  m_spJobWorkerSystem->RegisterObject<CEditorJobWorker>();

  connect(m_spResourceTreeModel.get(), &CResourceTreeItemModel::SignalProjectEdited,
          this, &CEditorModel::SignalProjectEdited, Qt::DirectConnection);
  connect(m_spScriptEditorModel.get(), &CScriptEditorModel::SignalProjectEdited,
          this, &CEditorModel::SignalProjectEdited, Qt::DirectConnection);
  connect(m_spFlowSceneModel.get(), &CFlowScene::nodeCreated,
          this, &CEditorModel::SlotNodeCreated);
  connect(m_spFlowSceneModel.get(), &CFlowScene::nodeDeleted,
          this, &CEditorModel::SlotNodeDeleted);

  connect(JobWorker(), &CEditorJobWorker::SignalEditorJobStarted,
          this, &CEditorModel::SlotJobStarted);
  connect(JobWorker(), &CEditorJobWorker::SignalEditorJobFinished,
          this, &CEditorModel::SlotJobFinished);
  connect(JobWorker(), &CEditorJobWorker::SignalEditorJobMessage,
          this, &CEditorModel::SlotJobMessage);
  connect(JobWorker(), &CEditorJobWorker::SignalEditorJobProgress,
          this, &CEditorModel::SlotJobProgressChanged);
}

CEditorModel::~CEditorModel()
{
}

//----------------------------------------------------------------------------------------
//
const tspProject& CEditorModel::CurrentProject() const
{
  return m_spCurrentProject;
}

//----------------------------------------------------------------------------------------
//
CFlowScene* CEditorModel::FlowSceneModel() const
{
  return m_spFlowSceneModel.get();
}

//----------------------------------------------------------------------------------------
//
bool CEditorModel::IsReadOnly() const
{
  return m_bReadOnly;
}

//----------------------------------------------------------------------------------------
//
CEditorJobWorker* CEditorModel::JobWorker() const
{
  return dynamic_cast<CEditorJobWorker*>(m_spJobWorkerSystem->Get().get());
}

//----------------------------------------------------------------------------------------
//
CKinkTreeModel* CEditorModel::KinkTreeModel() const
{
  return m_spKinkTreeModel.get();
}

//----------------------------------------------------------------------------------------
//
CResourceTreeItemModel* CEditorModel::ResourceTreeModel() const
{
  return m_spResourceTreeModel.get();
}

//----------------------------------------------------------------------------------------
//
CScriptEditorModel* CEditorModel::ScriptEditorModel() const
{
  return m_spScriptEditorModel.get();
}

//----------------------------------------------------------------------------------------
//
QString CEditorModel::ScriptTypeFilterForNewScripts() const
{
  return m_sScriptTypesFilter;
}

//----------------------------------------------------------------------------------------
//
QUndoStack* CEditorModel::UndoStack() const
{
  return m_spUndoStack.get();
}

//----------------------------------------------------------------------------------------
//
void CEditorModel::AddNewScriptFileToScene(QPointer<QWidget> pParentForDialog,
                                           tspScene spScene)
{
  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spScene && nullptr != spDbManager)
  {
    // if there is no script -> create
    QReadLocker locker(&spScene->m_rwLock);
    if (spScene->m_sScript.isNull() || spScene->m_sScript.isEmpty())
    {
      QRegExp rxTypeFilter(m_sScriptTypesFilter);
      QStringList vsScriptTypes = SResourceFormats::ScriptFormats();
      for (qint32 i = 0; vsScriptTypes.size() > i; ++i)
      {
        if (!rxTypeFilter.exactMatch(vsScriptTypes[i]))
        {
          vsScriptTypes.erase(vsScriptTypes.begin()+i);
          --i;
        }
      }

      const QString sProjectPath = PhysicalProjectPath(m_spCurrentProject);
      QDir projectDir(sProjectPath);
      QPointer<CEditorModel> pThisGuard(this);
      QFileDialog* dlg = new QFileDialog(pParentForDialog,
                                         tr("Create Script File for %1").arg(spScene->m_sName));
      dlg->setViewMode(QFileDialog::Detail);
      dlg->setFileMode(QFileDialog::AnyFile);
      dlg->setAcceptMode(QFileDialog::AcceptSave);
      dlg->setOptions(QFileDialog::DontUseCustomDirectoryIcons);
      dlg->setDirectoryUrl(projectDir.absolutePath());
      dlg->setFilter(QDir::AllDirs);
      dlg->setNameFilter(QString("Script Files (%1)").arg(vsScriptTypes.join(" ")));
      dlg->setDefaultSuffix(SResourceFormats::ScriptFormats().first());

      if (dlg->exec())
      {
        if (nullptr == pThisGuard) { return; }
        QList<QUrl> urls = dlg->selectedUrls();
        delete dlg;

        QUrl url = 1 == urls.size() ? urls[0] : QUrl();
        if (url.isValid())
        {
          QFileInfo info(url.toLocalFile());
          if (!info.absoluteFilePath().contains(projectDir.absolutePath()))
          {
            qWarning() << "File is not in subfolder of Project.";
          }
          else
          {
            QString sRelativePath = projectDir.relativeFilePath(info.absoluteFilePath());
            QUrl sUrlToSave = ResourceUrlFromLocalFile(sRelativePath);
            QFile scriptFile(info.absoluteFilePath());
            if (scriptFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
            {
              m_spScriptEditorModel->InitScript(scriptFile, info.suffix());

              tvfnActionsResource vfnActions = {[&spScene](const tspResource& spNewResource){
                QWriteLocker locker(&spNewResource->m_rwLock);
                spScene->m_sScript = spNewResource->m_sName;
              }};

              QString sResource = spDbManager->AddResource(m_spCurrentProject, sUrlToSave,
                                                           EResourceType::eScript, QString(),
                                                           vfnActions);
            }
            else
            {
              qWarning() << "Could not write script file.";
            }
          }
        }
      }
      else
      {
        if (nullptr == pThisGuard) { return; }
        delete dlg;
      }
    }
  }
}


//----------------------------------------------------------------------------------------
//
void CEditorModel::AddTutorialStateSwitchHandler(std::weak_ptr<ITutorialStateSwitchHandler> wpSwitcher)
{
  m_vwpTutorialStateSwitchHandlers.push_back(wpSwitcher);
}


//----------------------------------------------------------------------------------------
//
void CEditorModel::NextTutorialState()
{
  if (nullptr == m_spCurrentProject) { return; }

  QWriteLocker locker(&m_spCurrentProject->m_rwLock);

  if (ETutorialState::eFinished == m_spCurrentProject->m_tutorialState._to_integral()) { return; }

  ETutorialState oldState = m_spCurrentProject->m_tutorialState;
  size_t iIndex = m_spCurrentProject->m_tutorialState._to_index();
  ETutorialState newState = ETutorialState::_from_index(++iIndex);

  m_spCurrentProject->m_tutorialState = newState;

  locker.unlock();

  for (auto& wpSwitcher : m_vwpTutorialStateSwitchHandlers)
  {
    if (auto spSwitcher = wpSwitcher.lock())
    {
      spSwitcher->OnStateSwitch(newState, oldState);
    }
  }
}


//----------------------------------------------------------------------------------------
//
void CEditorModel::NextResetTutorialState()
{
  for (auto& wpSwitcher : m_vwpTutorialStateSwitchHandlers)
  {
    if (auto spSwitcher = wpSwitcher.lock())
    {
      spSwitcher->OnResetStates();
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorModel::AddEditorJobStateListener(const QString& sJobType,
                                             IEditorJobStateListener* pListener)
{
  if (nullptr != pListener)
  {
    m_vpEditorJobStateListeners[sJobType].push_back(pListener);
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorModel::InitNewProject(const QString& sNewProjectName, bool bTutorial)
{
  m_bInitializingNewProject = true;

  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
#if defined(Q_OS_ANDROID)
    // TODO:
    const QString sBaseProjectPath = m_spSettings->ContentFolder();
#else
    const QString sBaseProjectPath = m_spSettings->ContentFolder();
#endif

    spDbManager->AddProject(QDir(sBaseProjectPath + "/" + sNewProjectName));
    m_spCurrentProject = spDbManager->FindProject(sNewProjectName);
    const QString sProjName = PhysicalProjectName(m_spCurrentProject);

    if (nullptr != m_spCurrentProject)
    {
      m_spCurrentProject->m_rwLock.lockForRead();
      qint32 iId = m_spCurrentProject->m_iId;
      if (bTutorial)
      {
        m_spCurrentProject->m_tutorialState = ETutorialState::eUnstarted;
      }
      m_spCurrentProject->m_rwLock.unlock();

      // create project Folder
      spDbManager->PrepareNewProject(iId);
      // load project
      LoadProject(iId);
      // serialize pre-created data (project file only so far) also tries to create project
      // if it failed before
      spDbManager->SerializeProject(iId);
      // set write dir to allow project to save files
      CPhysFsFileEngine::setWriteDir(
            QString(sBaseProjectPath + "/" + sProjName).toStdString().data());
      // serialize base project again after init (this time including all editor data)
      SerializeProject();
      // save changes (flow, etc)...
      SaveProject();
      // ...and serialize again to write the changes
      SerializeProject();
    }
  }
  m_bInitializingNewProject = false;
}

//----------------------------------------------------------------------------------------
//
void CEditorModel::LoadProject(qint32 iId)
{
  if (nullptr != m_spCurrentProject && !m_bInitializingNewProject)
  {
    assert(false);
    qWarning() << "Old Project was not unloaded before loading project.";
  }

  // wait for jobs
  JobWorker()->StopRunningJobs();
  JobWorker()->WaitForFinished();

  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    m_spCurrentProject = spDbManager->FindProject(iId);
    if (nullptr == m_spCurrentProject)
    {
      qWarning() << QString("Project id %1 not found.").arg(iId);
      return;
    }

    const QString sProjName = PhysicalProjectName(m_spCurrentProject);
    CDatabaseManager::LoadProject(m_spCurrentProject);

    QReadLocker projectLocker(&m_spCurrentProject->m_rwLock);
    m_bReadOnly = m_spCurrentProject->m_bBundled | m_spCurrentProject->m_bReadOnly;
    ETutorialState tutorialState = m_spCurrentProject->m_tutorialState;

    if (!m_bReadOnly)
    {
      CDatabaseManager::SetProjectEditing(m_spCurrentProject, true);
    }

    // nodes
    if (!m_spCurrentProject->m_sSceneModel.isNull() &&
        !m_spCurrentProject->m_sSceneModel.isEmpty())
    {
      const QString sModelName = m_spCurrentProject->m_sSceneModel;
      projectLocker.unlock();

      auto spResource = spDbManager->FindResourceInProject(m_spCurrentProject, sModelName);
      if (nullptr != spResource)
      {
        QString sPath = ResourceUrlToAbsolutePath(spResource);
        QReadLocker resourceLocker(&spResource->m_rwLock);
        QString sBundle = spResource->m_sResourceBundle;
        resourceLocker.unlock();

        CDatabaseManager::LoadBundle(m_spCurrentProject, sBundle);

        QFile modelFile(sPath);
        if (modelFile.open(QIODevice::ReadOnly))
        {
          QByteArray arr = modelFile.readAll();
          m_spFlowSceneModel->loadFromMemory(arr);
        }
        else
        {
          qWarning() << tr("Could not open scene model file.");
        }
      }
    }
    else
    {
      projectLocker.unlock();
    }

    if (tutorialState._to_integral() == ETutorialState::eUnstarted)
    {
      NextTutorialState();
    }
    else if (ETutorialState::eFinished == m_spCurrentProject->m_tutorialState._to_integral());
    else
    {
      for (auto& wpSwitcher : m_vwpTutorialStateSwitchHandlers)
      {
        if (auto spSwitcher = wpSwitcher.lock())
        {
          spSwitcher->OnStateSwitch(tutorialState, ETutorialState::eUnstarted);
        }
      }
    }
  }

  m_spResourceTreeModel->InitializeModel(m_spCurrentProject);
  m_spScriptEditorModel->InitializeModel(m_spCurrentProject);
}

//----------------------------------------------------------------------------------------
//
QString CEditorModel::RenameProject(const QString& sNewProjectName)
{
  if (nullptr == m_spCurrentProject)
  {
    qWarning() << "Trying to rename null-project.";
    return QString();
  }

  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    m_spCurrentProject->m_rwLock.lockForRead();
    qint32 iId = m_spCurrentProject->m_iId;
    m_spCurrentProject->m_rwLock.unlock();

    QString sError;
    if (ProjectNameCheck(sNewProjectName, &sError))
    {
      spDbManager->RenameProject(iId, sNewProjectName);
    }
    else
    {
      qWarning() << sError;
    }

    m_spCurrentProject->m_rwLock.lockForRead();
    const QString sName = m_spCurrentProject->m_sName;
    m_spCurrentProject->m_rwLock.unlock();
    return sName;
  }
  return QString();
}

//----------------------------------------------------------------------------------------
//
void CEditorModel::SaveProject()
{
  if (nullptr == m_spCurrentProject && !m_bInitializingNewProject)
  {
    qWarning() << "Trying to save null-project.";
    return;
  }

  // wait for jobs
  JobWorker()->StopRunningJobs();
  JobWorker()->WaitForFinished();

  // nodes
  QByteArray arr = m_spFlowSceneModel->saveToMemory();

  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    QReadLocker projectLocker(&m_spCurrentProject->m_rwLock);
    if (m_spCurrentProject->m_sSceneModel.isNull() ||
        m_spCurrentProject->m_sSceneModel.isEmpty())
    {
      projectLocker.unlock();
      spDbManager->AddResource(m_spCurrentProject, ResourceUrlFromLocalFile(joip_resource::c_sSceneModelFile),
                               EResourceType::eOther, joip_resource::c_sSceneModelFile);
      projectLocker.relock();
      m_spCurrentProject->m_sSceneModel = joip_resource::c_sSceneModelFile;
    }

    auto spResource = spDbManager->FindResourceInProject(m_spCurrentProject, joip_resource::c_sSceneModelFile);
    if (nullptr != spResource)
    {
      QString sPath = ResourceUrlToAbsolutePath(spResource);
      QReadLocker resourceLocker(&spResource->m_rwLock);
      resourceLocker.unlock();

      QFile modelFile(sPath);
      if (modelFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
      {
        modelFile.write(arr);
      }
      else
      {
        qWarning() << tr("Could not save scene model file.");
      }
    }

    // check if flags for media or remote are needed
    bool bHasRemoteResources = false;
    bool bNeedsMedia = false;
    for (const auto& it : m_spCurrentProject->m_spResourcesMap)
    {
      QReadLocker resourceLocker(&it.second->m_rwLock);
      bHasRemoteResources |= !IsLocalFile(it.second->m_sPath);
      bNeedsMedia |= (it.second->m_type._to_integral() == EResourceType::eMovie ||
                      it.second->m_type._to_integral() == EResourceType::eSound);
    }
    m_spCurrentProject->m_bUsesWeb = bHasRemoteResources;
    m_spCurrentProject->m_bNeedsCodecs = bNeedsMedia;
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorModel::UnloadProject()
{
  // wait for jobs
  JobWorker()->StopRunningJobs();
  JobWorker()->WaitForFinished();

  m_spScriptEditorModel->DeInitializeModel();
  m_spResourceTreeModel->DeInitializeModel();
  m_spFlowSceneModel->clearScene();
  m_spUndoStack->clear();

  // reset to what is in the database
  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager && nullptr != m_spCurrentProject)
  {
    m_spCurrentProject->m_rwLock.lockForRead();
    qint32 iId = m_spCurrentProject->m_iId;
    m_spCurrentProject->m_rwLock.unlock();

    spDbManager->DeserializeProject(iId);
  }

  CDatabaseManager::SetProjectEditing(m_spCurrentProject, false);
  CDatabaseManager::UnloadProject(m_spCurrentProject);
  m_spCurrentProject = nullptr;

  NextResetTutorialState();

  m_bReadOnly = false;
}

//----------------------------------------------------------------------------------------
//
void CEditorModel::SerializeProject()
{
  if (nullptr == m_spCurrentProject && !m_bInitializingNewProject)
  {
    qWarning() << "Trying to serialize null-project.";
    return;
  }

  m_spScriptEditorModel->SerializeProject();

  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    m_spCurrentProject->m_rwLock.lockForRead();
    qint32 iId = m_spCurrentProject->m_iId;
    m_spCurrentProject->m_rwLock.unlock();

    // save to create folder structure
    spDbManager->SerializeProject(iId);
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorModel::ExportProject(CEditorExportJob::EExportFormat format)
{
  if (nullptr == m_spCurrentProject)
  {
    return;
  }

  QVariantList args;
  args << JobWorker()->GenerateNewId();
  {
    QReadLocker locker(&m_spCurrentProject->m_rwLock);
    args << m_spCurrentProject->m_sName;
  }
  args << static_cast<qint32>(format);
  JobWorker()->CreateNewEditorJob<CEditorExportJob>(args);
}

//----------------------------------------------------------------------------------------
//
void CEditorModel::SetScriptTypeFilterForNewScripts(const QString& sFilter)
{
  if (!sFilter.isEmpty())
  {
    m_sScriptTypesFilter = sFilter;
  }
  else
  {
    m_sScriptTypesFilter = ".*";
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorModel::SlotNodeCreated(Node &n)
{
  if (nullptr == m_spCurrentProject)
  {
    qWarning() << "Node created in null-project.";
    return;
  }

  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    CSceneNodeModel* pSceneModel = dynamic_cast<CSceneNodeModel*>(n.nodeDataModel());
    if (nullptr != pSceneModel)
    {
      m_spCurrentProject->m_rwLock.lockForRead();
      qint32 iId = m_spCurrentProject->m_iId;
      m_spCurrentProject->m_rwLock.unlock();
      pSceneModel->SetProjectId(iId);

      qint32 iSceneId = pSceneModel->SceneId();
      auto spScene = spDbManager->FindScene(m_spCurrentProject, iSceneId);
      AddNewScriptFileToScene(m_pParentWidget, spScene);

      connect(pSceneModel, &CSceneNodeModel::SignalAddScriptFileRequested,
              this, &CEditorModel::SlotAddNewScriptFileToScene, Qt::UniqueConnection);
    }
    emit SignalProjectEdited();
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorModel::SlotNodeDeleted(QtNodes::Node &n)
{
  if (nullptr == m_spCurrentProject)
  {
    qWarning() << "Node created in null-project.";
    return;
  }

  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    CSceneNodeModel* pSceneModel = dynamic_cast<CSceneNodeModel*>(n.nodeDataModel());
    if (nullptr != pSceneModel)
    {
      qint32 iSceneId = pSceneModel->SceneId();
      spDbManager->RemoveScene(m_spCurrentProject, iSceneId);
    }
    emit SignalProjectEdited();
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorModel::SlotAddNewScriptFileToScene()
{
  if (nullptr == m_spCurrentProject)
  {
    qWarning() << "Node created in null-project.";
    return;
  }

  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    CSceneNodeModel* pSceneModel = dynamic_cast<CSceneNodeModel*>(sender());
    if (nullptr != pSceneModel)
    {
      m_spCurrentProject->m_rwLock.lockForRead();
      qint32 iId = m_spCurrentProject->m_iId;
      m_spCurrentProject->m_rwLock.unlock();
      pSceneModel->SetProjectId(iId);

      qint32 iSceneId = pSceneModel->SceneId();
      auto spScene = spDbManager->FindScene(m_spCurrentProject, iSceneId);
      AddNewScriptFileToScene(m_pParentWidget, spScene);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorModel::SlotJobFinished(qint32 iId, QString type)
{
  auto it = m_vpEditorJobStateListeners.find(type);
  if (m_vpEditorJobStateListeners.end() != it)
  {
    for (IEditorJobStateListener* pListener : it->second)
    {
      pListener->JobFinished(iId);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorModel::SlotJobStarted(qint32 iId, QString type)
{
  auto it = m_vpEditorJobStateListeners.find(type);
  if (m_vpEditorJobStateListeners.end() != it)
  {
    for (IEditorJobStateListener* pListener : it->second)
    {
      pListener->JobStarted(iId);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorModel::SlotJobMessage(qint32 iId, QString type, QString sMsg)
{
  auto it = m_vpEditorJobStateListeners.find(type);
  if (m_vpEditorJobStateListeners.end() != it)
  {
    for (IEditorJobStateListener* pListener : it->second)
    {
      pListener->JobMessage(iId, sMsg);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorModel::SlotJobProgressChanged(qint32 iId, QString type, qint32 iProgress)
{
  auto it = m_vpEditorJobStateListeners.find(type);
  if (m_vpEditorJobStateListeners.end() != it)
  {
    for (IEditorJobStateListener* pListener : it->second)
    {
      pListener->JobProgressChanged(iId, iProgress);
    }
  }
}
