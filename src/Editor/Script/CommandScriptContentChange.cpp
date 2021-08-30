#include "CommandScriptContentChange.h"
#include "Editor/EditorCommandIds.h"

CCommandScriptContentChange::CCommandScriptContentChange(QPointer<QTextDocument> pScriptDocument,
                                                         QUndoCommand* pParent) :
  QUndoCommand(QTextDocument::tr("Script change"), pParent),
  m_pScriptDocument(pScriptDocument),
  m_bAddedRedoCommand(false)
{
}
CCommandScriptContentChange::~CCommandScriptContentChange()
{}

//----------------------------------------------------------------------------------------
//
void CCommandScriptContentChange::undo()
{
  if (!m_pScriptDocument.isNull())
  {
    m_pScriptDocument->undo();
  }
}

//----------------------------------------------------------------------------------------
//
void CCommandScriptContentChange::redo()
{
  // guard to not redo upon adding, since we add this after a command has been added to the
  // internal stack of the document
  if (!m_bAddedRedoCommand)
  {
    m_bAddedRedoCommand = true;
    return;
  }

  if (!m_pScriptDocument.isNull())
  {
    m_pScriptDocument->redo();
  }
}

//----------------------------------------------------------------------------------------
//
int CCommandScriptContentChange::id() const
{
  return EEditorCommandId::eNone;
}

//----------------------------------------------------------------------------------------
//
bool CCommandScriptContentChange::mergeWith(const QUndoCommand* pOther)
{
  Q_UNUSED(pOther);
  return false;
}
