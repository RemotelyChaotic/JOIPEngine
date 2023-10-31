#include "ScriptEditorKeyHandler.h"
#include "ScriptEditorWidget.h"

#include <QTextCursor>
#include <QTextDocumentFragment>

namespace
{
  template<typename T>
  void RegisterHandler(std::map<Qt::Key, std::shared_ptr<IScriptEditorKeyHandler>>& handlerMap,
                       CScriptEditorWidget* pCodeEditor,
                       Qt::Key* pPreviouslyClickedKey)
  {
    std::shared_ptr<T> spEnterHandler =
        std::make_shared<T>(pCodeEditor, pPreviouslyClickedKey);
    for (Qt::Key key : spEnterHandler->HandledKeys())
    {
      handlerMap[key] = spEnterHandler;
    }
  }
}

//----------------------------------------------------------------------------------------
//
IScriptEditorKeyHandler::IScriptEditorKeyHandler(CScriptEditorWidget* pCodeEditor,
                                                 Qt::Key* pPreviouslyClickedKey) :
  m_pCodeEditor(pCodeEditor), m_pPreviouslyClickedKey(pPreviouslyClickedKey)
{}
IScriptEditorKeyHandler::~IScriptEditorKeyHandler() = default;

void IScriptEditorKeyHandler::RegisterHandlers(
    std::map<Qt::Key, std::shared_ptr<IScriptEditorKeyHandler>>& handlerMap,
    CScriptEditorWidget* pCodeEditor,
    Qt::Key* pPreviouslyClickedKey)
{
  RegisterHandler<CScriptEditorEnterHandler>(handlerMap, pCodeEditor, pPreviouslyClickedKey);
  RegisterHandler<CScriptEditorTabHandler>(handlerMap, pCodeEditor, pPreviouslyClickedKey);
  RegisterHandler<CScriptEditorBracesHandler>(handlerMap, pCodeEditor, pPreviouslyClickedKey);
  RegisterHandler<CScriptEditorQuotesHandler>(handlerMap, pCodeEditor, pPreviouslyClickedKey);
}

//----------------------------------------------------------------------------------------
//
CScriptEditorEnterHandler::CScriptEditorEnterHandler(CScriptEditorWidget* pCodeEditor,
                                                     Qt::Key* pPreviouslyClickedKey) :
    IScriptEditorKeyHandler(pCodeEditor, pPreviouslyClickedKey)
{
}
CScriptEditorEnterHandler::~CScriptEditorEnterHandler() = default;

bool CScriptEditorEnterHandler::KeyEvent(QKeyEvent* pKeyEvent)
{
  // get indentation of current line, and mimic for new line
  QTextCursor cursor = m_pCodeEditor->textCursor();
  cursor.movePosition(QTextCursor::StartOfLine);
  cursor.select(QTextCursor::SelectionType::LineUnderCursor);
  QString sSelectedText = cursor.selectedText();
  QString sIndentation;
  for (qint32 i = 0; i < sSelectedText.length(); ++i)
  {
    if (sSelectedText[i].isSpace())
    {
      sIndentation += sSelectedText[i];
    }
    else
    {
      break;
    }
  }
  m_pCodeEditor->insertPlainText(QString("\n") + sIndentation);
  *m_pPreviouslyClickedKey = Qt::Key(pKeyEvent->key());
  pKeyEvent->ignore();
  return true;
}
std::set<Qt::Key> CScriptEditorEnterHandler::HandledKeys() const
{
  return {Qt::Key_Enter, Qt::Key_Return};
}

//----------------------------------------------------------------------------------------
//
CScriptEditorTabHandler::CScriptEditorTabHandler(CScriptEditorWidget* pCodeEditor,
                                                 Qt::Key* pPreviouslyClickedKey) :
  IScriptEditorKeyHandler(pCodeEditor, pPreviouslyClickedKey)
{
}
CScriptEditorTabHandler::~CScriptEditorTabHandler() = default;

bool CScriptEditorTabHandler::KeyEvent(QKeyEvent* pKeyEvent)
{
  QTextCursor cursor = m_pCodeEditor->textCursor();
  qint32 iSelectedLines = 0;
  QString sSelection;
  if(cursor.hasSelection())
  {
    sSelection = cursor.selection().toPlainText();
    iSelectedLines = sSelection.count("\n")+1;
  }

  if (2 > iSelectedLines)
  {
    m_pCodeEditor->insertPlainText(QString("").leftJustified(4, ' ', false));
    pKeyEvent->ignore();
  }
  else
  {
    QStringList vsLines = sSelection.split("\n");
    for (QString& sLine : vsLines)
    {
      sLine.prepend(QString("").leftJustified(4, ' ', false));
    }
    *m_pPreviouslyClickedKey = Qt::Key(pKeyEvent->key());
    m_pCodeEditor->insertPlainText(vsLines.join("\n"));
    pKeyEvent->ignore();
  }
  // workaround for losing focus after blocking tab
  QMetaObject::invokeMethod(m_pCodeEditor, "setFocus", Qt::QueuedConnection);
  return true;
}
std::set<Qt::Key> CScriptEditorTabHandler::HandledKeys() const
{
  return {Qt::Key_Tab};
}

