#include "EditorModel.h"
#include "Application.h"
#include "NodeEditor/EndNodeModel.h"
#include "NodeEditor/PathMergerModel.h"
#include "NodeEditor/PathSplitterModel.h"
#include "NodeEditor/SceneNodeModel.h"
#include "NodeEditor/SceneTranstitionData.h"
#include "NodeEditor/StartNodeModel.h"
#include "Project/KinkTreeModel.h"
#include "Resources/ResourceTreeItemModel.h"
#include "Script/ScriptEditorModel.h"
#include "Systems/DatabaseManager.h"
#include "Systems/Project.h"

#include <nodes/ConnectionStyle>
#include <nodes/DataModelRegistry>
#include <nodes/FlowScene>
#include <nodes/Node>
#include <nodes/NodeData>
#include <nodes/NodeDataModel>

#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QWidget>

using QtNodes::DataModelRegistry;
using QtNodes::FlowScene;
using QtNodes::Node;

namespace
{
  std::shared_ptr<DataModelRegistry> RegisterDataModels()
  {
    auto ret = std::make_shared<DataModelRegistry>();
    ret->registerModel<CStartNodeModel>("Control");
    ret->registerModel<CSceneNodeModel>("Scene");
    ret->registerModel<CEndNodeModel>("Control");
    ret->registerModel<CPathMergerModel>("Path");
    ret->registerModel<CPathSplitterModel>("Path");
    return ret;
  }
}

