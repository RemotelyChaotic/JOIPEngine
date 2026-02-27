#include "CommandAddNewFlow.h"
#include "Application.h"

#include "Editor/EditorCommandIds.h"
#include "Editor/EditorModel.h"

#include "Systems/DatabaseManager.h"
#include "Systems/Database/Resource.h"

#include <QDebug>
#include <QDirIterator>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QPushButton>

namespace
{
  //--------------------------------------------------------------------------------------
  //
  tspResource AddNewFlowFile(tspProject spProject,
                             std::weak_ptr<CDatabaseManager> m_wpDbManager,
                             QPointer<QWidget> pParentForDialog)
  {

    QString sResource =
        CEditorModel::AddNewFile(pParentForDialog, spProject, EResourceType::eFlow,
                                 QObject::tr("Create Flow File"), QString(),
                                 SResourceFormats::FlowFormats(), {});
    auto spDbManager = m_wpDbManager.lock();
    if (nullptr != spProject && nullptr != spDbManager && !sResource.isEmpty())
    {
      return spDbManager->FindResourceInProject(spProject, sResource);
    }
    return nullptr;
  }
}

//----------------------------------------------------------------------------------------
//
CCommandAddNewFlow::CCommandAddNewFlow(const tspProject& spProject,
                                       QPointer<QWidget> pParentForDialog,
                                       QUndoCommand* pParent) :
   QUndoCommand("Added flow file", pParent),
   m_addedResource(),
   m_spCurrentProject(spProject),
   m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>()),
   m_pParentForDialog(pParentForDialog)
{
}
CCommandAddNewFlow::~CCommandAddNewFlow() = default;

//----------------------------------------------------------------------------------------
//
void CCommandAddNewFlow::undo()
{
  if (nullptr != m_addedResource)
  {
    auto spDbManager = m_wpDbManager.lock();
    if (nullptr != spDbManager)
    {
      QReadLocker locker(&m_addedResource->m_rwLock);
      const QString sResource = m_addedResource->m_sName;
      locker.unlock();
      spDbManager->RemoveResource(m_spCurrentProject, sResource);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CCommandAddNewFlow::redo()
{
  if (nullptr == m_addedResource)
  {
    tspResource spResource =
        AddNewFlowFile(m_spCurrentProject, m_wpDbManager, m_pParentForDialog);
    m_addedResource = spResource;
  }
  else
  {
    // re-add removed resource
    auto spDbManager = m_wpDbManager.lock();
    if (nullptr != spDbManager)
    {
      const QString sName = spDbManager->AddResource(m_spCurrentProject,
                                                     QUrl(m_addedResource->m_sPath),
                                                     m_addedResource->m_type,
                                                     m_addedResource->m_sName);
      tspResource spAdded = spDbManager->FindResourceInProject(m_spCurrentProject, sName);
      if (nullptr != spAdded)
      {
        // set source by hand
        spAdded->m_rwLock.lockForWrite();
        spAdded->m_sSource = m_addedResource->m_sSource;
        spAdded->m_rwLock.unlock();
        // store a copy
        m_addedResource.reset(new SResource(*spAdded));
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
int CCommandAddNewFlow::id() const
{
  return EEditorCommandId::eAddFlow;
}

//----------------------------------------------------------------------------------------
//
bool CCommandAddNewFlow::mergeWith(const QUndoCommand* pOther)
{
  if (nullptr == pOther || id() != pOther->id())
  {
    return false;
  }

  const CCommandAddNewFlow* pOtherCasted = dynamic_cast<const CCommandAddNewFlow*>(pOther);
  if (nullptr == pOtherCasted) { return false; }

  return false;
}
