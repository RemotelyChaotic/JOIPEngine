#include "CommandAddNewDialogFile.h"
#include "Application.h"

#include "Editor/EditorCommandIds.h"
#include "Editor/EditorModel.h"

#include "Systems/DatabaseManager.h"

CCommandAddNewDialogueFile::CCommandAddNewDialogueFile(const tspProject& spProject,
                                                   QPointer<CEditorModel> pEditorModel,
                                                   QPointer<QWidget> pParent) :
  QUndoCommand("Added new dialogue file"),
  m_spProject(spProject),
  m_pEditorModel(pEditorModel),
  m_pParent(pParent)
{
}
CCommandAddNewDialogueFile::~CCommandAddNewDialogueFile()
{

}

//----------------------------------------------------------------------------------------
//
void CCommandAddNewDialogueFile::undo()
{
  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager && !m_data.m_sName.isEmpty())
  {
    spDbManager->RemoveResource(m_spProject, m_data.m_sName);
  }
}

//----------------------------------------------------------------------------------------
//
void CCommandAddNewDialogueFile::redo()
{
  if (m_data.m_sName.isEmpty() && nullptr != m_pEditorModel)
  {
    m_data.m_sName = m_pEditorModel->AddNewFileToScene(
        m_pParent, nullptr, EResourceType::eDatabase, "{}",
        QStringList() << QString("*.%1").arg(joip_resource::c_sDialogueFileType));

    setText("Added new dialogue file " + m_data.m_sName);

    auto spDbManager = m_wpDbManager.lock();
    if (nullptr != spDbManager && !m_data.m_sName.isEmpty())
    {
      tspResource spResource = spDbManager->FindResourceInProject(m_spProject, m_data.m_sName);
      QReadLocker l(&spResource->m_rwLock);
      m_data = *spResource;
    }
  }
  else
  {
    auto spDbManager = m_wpDbManager.lock();
    if (nullptr != spDbManager && !m_data.m_sName.isEmpty())
    {
      spDbManager->AddResource(m_spProject,
                               QUrl(m_data.m_sPath),
                               m_data.m_type,
                               m_data.m_sName);
      tspResource spAdded = spDbManager->FindResourceInProject(m_spProject, m_data.m_sName);
      if (nullptr != spAdded)
      {
        // set source by hand
        spAdded->m_rwLock.lockForWrite();
        spAdded->CopyFrom(m_data);
        spAdded->m_rwLock.unlock();
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
int CCommandAddNewDialogueFile::id() const
{
  return EEditorCommandId::eAddNewDialogueFile;
}

//----------------------------------------------------------------------------------------
//
bool CCommandAddNewDialogueFile::mergeWith(const QUndoCommand* pOther)
{
  Q_UNUSED(pOther)
  return false;
}

//----------------------------------------------------------------------------------------
//
QString CCommandAddNewDialogueFile::AddedResource() const
{
  return m_data.m_sName;
}
