#include "CommandRemoveResource.h"
#include "Application.h"
#include "Editor/EditorCommandIds.h"
#include "Systems/DatabaseManager.h"
#include <QFileInfo>

namespace
{
  QString CommandTextForFiles(const QStringList& files)
  {
    if (0 == files.size()) { return QString("Removing 0 resources."); }
    else if (1 == files.size())
    {
      return QString("Removing resource file %1.").arg(QFileInfo(files.front()).fileName());
    }
    else { return QString("Removing %1 resources.").arg(files.size()); }
  }
  QString CommandTextForResources(const tspResourceMap& files)
  {
    if (0 == files.size()) { return QString("Removing 0 resources."); }
    else if (1 == files.size())
    {
      return QString("Removing resource file %1.").arg(files.begin()->first);
    }
    else { return QString("Removing %1 resources.").arg(files.size()); }
  }
}

//----------------------------------------------------------------------------------------
//
CCommandRemoveResource::CCommandRemoveResource(const tspProject& spProject,
                                               const QStringList& vsResources,
                                               QUndoCommand* pParent) :
  QUndoCommand(CommandTextForFiles(vsResources), pParent),
  m_removedResources(),
  m_spCurrentProject(spProject),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>()),
  m_vsResources(vsResources)
{

}

CCommandRemoveResource::~CCommandRemoveResource()
{}

//----------------------------------------------------------------------------------------
//
void CCommandRemoveResource::undo()
{
  // re-add removed resources
  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    for (auto& resourceIt : m_removedResources)
    {
      const QString sName = spDbManager->AddResource(m_spCurrentProject,
                                                     QUrl(resourceIt.second->m_sPath),
                                                     resourceIt.second->m_type,
                                                     resourceIt.first);
      tspResource spAdded = spDbManager->FindResourceInProject(m_spCurrentProject, sName);
      if (nullptr != spAdded)
      {
        // set source by hand
        spAdded->m_rwLock.lockForWrite();
        spAdded->m_sSource = resourceIt.second->m_sSource;
        spAdded->m_rwLock.unlock();
        // store a copy
        resourceIt.second.reset(new SResource(*spAdded));
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CCommandRemoveResource::redo()
{
  if (m_removedResources.empty())
  {
    auto spDbManager = m_wpDbManager.lock();
    if (nullptr != spDbManager)
    {
      for (const QString& sResource : qAsConst(m_vsResources))
      {
        auto spResource = spDbManager->FindResourceInProject(m_spCurrentProject, sResource);
        if (nullptr != spResource)
        {
          // store a copy
          m_removedResources.insert({sResource, std::make_shared<SResource>(*spResource)});
        }
      }
    }
  }

  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    for (const auto& it : m_removedResources)
    {
      spDbManager->RemoveResource(m_spCurrentProject, it.first);
    }
  }
}

//----------------------------------------------------------------------------------------
//
int CCommandRemoveResource::id() const
{
  return EEditorCommandId::eRemoveResource;
}

//----------------------------------------------------------------------------------------
//
bool CCommandRemoveResource::mergeWith(const QUndoCommand* pOther)
{
  if (nullptr == pOther || id() != pOther->id())
  {
    return false;
  }

  const CCommandRemoveResource* pOtherCasted = dynamic_cast<const CCommandRemoveResource*>(pOther);
  if (nullptr == pOtherCasted) { return false; }

  m_removedResources.insert(pOtherCasted->m_removedResources.begin(),
                            pOtherCasted->m_removedResources.end());
  setText(CommandTextForResources(m_removedResources));
  return true;
}
