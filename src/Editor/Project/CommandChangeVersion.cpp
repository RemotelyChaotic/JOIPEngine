#include "CommandChangeVersion.h"
#include "Editor/EditorCommandIds.h"
#include "Editor/EditorWidgetTypes.h"

CCommandChangeVersion::CCommandChangeVersion(QPointer<QSpinBox> pProjectMajorVersion,
                                             QPointer<QSpinBox> pProjectMinorVersion,
                                             QPointer<QSpinBox> pProjectPatchVersion,
                                             QUndoCommand* pParent) :
  QUndoCommand("Version -> " + static_cast<QString>(SVersion(pProjectMajorVersion->value(),
                                                             pProjectMinorVersion->value(),
                                                             pProjectPatchVersion->value())), pParent),
  m_pProjectMajorVersion(pProjectMajorVersion),
  m_pProjectMinorVersion(pProjectMinorVersion),
  m_pProjectPatchVersion(pProjectPatchVersion),
  m_originalVersion(pProjectMajorVersion->property(editor::c_sPropertyOldValue).toInt(),
                    pProjectMinorVersion->property(editor::c_sPropertyOldValue).toInt(),
                    pProjectPatchVersion->property(editor::c_sPropertyOldValue).toInt()),
  m_newVersion(pProjectMajorVersion->value(),
               pProjectMinorVersion->value(),
               pProjectPatchVersion->value())
{
}
CCommandChangeVersion::~CCommandChangeVersion()
{}

//----------------------------------------------------------------------------------------
//
void CCommandChangeVersion::undo()
{
  ApplyValue(m_originalVersion);
}

//----------------------------------------------------------------------------------------
//
void CCommandChangeVersion::redo()
{
  ApplyValue(m_newVersion);
}

//----------------------------------------------------------------------------------------
//
int CCommandChangeVersion::id() const
{
  return EEditorCommandId::eChangeVersion;
}

//----------------------------------------------------------------------------------------
//
bool CCommandChangeVersion::mergeWith(const QUndoCommand* pOther)
{
  if (nullptr == pOther || id() != pOther->id())
  {
    return false;
  }

  const CCommandChangeVersion* pOtherCasted = dynamic_cast<const CCommandChangeVersion*>(pOther);
  if (nullptr == pOtherCasted) { return false; }

  m_newVersion = pOtherCasted->m_newVersion;
  setText("Version -> " + static_cast<QString>(m_newVersion));
  return true;
}

//----------------------------------------------------------------------------------------
//
void CCommandChangeVersion::ApplyValue(const SVersion& value)
{
  if (nullptr != m_pProjectMajorVersion)
  {
    m_pProjectMajorVersion->blockSignals(true);
    m_pProjectMajorVersion->setProperty(editor::c_sPropertyOldValue, value.m_iMajor);
    m_pProjectMajorVersion->setValue(value.m_iMajor);
    m_pProjectMajorVersion->blockSignals(false);
  }
  if (nullptr != m_pProjectMinorVersion)
  {
    m_pProjectMinorVersion->blockSignals(true);
    m_pProjectMajorVersion->setProperty(editor::c_sPropertyOldValue, value.m_iMinor);
    m_pProjectMinorVersion->setValue(value.m_iMinor);
    m_pProjectMinorVersion->blockSignals(false);
  }
  if (nullptr != m_pProjectPatchVersion)
  {
    m_pProjectPatchVersion->blockSignals(true);
    m_pProjectMajorVersion->setProperty(editor::c_sPropertyOldValue, value.m_iPatch);
    m_pProjectPatchVersion->setValue(value.m_iPatch);
    m_pProjectPatchVersion->blockSignals(false);
  }
}
