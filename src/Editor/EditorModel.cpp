#include "EditorModel.h"
#include "Application.h"
#include "NodeEditor/EndNodeModel.h"
#include "NodeEditor/FlowScene.h"
#include "NodeEditor/PathMergerModel.h"
#include "NodeEditor/PathSplitterModel.h"
#include "NodeEditor/NodeEditorRegistry.h"
#include "NodeEditor/SceneNodeModel.h"
#include "NodeEditor/SceneNodeModelWidget.h"
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

namespace
{
  const char c_sTemporaryRccFileProperty[] = "RccFile";
}

//----------------------------------------------------------------------------------------
//
CEditorModel::CEditorModel(QWidget* pParent) :
  QObject(nullptr),
  m_spKinkTreeModel(std::make_unique<CKinkTreeModel>()),
  m_spScriptEditorModel(std::make_unique<CScriptEditorModel>(pParent)),
  m_spFlowSceneModel(std::make_unique<CFlowScene>(CNodeEditorRegistry::RegisterDataModels(), nullptr)),
  m_spUndoStack(std::make_unique<QUndoStack>()),
  m_spResourceTreeModel(std::make_unique<CResourceTreeItemModel>(m_spUndoStack.get())),
  m_spExportProcess(std::make_unique<QProcess>()),
  m_spSettings(CApplication::Instance()->Settings()),
  m_spCurrentProject(nullptr),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>()),
  m_vwpTutorialStateSwitchHandlers(),
  m_pParentWidget(pParent),
  m_bInitializingNewProject(false),
  m_bReadOnly(false)
{
  connect(m_spResourceTreeModel.get(), &CResourceTreeItemModel::SignalProjectEdited,
          this, &CEditorModel::SignalProjectEdited, Qt::DirectConnection);
  connect(m_spScriptEditorModel.get(), &CScriptEditorModel::SignalProjectEdited,
          this, &CEditorModel::SignalProjectEdited, Qt::DirectConnection);
  connect(m_spFlowSceneModel.get(), &CFlowScene::nodeCreated,
          this, &CEditorModel::SlotNodeCreated);
  connect(m_spFlowSceneModel.get(), &CFlowScene::nodeDeleted,
          this, &CEditorModel::SlotNodeDeleted);
  connect(m_spExportProcess.get(), &QProcess::errorOccurred,
          this, &CEditorModel::SlotExportErrorOccurred);
  connect(m_spExportProcess.get(), qOverload<int,QProcess::ExitStatus>(&QProcess::finished),
          this, &CEditorModel::SlotExportFinished);
  connect(m_spExportProcess.get(), &QProcess::started,
          this, &CEditorModel::SlotExportStarted);
  connect(m_spExportProcess.get(), &QProcess::stateChanged,
          this, &CEditorModel::SlotExportStateChanged);
}

