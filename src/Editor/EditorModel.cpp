#include "EditorModel.h"
#include "Application.h"
#include "Backend/DatabaseManager.h"
#include "Backend/Project.h"
#include "Resources/ResourceTreeItemModel.h"

#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QWidget>

CEditorModel::CEditorModel(QObject* pParent) :
  QObject(pParent),
  m_spResourceTreeModel(std::make_unique<CResourceTreeItemModel>()),
  m_spSettings(CApplication::Instance()->Settings()),
  m_spCurrentProject(nullptr),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>()),
  m_bInitializingNewProject(false)
{
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
CResourceTreeItemModel* CEditorModel::ResourceTreeModel() const
{
  return m_spResourceTreeModel.get();
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
      LoadProject(iId);
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
  }

  m_spResourceTreeModel->InitializeModel(m_spCurrentProject);
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

    spDbManager->RenameProject(iId, sNewProjectName);

    m_spCurrentProject->m_rwLock.lockForRead();
    const QString sName = m_spCurrentProject->m_sName;
    m_spCurrentProject->m_rwLock.unlock();
    return sName;
  }
  return QString();
}

//----------------------------------------------------------------------------------------
//
void CEditorModel::UnloadProject()
{
  m_spResourceTreeModel->DeInitializeModel();

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
  if (nullptr != m_spCurrentProject)
  {
    qWarning() << "Trying to serialize null-project.";
    return;
  }

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
