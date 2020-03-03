#include "EditorModel.h"
#include "Application.h"
#include "Backend/DatabaseManager.h"
#include "Backend/Project.h"
#include <QDebug>

CEditorModel::CEditorModel(QObject* pParent) :
  QObject(pParent),
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