//----------------------------------------------------------------------------------------
//

CScriptEditorBracesHandler::CScriptEditorBracesHandler(CScriptEditorWidget* pCodeEditor,
                                                       Qt::Key* pPreviouslyClickedKey) :
  IScriptEditorKeyHandler(pCodeEditor, pPreviouslyClickedKey)
{
}
CScriptEditorBracesHandler::~CScriptEditorBracesHandler() = default;

bool CScriptEditorBracesHandler::KeyEvent(QKeyEvent* pKeyEvent)
{
  if (pKeyEvent->key() == Qt::Key_BraceLeft)
  {
    m_pCodeEditor->insertPlainText(QString("{  }"));
    QTextCursor cursor = m_pCodeEditor->textCursor();
    cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, 2);
    m_pCodeEditor->setTextCursor(cursor);
    *m_pPreviouslyClickedKey = Qt::Key(pKeyEvent->key());
    pKeyEvent->ignore();
    return true;
  }
  else if (pKeyEvent->key() == Qt::Key_BraceRight &&
           Qt::Key_BraceLeft == *m_pPreviouslyClickedKey)
  {
    QTextCursor cursor = m_pCodeEditor->textCursor();
    cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, 2);
    m_pCodeEditor->setTextCursor(cursor);
    *m_pPreviouslyClickedKey = Qt::Key(0);
    pKeyEvent->ignore();
    return true;
  }
  else if (pKeyEvent->key() == Qt::Key_ParenLeft)
  {
    m_pCodeEditor->insertPlainText(QString("()"));
    QTextCursor cursor = m_pCodeEditor->textCursor();
    cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, 1);
    *m_pPreviouslyClickedKey = Qt::Key(pKeyEvent->key());
    m_pCodeEditor->setTextCursor(cursor);
    pKeyEvent->ignore();
    return true;
  }
  else if (pKeyEvent->key() == Qt::Key_ParenRight &&
           Qt::Key_ParenLeft == *m_pPreviouslyClickedKey)
  {
    QTextCursor cursor = m_pCodeEditor->textCursor();
    cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, 1);
    m_pCodeEditor->setTextCursor(cursor);
    *m_pPreviouslyClickedKey = Qt::Key(0);
    pKeyEvent->ignore();
    return true;
  }
  else if (pKeyEvent->key() == Qt::Key_BracketLeft)
  {
    m_pCodeEditor->insertPlainText(QString("[]"));
    QTextCursor cursor = m_pCodeEditor->textCursor();
    cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, 1);
    m_pCodeEditor->setTextCursor(cursor);
    *m_pPreviouslyClickedKey = Qt::Key(pKeyEvent->key());
    pKeyEvent->ignore();
    return true;
  }
  else if (pKeyEvent->key() == Qt::Key_BracketRight &&
           Qt::Key_BracketLeft == *m_pPreviouslyClickedKey)
  {
    QTextCursor cursor = m_pCodeEditor->textCursor();
    cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, 1);
    m_pCodeEditor->setTextCursor(cursor);
    *m_pPreviouslyClickedKey = Qt::Key(0);
    pKeyEvent->ignore();
    return true;
  }
  return false;
}
std::set<Qt::Key> CScriptEditorBracesHandler::HandledKeys() const
{
  return {Qt::Key_BraceLeft, Qt::Key_BraceRight, Qt::Key_ParenLeft, Qt::Key_ParenRight,
          Qt::Key_BracketLeft, Qt::Key_BracketRight};
}


//----------------------------------------------------------------------------------------
//
CScriptEditorQuotesHandler::CScriptEditorQuotesHandler(CScriptEditorWidget* pCodeEditor,
                                                       Qt::Key* pPreviouslyClickedKey) :
    IScriptEditorKeyHandler(pCodeEditor, pPreviouslyClickedKey)
{
}
CScriptEditorQuotesHandler::~CScriptEditorQuotesHandler() = default;

bool CScriptEditorQuotesHandler::KeyEvent(QKeyEvent* pKeyEvent)
{
  if (Qt::Key_QuoteDbl != *m_pPreviouslyClickedKey)
  {
    m_pCodeEditor->insertPlainText(QString("\"\""));
    QTextCursor cursor = m_pCodeEditor->textCursor();
    cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, 1);
    m_pCodeEditor->setTextCursor(cursor);
    *m_pPreviouslyClickedKey = Qt::Key(pKeyEvent->key());
  }
  else
  {
    QTextCursor cursor = m_pCodeEditor->textCursor();
    cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, 1);
    m_pCodeEditor->setTextCursor(cursor);
    *m_pPreviouslyClickedKey = Qt::Key(0);
  }
  pKeyEvent->ignore();
  return true;
}
std::set<Qt::Key> CScriptEditorQuotesHandler::HandledKeys() const
{
  return {Qt::Key_QuoteDbl};
}
