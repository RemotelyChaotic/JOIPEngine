#include "CommandChangeDescribtion.h"
#include "Editor/EditorCommandIds.h"

CCommandChangeDescribtion::CCommandChangeDescribtion(QPointer<QTextDocument> pDescribtionDocument,
                                                     QUndoCommand* pParent) :
  QUndoCommand(QTextDocument::tr("Describtion change"), pParent),
  m_pDescribtionDocument(pDescribtionDocument)
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
  }
}

//----------------------------------------------------------------------------------------
//
void CCommandChangeDescribtion::redo()
{
  if (!m_pDescribtionDocument.isNull())
  {
    m_pDescribtionDocument->redo();
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
