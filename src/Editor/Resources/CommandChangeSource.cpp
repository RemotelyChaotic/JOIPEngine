#include "CommandChangeSource.h"
#include "Application.h"
#include "Editor/EditorCommandIds.h"
#include "Systems/DatabaseManager.h"

CCommandChangeSource::CCommandChangeSource(const tspProject& spCurrentProject,
                                           const QString& sNameOfResource,
                                           const QUrl& sOldSource,
                                           const QUrl& sNewSource,
                                           const std::function<void(void)>& fnOnChanged,
                                           QUndoCommand* pParent) :
  QUndoCommand(QString("Source of %1 changed").arg(sNameOfResource), pParent),
  m_spCurrentProject(spCurrentProject),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>()),
  m_sNameOfResource(sNameOfResource),
  m_sOldSource(sOldSource),
  m_sNewSource(sNewSource),
  m_fnOnChanged(fnOnChanged)
{
}
CCommandChangeSource::~CCommandChangeSource()
{}

//----------------------------------------------------------------------------------------
//
void CCommandChangeSource::undo()
{
  DoUndoRedo(m_sOldSource);
}

//----------------------------------------------------------------------------------------
//
void CCommandChangeSource::redo()
{
  DoUndoRedo(m_sNewSource);
}

//----------------------------------------------------------------------------------------
//
int CCommandChangeSource::id() const
{
  return EEditorCommandId::eChangeSource;
}

//----------------------------------------------------------------------------------------
//
bool CCommandChangeSource::mergeWith(const QUndoCommand* pOther)
{
  if (nullptr == pOther || id() != pOther->id())
  {
    return false;
  }

  const CCommandChangeSource* pOtherCasted = dynamic_cast<const CCommandChangeSource*>(pOther);
  if (nullptr == pOtherCasted) { return false; }
  if (m_sNameOfResource == pOtherCasted->m_sNameOfResource) { return false; }

  m_sNewSource = pOtherCasted->m_sNewSource;

  return true;
}

//----------------------------------------------------------------------------------------
//
void CCommandChangeSource::DoUndoRedo(const QUrl& sSource)
{
  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    tspResource spResource = spDbManager->FindResourceInProject(m_spCurrentProject, m_sNameOfResource);
    if (nullptr != spResource)
    {
      {
        QWriteLocker locker(&spResource->m_rwLock);
        spResource->m_sSource = sSource;
      }

      if (nullptr != m_fnOnChanged) { m_fnOnChanged(); }
    }
  }
}
