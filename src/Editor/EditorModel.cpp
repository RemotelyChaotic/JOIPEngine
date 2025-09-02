#include "EditorModel.h"
#include "Application.h"
#include "EditorJobWorker.h"
#include "EditorEditableFileModel.h"

#include "DialogueEditor/DialogueEditorTreeModel.h"

#include "EditorJobs/IEditorJobStateListener.h"

#include "NodeEditor/EndNodeModel.h"
#include "NodeEditor/FlowScene.h"
#include "NodeEditor/PathMergerModel.h"
#include "NodeEditor/PathSplitterModel.h"
#include "NodeEditor/NodeEditorRegistry.h"
#include "NodeEditor/NodeGraphicsObjectProvider.h"
#include "NodeEditor/SceneNodeModel.h"
#include "NodeEditor/SceneTranstitionData.h"
#include "NodeEditor/StartNodeModel.h"

#include "Script/ScriptCompleterFileProcessors.h"
#include "Script/ScriptEditorCompleterModel.h"

#include "Project/KinkTreeModel.h"
#include "Resources/ResourceTreeItemModel.h"
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
  m_spEditableFileModel(std::make_unique<CEditorEditableFileModel>(pParent)),
  m_spEditorCompleterModel(std::make_unique<CScriptEditorCompleterModel>(pParent)),
  m_spFlowSceneModel(std::make_unique<CFlowScene>(
          CNodeEditorRegistry::RegisterDataModels(),
          std::make_shared<CDefaultGraphicsObjectProvider>(),
          nullptr)),
  m_spUndoStack(std::make_unique<QUndoStack>()),
  m_spResourceTreeModel(std::make_unique<CResourceTreeItemModel>(m_spUndoStack.get())),
  m_spSettings(CApplication::Instance()->Settings()),
  m_spJobWorkerSystem(std::make_shared<CThreadedSystem>("EditorJobWorker")),
  m_spDialogueModel(std::make_shared<CDialogueEditorTreeModel>(pParent)),
  m_spCurrentProject(nullptr),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>()),
  m_vwpTutorialStateSwitchHandlers(),
  m_pParentWidget(pParent),
  m_sScriptTypesFilter(".*"),
  m_bInitializingNewProject(false),
  m_bReadOnly(false)
{
  m_spJobWorkerSystem->RegisterObject<CEditorJobWorker>();

  m_spEditorCompleterModel->RegisterFileProcessor<CScriptCompleterFileProcessorJs>(
      SScriptDefinitionData::c_sScriptTypeJs);
  m_spEditorCompleterModel->RegisterFileProcessor<CScriptCompleterFileProcessorLua>(
      SScriptDefinitionData::c_sScriptTypeLua);
  auto qmlProcessor = std::make_shared<CScriptCompleterFileProcessorQml>();
  m_spEditorCompleterModel->RegisterFileProcessor(SScriptDefinitionData::c_sScriptTypeQml, qmlProcessor);
  m_spEditorCompleterModel->RegisterFileProcessor(SScriptDefinitionData::c_sScriptTypeLayout, qmlProcessor);

  connect(m_spResourceTreeModel.get(), &CResourceTreeItemModel::SignalProjectEdited,
          this, &CEditorModel::SignalProjectEdited, Qt::DirectConnection);
  connect(m_spEditableFileModel.get(), &CEditorEditableFileModel::SignalProjectEdited,
          this, &CEditorModel::SignalProjectEdited, Qt::DirectConnection);
  connect(m_spFlowSceneModel.get(), &CFlowScene::nodeCreated,
          this, &CEditorModel::SlotNodeCreated);
  connect(m_spFlowSceneModel.get(), &CFlowScene::nodeDeleted,
          this, &CEditorModel::SlotNodeDeleted);

  connect(this, &CEditorModel::SignalProjectPropertiesEdited,
          m_spResourceTreeModel.get(), &CResourceTreeItemModel::SlotProjectPropertiesEdited);
  connect(this, &CEditorModel::SignalProjectPropertiesEdited,
          m_spEditableFileModel.get(), &CEditorEditableFileModel::SlotProjectPropertiesEdited);

  connect(JobWorker(), &CEditorJobWorker::SignalEditorJobStarted,
          this, &CEditorModel::SlotJobStarted);
  connect(JobWorker(), &CEditorJobWorker::SignalEditorJobFinished,
          this, &CEditorModel::SlotJobFinished);
  connect(JobWorker(), &CEditorJobWorker::SignalEditorJobMessage,
          this, &CEditorModel::SlotJobMessage);
  connect(JobWorker(), &CEditorJobWorker::SignalEditorJobProgress,
          this, &CEditorModel::SlotJobProgressChanged);

  if (auto spDbManager = m_wpDbManager.lock())
  {
    connect(spDbManager.get(), &CDatabaseManager::SignalSceneDataChanged,
            this, &CEditorModel::SignalProjectEdited, Qt::DirectConnection);
  }
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
CDialogueEditorTreeModel* CEditorModel::DialogueModel() const
{
  return m_spDialogueModel.get();
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
CEditorEditableFileModel* CEditorModel::EditableFileModel() const
{
  return m_spEditableFileModel.get();
}

//----------------------------------------------------------------------------------------
//
CScriptEditorCompleterModel* CEditorModel::EditorCompleterModel() const
{
  return m_spEditorCompleterModel.get();
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
namespace
{
  void UpdateQmldir(const QFileInfo& sAbsoluteFile, const QString& sRelativePath,
                    std::shared_ptr<CDatabaseManager> spDbManager,
                    tspProject spProject)
  {
    const QString sQmlDir = sAbsoluteFile.absolutePath() + "/qmldir";
    QString sImport;
    {
      QString sContent;
      qint32 iPos = sRelativePath.lastIndexOf("/");
      if (-1 != iPos)
      {
        sImport = sRelativePath.left(iPos).replace("/", ".").replace("\\", ".");
      }
    }

    if (sImport.isEmpty()) { return; }

    // if qmldir does not exist, create it
    if (!QFileInfo(sQmlDir).exists())
    {
      QFile qmldirFile(sQmlDir);
      if (qmldirFile.open(QIODevice::ReadWrite))
      {
        QString sContent = QString("module %1").arg(sImport);
        qmldirFile.write(sContent.toUtf8());
      }
      else
      {
        qWarning() << QObject::tr("Could not create qmldir for new Layout file.");
        return;
      }
    }

    // update qmldir file
    QFile qmldirFile(sQmlDir);
    if (qmldirFile.open(QIODevice::ReadWrite | QIODevice::Append))
    {
      QString sNewModule = "\n%1 1.0 %2";
      QString sModule = sNewModule.arg(sAbsoluteFile.baseName())
                                   .arg(sAbsoluteFile.fileName());
      qmldirFile.write(sModule.toUtf8());
    }
    else
    {
      qWarning() << QObject::tr("Could not update qmldir with new Layout file.");
      return;
    }

    // update resources to include the file
    QUrl url = ResourceUrlFromLocalFile(QString(sRelativePath)
                                        .replace(sAbsoluteFile.fileName(), "qmldir"));
    QString sResource = QString("qmldir_%1").arg(sImport);
    tspResource spRes = spDbManager->FindResourceInProject(spProject, sResource);
    if (nullptr == spRes)
    {
      spDbManager->AddResource(spProject, url, EResourceType::eOther, sResource);
    }
  }

  //--------------------------------------------------------------------------------------
  //
  void InitScript(QIODevice& file, const QString& sType, const QString& sCustomInitContent)
  {
    if (sCustomInitContent.isEmpty())
    {
      auto itDefinition = SScriptDefinitionData::DefinitionMap().find(sType);
      if (SScriptDefinitionData::DefinitionMap().end() != itDefinition)
      {
        file.write(itDefinition->second.sInitText.toUtf8());
      }
    }
    else
    {
      file.write(sCustomInitContent.toUtf8());
    }
  }
}

//----------------------------------------------------------------------------------------
//
QString CEditorModel::AddNewFileToScene(QPointer<QWidget> pParentForDialog,
                                        tspScene spScene,
                                        EResourceType type,
                                        const QString& sCustomInitContent,
                                        const QStringList& vsFormats)
{
  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    QStringList formats = vsFormats;
    if (formats.isEmpty())
    {
      switch (type)
      {
        case EResourceType::eScript:
        {
          formats = SResourceFormats::ScriptFormats();
        } break;
        case EResourceType::eLayout:
        {
          formats = SResourceFormats::LayoutFormats();
        } break;
        default:
          qWarning() << tr("No formats specified for editor model dialogue.");
          return QString();
      }
    }

    QRegExp rxTypeFilter(m_sScriptTypesFilter);
    for (qint32 i = 0; formats.size() > i; ++i)
    {
      if (!rxTypeFilter.exactMatch(formats[i]))
      {
        formats.erase(formats.begin()+i);
        --i;
      }
    }

    // if there is no script -> create
    QString sSceneName;
    if (nullptr != spScene)
    {
      QReadLocker locker(&spScene->m_rwLock);
      sSceneName = spScene->m_sName;
    }

    const QString sProjectPath = PhysicalProjectPath(m_spCurrentProject);
    QDir projectDir(sProjectPath);
    QPointer<CEditorModel> pThisGuard(this);
    QString sTitle = sSceneName.isEmpty() ? tr("Create File") :
                                            tr("Create File for %1").arg(sSceneName);
    QFileDialog* dlg = new QFileDialog(pParentForDialog, sTitle);
    dlg->setViewMode(QFileDialog::Detail);
    dlg->setFileMode(QFileDialog::AnyFile);
    dlg->setAcceptMode(QFileDialog::AcceptSave);
    dlg->setOptions(QFileDialog::DontUseCustomDirectoryIcons);
    dlg->setDirectoryUrl(projectDir.absolutePath());
    dlg->setFilter(QDir::AllDirs);
    dlg->setNameFilter(QString("Files (%1)").arg(formats.join(" ")));
    dlg->setDefaultSuffix(formats.first());

    if (dlg->exec())
    {
      if (nullptr == pThisGuard) { return QString(); }
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
            if (EResourceType::eLayout == type._to_integral())
            {
              UpdateQmldir(info, sRelativePath, spDbManager, m_spCurrentProject);
            }
            InitScript(scriptFile, info.suffix(), sCustomInitContent);

            tvfnActionsResource vfnActions =
                {[&spScene, type, spDbManager](const tspResource& spNewResource){
              if (nullptr == spScene) { return; }
              qint32 iProjId = -1;
              spScene->m_spParent->m_rwLock.lockForRead();
              iProjId = spScene->m_spParent->m_iId;
              spScene->m_spParent->m_rwLock.unlock();
              QWriteLocker locker(&spNewResource->m_rwLock);
              if (EResourceType::eScript == type._to_integral())
              {
                spScene->m_sScript = spNewResource->m_sName;
                emit spDbManager->SignalSceneDataChanged(iProjId, spScene->m_iId);
              }
              else if (EResourceType::eLayout == type._to_integral())
              {
                spScene->m_sSceneLayout = spNewResource->m_sName;
                emit spDbManager->SignalSceneDataChanged(iProjId, spScene->m_iId);
              }
            }};

            QString sResource = spDbManager->AddResource(m_spCurrentProject, sUrlToSave,
                                                         type, QString(),
                                                         vfnActions);
            return sResource;
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
      if (nullptr == pThisGuard) { return QString(); }
      delete dlg;
    }
  }
  return QString();
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
void CEditorModel::AddEditorToolbox(const QString& sTool, IEditorToolBox* pTool)
{
  if (nullptr != pTool)
  {
    m_editorTools[sTool] = pTool;
  }
}

//----------------------------------------------------------------------------------------
//
const std::map<QString, IEditorToolBox*>& CEditorModel::EditorToolboxes() const
{
  return m_editorTools;
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

    QString sFinalProjName = sNewProjectName;
    QString sError;
    if (!ProjectNameCheck(sNewProjectName, &sError))
    {
      sFinalProjName = ToValidProjectName(sNewProjectName);
      qWarning() << sError;
    }

    qint32 iId = spDbManager->AddProject(QDir(sBaseProjectPath + "/" + sFinalProjName));
    m_spCurrentProject = spDbManager->FindProject(iId);
    const QString sProjName = PhysicalProjectName(m_spCurrentProject);

    if (nullptr != m_spCurrentProject)
    {
      m_spCurrentProject->m_rwLock.lockForRead();
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
    Q_UNUSED(sProjName)
    CDatabaseManager::LoadProject(m_spCurrentProject);

    QReadLocker projectLocker(&m_spCurrentProject->m_rwLock);
    m_bReadOnly = m_spCurrentProject->m_bBundled || m_spCurrentProject->m_bReadOnly;
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

    m_spEditorCompleterModel->SetProject(m_spCurrentProject);

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
  m_spEditableFileModel->InitializeModel(m_spCurrentProject);
  m_spDialogueModel->InitializeModel(m_spCurrentProject);
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

  m_spDialogueModel->DeInitializeModel();
  m_spEditableFileModel->DeInitializeModel();
  m_spResourceTreeModel->DeInitializeModel();
  m_spFlowSceneModel->clearScene();
  m_spUndoStack->clear();

  m_spEditorCompleterModel->SetProject(nullptr);

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

  m_spEditableFileModel->SerializeProject();

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
    m_spCurrentProject->m_rwLock.lockForRead();
    qint32 iId = m_spCurrentProject->m_iId;
    m_spCurrentProject->m_rwLock.unlock();

    CSceneNodeModel* pSceneModel = dynamic_cast<CSceneNodeModel*>(n.nodeDataModel());
    if (nullptr != pSceneModel)
    {
      pSceneModel->SetProjectId(iId);
      pSceneModel->SetResourceItemModel(ResourceTreeModel());
      connect(pSceneModel, &CSceneNodeModel::SignalAddScriptFileRequested,
              this, &CEditorModel::SlotAddNewScriptFileToScene, Qt::UniqueConnection);
      connect(pSceneModel, &CSceneNodeModel::SignalAddLayoutFileRequested,
              this, &CEditorModel::SlotAddNewLayoutFileToScene, Qt::UniqueConnection);
    }
    CPathSplitterModel* pPathSplitterModel = dynamic_cast<CPathSplitterModel*>(n.nodeDataModel());
    if (nullptr != pPathSplitterModel)
    {
      pPathSplitterModel->SetProjectId(iId);
      connect(pPathSplitterModel, &CPathSplitterModel::SignalAddLayoutFileRequested,
              this, &CEditorModel::SlotAddNewLayoutFileToScene, Qt::UniqueConnection);
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
void CEditorModel::SlotAddNewScriptFile(const QString& sCustomInitContent)
{
  if (nullptr == m_spCurrentProject)
  {
    qWarning() << "Node created in null-project.";
    return;
  }

  AddNewFileToScene(m_pParentWidget, nullptr, EResourceType::eScript, sCustomInitContent);
}

//----------------------------------------------------------------------------------------
//
void CEditorModel::SlotAddNewLayoutFile(const QString& sCustomInitContent)
{
  if (nullptr == m_spCurrentProject)
  {
    qWarning() << "Node created in null-project.";
    return;
  }

  AddNewFileToScene(m_pParentWidget, nullptr, EResourceType::eLayout, sCustomInitContent);
}

//----------------------------------------------------------------------------------------
//
void CEditorModel::SlotAddNewScriptFileToScene(const QString& sCustomInitContent)
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
      qint32 iSceneId = pSceneModel->SceneId();
      auto spScene = spDbManager->FindScene(m_spCurrentProject, iSceneId);
      AddNewFileToScene(m_pParentWidget, spScene, EResourceType::eScript, sCustomInitContent);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorModel::SlotAddNewLayoutFileToScene(const QString& sCustomInitContent)
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
      qint32 iSceneId = pSceneModel->SceneId();
      auto spScene = spDbManager->FindScene(m_spCurrentProject, iSceneId);
      AddNewFileToScene(m_pParentWidget, spScene, EResourceType::eLayout, sCustomInitContent);
    }
    CPathSplitterModel* pSplitterModel = dynamic_cast<CPathSplitterModel*>(sender());
    if (nullptr != pSplitterModel)
    {
      AddNewFileToScene(m_pParentWidget, nullptr, EResourceType::eLayout, sCustomInitContent);
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