//----------------------------------------------------------------------------------------
//
CEditorModel::CEditorModel(QWidget* pParent) :
  QObject(nullptr),
  m_spKinkTreeModel(std::make_unique<CKinkTreeModel>()),
  m_spResourceTreeModel(std::make_unique<CResourceTreeItemModel>()),
  m_spScriptEditorModel(std::make_unique<CScriptEditorModel>(pParent)),
  m_spFlowSceneModel(std::make_unique<FlowScene>(RegisterDataModels(), nullptr)),
  m_spSettings(CApplication::Instance()->Settings()),
  m_spCurrentProject(nullptr),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>()),
  m_pParentWidget(pParent),
  m_bInitializingNewProject(false)
{
  connect(m_spResourceTreeModel.get(), &CResourceTreeItemModel::SignalProjectEdited,
          this, &CEditorModel::SignalProjectEdited, Qt::DirectConnection);
  connect(m_spScriptEditorModel.get(), &CScriptEditorModel::SignalProjectEdited,
          this, &CEditorModel::SignalProjectEdited, Qt::DirectConnection);
  connect(m_spFlowSceneModel.get(), &FlowScene::nodeCreated,
          this, &CEditorModel::SlotNodeCreated);
  connect(m_spFlowSceneModel.get(), &FlowScene::nodeDeleted,
          this, &CEditorModel::SlotNodeDeleted);
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
QtNodes::FlowScene* CEditorModel::FlowSceneModel() const
{
  return m_spFlowSceneModel.get();
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
void CEditorModel::AddFilesToProjectResources(QPointer<QWidget> pParentForDialog,
  const QStringList& vsFiles, const QStringList& imageFormatsList,
  const QStringList& videoFormatsList, const QStringList& audioFormatsList,
  const QStringList& otherFormatsList)
{
  if (nullptr == m_spCurrentProject) { return; }

  // add file to respective category
  bool bAddedFiles = false;
  QStringList vsNeedsToMove;
  const QString sName = PhysicalProjectName(m_spCurrentProject);
  const QDir projectDir(m_spSettings->ContentFolder() + "/" + sName);
  for (QString sFileName : vsFiles)
  {
    QFileInfo info(sFileName);
    if (!info.canonicalFilePath().contains(projectDir.absolutePath()))
    {
      vsNeedsToMove.push_back(sFileName);
    }
    else
    {
      QString sRelativePath = projectDir.relativeFilePath(sFileName);
      QUrl url = QUrl::fromLocalFile(sRelativePath);
      const QString sEnding = "*." + info.suffix();
      auto spDbManager = m_wpDbManager.lock();
      if (nullptr != spDbManager)
      {
        if (imageFormatsList.contains(sEnding))
        {
          spDbManager->AddResource(m_spCurrentProject, url, EResourceType::eImage);
          bAddedFiles = true;
        }
        else if (videoFormatsList.contains(sEnding))
        {
          spDbManager->AddResource(m_spCurrentProject, url, EResourceType::eMovie);
          bAddedFiles = true;
        }
        else if (audioFormatsList.contains(sEnding))
        {
          spDbManager->AddResource(m_spCurrentProject, url, EResourceType::eSound);
          bAddedFiles = true;
        }
        else if (otherFormatsList.contains(sEnding))
        {
          spDbManager->AddResource(m_spCurrentProject, url, EResourceType::eOther);
          bAddedFiles = true;
        }
      }
    }
  }

  // handle action
  if (bAddedFiles)
  {
    emit SignalProjectEdited();
  }
  if (!vsNeedsToMove.isEmpty())
  {
    QMessageBox msgBox(pParentForDialog);
    msgBox.setText(tr("At least one file is not in the subfolder of project."));
    msgBox.setInformativeText(tr("Do you want to move or copy the file(s)?"));
    QPushButton* pMove = msgBox.addButton(tr("Move"), QMessageBox::AcceptRole);
    QPushButton* pCopy = msgBox.addButton(tr("Copy"), QMessageBox::ActionRole);
    QPushButton* pCancel = msgBox.addButton(QMessageBox::Cancel);
    msgBox.setDefaultButton(pCancel);
    msgBox.setModal(true);
    msgBox.setWindowFlag(Qt::FramelessWindowHint);

    QPointer<CEditorModel> pMeMyselfMyPointerAndI(this);
    msgBox.exec();
    if (nullptr == pMeMyselfMyPointerAndI)
    {
      return;
    }

    QStringList filesToAdd;
    if (msgBox.clickedButton() == pMove)
    {
      // Move the Files
      const QString sDirToMoveTo = QFileDialog::getExistingDirectory(pParentForDialog,
          tr("Select Destination"), projectDir.absolutePath());
      for (QString sFileName : vsFiles)
      {
        QFileInfo info(sFileName);
        QFile file(info.absoluteFilePath());
        const QString sNewName = sDirToMoveTo + "/" + info.fileName();
        if (!file.rename(sNewName))
        {
          qWarning() << QString(tr("Renaming file '%1' failed.")).arg(sNewName);
        }
        else
        {
          if (sNewName.contains(projectDir.absolutePath()))
          {
            filesToAdd.push_back(sNewName);
          }
        }
      }
      AddFilesToProjectResources(pParentForDialog, filesToAdd, imageFormatsList,
                                 videoFormatsList, audioFormatsList, otherFormatsList);
    }
    else if (msgBox.clickedButton() == pCopy)
    {
      // copy the Files
      const QString sDirToCopyTo = QFileDialog::getExistingDirectory(pParentForDialog,
          tr("Select Destination"), projectDir.absolutePath());
      for (QString sFileName : vsFiles)
      {
        QFileInfo info(sFileName);
        QFile file(info.absoluteFilePath());
        const QString sNewName = sDirToCopyTo + "/" + info.fileName();
        if (!file.copy(sNewName))
        {
          qWarning() << QString(tr("Copying file '%1' failed.")).arg(sNewName);
        }
        else
        {
          if (sNewName.contains(projectDir.absolutePath()))
          {
            filesToAdd.push_back(sNewName);
          }
        }
      }
      AddFilesToProjectResources(pParentForDialog, filesToAdd, imageFormatsList,
                                 videoFormatsList, audioFormatsList, otherFormatsList);
    }
    else if (msgBox.clickedButton() == pCancel)
    {
      // nothing to do
    }
  }
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
      const QString sName = PhysicalProjectName(m_spCurrentProject);
      QString sCurrentFolder = CApplication::Instance()->Settings()->ContentFolder();
      QUrl sUrl = QFileDialog::getSaveFileUrl(pParentForDialog,
          QString(tr("Create Script File for %1")).arg(spScene->m_sName),
          QUrl::fromLocalFile(sCurrentFolder + "/" + sName),
          "Script Files (*.js)");

      if (sUrl.isValid())
      {
        QFileInfo info(sUrl.toLocalFile());
        QDir projectDir(m_spSettings->ContentFolder() + "/" + sName);
        if (!info.absoluteFilePath().contains(projectDir.absolutePath()))
        {
          qWarning() << "File is not in subfolder of Project.";
        }
        else
        {
          QString sRelativePath = projectDir.relativeFilePath(info.absoluteFilePath());
          QUrl sUrlToSave = QUrl::fromLocalFile(sRelativePath);
          QFile jsFile(info.absoluteFilePath());
          if (jsFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
          {
            jsFile.write(QString("// instert code to control scene").toUtf8());
            QString sResource = spDbManager->AddResource(m_spCurrentProject, sUrlToSave,
                                                         EResourceType::eOther);
            spScene->m_sScript = sResource;
          }
          else
          {
            qWarning() << "Could not write script file.";
          }
        }
      }
    }
  }
}


//----------------------------------------------------------------------------------------
//
void CEditorModel::InitNewProject(const QString& sNewProjectName)
{
  m_bInitializingNewProject = true;

  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    spDbManager->AddProject(sNewProjectName);
    m_spCurrentProject = spDbManager->FindProject(sNewProjectName);

    if (nullptr != m_spCurrentProject)
    {
      m_spCurrentProject->m_rwLock.lockForRead();
      qint32 iId = m_spCurrentProject->m_iId;
      m_spCurrentProject->m_rwLock.unlock();

      // load project
      LoadProject(iId);
      // serialize base project after init
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
    qWarning() << "Old Project was not unloaded before loading project.";
  }

  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    m_spCurrentProject = spDbManager->FindProject(iId);
    if (nullptr == m_spCurrentProject)
    {
      qWarning() << QString("Project id %1 not found.").arg(iId);
    }


    // nodes
    QReadLocker projectLocker(&m_spCurrentProject->m_rwLock);
    if (!m_spCurrentProject->m_sSceneModel.isNull() &&
        !m_spCurrentProject->m_sSceneModel.isEmpty())
    {
      const QString sModelName = m_spCurrentProject->m_sSceneModel;
      projectLocker.unlock();

      auto spResource = spDbManager->FindResourceInProject(m_spCurrentProject, sModelName);
      if (nullptr != spResource)
      {
        QReadLocker resourceLocker(&spResource->m_rwLock);
        QUrl path = spResource->m_sPath;
        resourceLocker.unlock();

        QString sPath = ResourceUrlToAbsolutePath(path, PhysicalProjectName(m_spCurrentProject));
        QFile modelFile(sPath);
        if (modelFile.open(QIODevice::ReadOnly))
        {
          QByteArray arr = modelFile.readAll();
          m_spFlowSceneModel->loadFromMemory(arr);
        }
        else
        {
          qWarning() << tr("Could not open save scene model file.");
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
      spDbManager->AddResource(m_spCurrentProject, QUrl::fromLocalFile("SceneModel.flow"),
                               EResourceType::eOther, "SceneModel.flow");
      projectLocker.relock();
      m_spCurrentProject->m_sSceneModel = "SceneModel.flow";
    }
    projectLocker.unlock();

    auto spResource = spDbManager->FindResourceInProject(m_spCurrentProject, "SceneModel.flow");
    if (nullptr != spResource)
    {
      QReadLocker resourceLocker(&spResource->m_rwLock);
      QUrl path = spResource->m_sPath;
      resourceLocker.unlock();

      QString sPath = ResourceUrlToAbsolutePath(path, PhysicalProjectName(m_spCurrentProject));
      QFile modelFile(sPath);
      if (modelFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
      {
        modelFile.write(arr);
      }
      else
      {
        qWarning() << tr("Could not open save scene model file.");
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorModel::UnloadProject()
{
  m_spScriptEditorModel->DeInitializeModel();
  m_spResourceTreeModel->DeInitializeModel();
  m_spFlowSceneModel->clearScene();

  // reset to what is in the database
  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager && nullptr != m_spCurrentProject)
  {
    m_spCurrentProject->m_rwLock.lockForRead();
    qint32 iId = m_spCurrentProject->m_iId;
    m_spCurrentProject->m_rwLock.unlock();

    spDbManager->DeserializeProject(iId);
  }
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