CEditorModel::~CEditorModel()
{
  // warte auf export
  if (m_spExportProcess->state() != QProcess::ProcessState::NotRunning)
  {
    if (!m_spExportProcess->waitForFinished())
    {
      m_spExportProcess->kill();
    }
  }
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
      const QString sProjectPath = PhysicalProjectPath(m_spCurrentProject);
      QPointer<CEditorModel> pThisGuard(this);
      QFileDialog* dlg = new QFileDialog(pParentForDialog,
                                         tr("Create Script File for %1").arg(spScene->m_sName));
      dlg->setViewMode(QFileDialog::Detail);
      dlg->setFileMode(QFileDialog::AnyFile);
      dlg->setAcceptMode(QFileDialog::AcceptSave);
      dlg->setOptions(QFileDialog::DontUseCustomDirectoryIcons);
      dlg->setSupportedSchemes(QStringList() << QString(CPhysFsFileEngineHandler::c_sScheme).replace(":/", ""));
      dlg->setDirectoryUrl(QUrl(CPhysFsFileEngineHandler::c_sScheme));
      dlg->setFilter(QDir::AllDirs);
      dlg->setNameFilter(QString("Script Files (%1)").arg(SResourceFormats::ScriptFormats().join(" ")));
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
          QDir projectDir(sProjectPath);
          if (!info.absoluteFilePath().contains(projectDir.absolutePath()))
          {
            qWarning() << "File is not in subfolder of Project.";
          }
          else
          {
            QString sRelativePath = projectDir.relativeFilePath(info.absoluteFilePath());
            QUrl sUrlToSave = ResourceUrlFromLocalFile(sRelativePath);
            QFile jsFile(info.absoluteFilePath());
            if (jsFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
            {
              jsFile.write(QString("// insert code to control scene").toUtf8());

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
void CEditorModel::InitNewProject(const QString& sNewProjectName, bool bTutorial)
{
  m_bInitializingNewProject = true;

  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
#if Q_OS_ANDROID
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

  // warte auf export
  if (m_spExportProcess->state() != QProcess::ProcessState::NotRunning)
  {
    if (!m_spExportProcess->waitForFinished())
    {
      m_spExportProcess->kill();
    }
  }

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

  // warte auf export
  if (m_spExportProcess->state() != QProcess::ProcessState::NotRunning)
  {
    if (!m_spExportProcess->waitForFinished())
    {
      m_spExportProcess->kill();
    }
  }

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
  // warte auf export
  if (m_spExportProcess->state() != QProcess::ProcessState::NotRunning)
  {
    if (!m_spExportProcess->waitForFinished())
    {
      m_spExportProcess->kill();
    }
  }

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
void CEditorModel::ExportProject()
{
  if (nullptr == m_spCurrentProject)
  {
    return;
  }

  const QString sName = PhysicalProjectName(m_spCurrentProject);
  const QString sFolder = CPhysFsFileEngineHandler::c_sScheme + sName;

  QFile rccFile(sFolder + "/" + "JOIPEngineExport.qrc");
  if (!rccFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
  {
    QString sError =  tr("Could not write temporary resource file '%1'").arg(rccFile.fileName());
    qWarning() << sError;
    emit SignalProjectExportError(EExportError::eWriteFailed, sError);
    return;
  }

  QTextStream outStream(&rccFile);
  outStream.setCodec("UTF-8");
  outStream << "<!DOCTYPE RCC><RCC version=\"1.0\">" << "<qresource prefix=\"/\">";
  m_spCurrentProject->m_rwLock.lockForRead();
  for (auto spResourcePair : m_spCurrentProject->m_spResourcesMap)
  {
    const QString sOutPath = ResourceUrlToAbsolutePath(spResourcePair.second);
    spResourcePair.second->m_rwLock.lockForRead();
    if (IsLocalFile(spResourcePair.second->m_sPath))
    {
      outStream << QString("<file alias=\"%2\">%1</file>")
                   .arg(sOutPath)
                   .arg(spResourcePair.second->m_sName);
    }
    spResourcePair.second->m_rwLock.unlock();
  }
  m_spCurrentProject->m_rwLock.unlock();
  outStream << QString("<file alias=\"%2\">%1</file>")
               .arg(joip_resource::c_sProjectFileName)
               .arg(joip_resource::c_sProjectFileName);
  outStream << "</qresource>" << "</RCC>";

  rccFile.close();

  if (m_spExportProcess->state() == QProcess::ProcessState::NotRunning)
  {
    m_spExportProcess->setProperty(c_sTemporaryRccFileProperty, rccFile.fileName());
    m_spExportProcess->setWorkingDirectory(sFolder);
    m_spExportProcess->start("rcc",
                             QStringList() << "--binary" << "--no-compress"
                             << rccFile.fileName()
                             << "--output" << sFolder + "/" + sName + ".proj");
  }
  else
  {
    QString sError = tr("Export is allready running.");
    qWarning() << sError;
    emit SignalProjectExportError(EExportError::eProcessError, sError);
    emit SignalProjectExportFinished();
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
void CEditorModel::SlotExportErrorOccurred(QProcess::ProcessError error)
{
  switch (error)
  {
  case QProcess::ProcessError::Crashed: qWarning() << tr("Export process crashed."); break;
  case QProcess::ProcessError::Timedout: qWarning() << tr("Export process timed out."); break;
  case QProcess::ProcessError::ReadError: qWarning() << tr("Export process read error."); break;
  case QProcess::ProcessError::WriteError: qWarning() << tr("Export process write error."); break;
  case QProcess::ProcessError::UnknownError: qWarning() << tr("Unknown error in export process."); break;
  case QProcess::ProcessError::FailedToStart: qWarning() << tr("Export process failed to start."); break;
  default: break;
  }

  qWarning() << m_spExportProcess->errorString();
}

//----------------------------------------------------------------------------------------
//
void CEditorModel::SlotExportFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
  if (exitStatus == QProcess::ExitStatus::CrashExit)
  {
    QString sError = tr("Export process crashed with code %1 (%2).")
        .arg(exitCode).arg(m_spExportProcess->errorString());
    qWarning() << sError;
    emit SignalProjectExportError(EExportError::eProcessError, sError);
  }

  QFile rccFile(m_spExportProcess->property(c_sTemporaryRccFileProperty).toString());
  if (!rccFile.remove())
  {
    QString sError = tr("Could not remove temporary qrc file '%1'.")
        .arg(rccFile.fileName());
    qWarning() << sError;
    emit SignalProjectExportError(EExportError::eCleanupFailed, sError);
  }

  emit SignalProjectExportFinished();
}

//----------------------------------------------------------------------------------------
//
void CEditorModel::SlotExportStarted()
{
  emit SignalProjectExportStarted();
}

//----------------------------------------------------------------------------------------
//
void CEditorModel::SlotExportStateChanged(QProcess::ProcessState newState)
{
  switch (newState)
  {
  case QProcess::ProcessState::Running: qDebug() << tr("Running export."); break;
  case QProcess::ProcessState::Starting: qDebug() << tr("Starting export."); break;
  case QProcess::ProcessState::NotRunning: qDebug() << tr("Export finished."); break;
  default: break;
  }
}

