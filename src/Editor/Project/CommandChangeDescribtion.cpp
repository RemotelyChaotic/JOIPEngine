#include "CommandChangeDescribtion.h"
#include "Editor/EditorCommandIds.h"

CCommandChangeDescribtion::CCommandChangeDescribtion(QPointer<QTextDocument> pDescribtionDocument,
                                                     const std::function<void(void)>& fnOnUndoRedo,
                                                     QUndoCommand* pParent) :
  QUndoCommand(QTextDocument::tr("Description change"), pParent),
  m_pDescribtionDocument(pDescribtionDocument),
  m_fnOnUndoRedo(fnOnUndoRedo),
  m_bAddedRedoCommand(false)
{
}
CCommandChangeDescribtion::~CCommandChangeDescribtion()
{

}

//----------------------------------------------------------------------------------------
//
void CCommandChangeDescribtion::undo()
{
  if (!m_pDescribtionDocument.isNull())
  {
    m_pDescribtionDocument->undo();
    if (nullptr != m_fnOnUndoRedo)
    {
      m_fnOnUndoRedo();
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CCommandChangeDescribtion::redo()
{
  // guard to not redo upon adding, since we add this after a command has been added to the
  // internal stack of the document
  if (!m_bAddedRedoCommand)
  {
    m_bAddedRedoCommand = true;
    return;
  }

  if (!m_pDescribtionDocument.isNull())
  {
    m_pDescribtionDocument->redo();
    if (nullptr != m_fnOnUndoRedo)
    {
      m_fnOnUndoRedo();
    }
  }
}

//----------------------------------------------------------------------------------------
//
int CCommandChangeDescribtion::id() const
{
  return EEditorCommandId::eNone;
}

//----------------------------------------------------------------------------------------
//
bool CCommandChangeDescribtion::mergeWith(const QUndoCommand* pOther)
{
  Q_UNUSED(pOther);
  return false;
}
