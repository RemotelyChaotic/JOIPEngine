#include "HighlightedSearchableTextEdit.h"
#include "EditorHighlighter.h"
#include <QMenu>

CHighlightedSearchableTextEdit::CHighlightedSearchableTextEdit(QPointer<QTextEdit> pEditor) :
  QObject(pEditor),
  m_pEditor(pEditor),
  m_pHighlighter(nullptr),
  m_pSearchBar(nullptr),
  m_highlightCursor(),
  m_sLastSearch()
{
  if (nullptr != pEditor)
  {
    pEditor->viewport()->installEventFilter(this);
  }
  Initalize(pEditor->document(), pEditor);
}

CHighlightedSearchableTextEdit::CHighlightedSearchableTextEdit(QPointer<QPlainTextEdit> pEditor) :
  QObject(pEditor),
  m_pEditor(pEditor),
  m_pHighlighter(nullptr),
  m_pSearchBar(nullptr),
  m_highlightCursor(),
  m_sLastSearch()
{
  if (nullptr != pEditor)
  {
    pEditor->viewport()->installEventFilter(this);
  }
  Initalize(pEditor->document(), pEditor);
}

CHighlightedSearchableTextEdit::~CHighlightedSearchableTextEdit() = default;

//----------------------------------------------------------------------------------------
//
QPointer<CEditorHighlighter> CHighlightedSearchableTextEdit::Highlighter() const
{
  return m_pHighlighter;
}

//----------------------------------------------------------------------------------------
//
QPointer<CEditorSearchBar> CHighlightedSearchableTextEdit::SearchBar() const
{
  return m_pSearchBar;
}

//----------------------------------------------------------------------------------------
//
QMenu* CHighlightedSearchableTextEdit::CreateContextMenu()
{
  QMenu* pMenu = CreateStandardContextMenu();
  pMenu->addAction(tr("Search"), this, SLOT(SlotShowHideSearchFilter()),
                   QKeySequence::Find);
  return pMenu;
}

//----------------------------------------------------------------------------------------
//
void CHighlightedSearchableTextEdit::SetSyntaxHighlightingEnabled(bool bEnabled)
{
  m_pHighlighter->SetSyntaxHighlightingEnabled(bEnabled);
}

//----------------------------------------------------------------------------------------
//
void CHighlightedSearchableTextEdit::SlotShowHideSearchFilter()
{
  if (m_pSearchBar->isVisible())
  {
    m_pSearchBar->Hide();
  }
  else
  {
    QTextCursor cursor = TextCursor();
    m_pSearchBar->SetFilter(cursor.selectedText());
    m_pSearchBar->Show();
  }
}

//----------------------------------------------------------------------------------------
//
void CHighlightedSearchableTextEdit::SlotSearchAreaHidden()
{
  m_highlightCursor = QTextCursor();
  m_sLastSearch = QString();
  m_pHighlighter->SetSearchExpression(QString());
  qobject_cast<QWidget*>(parent())->setFocus();
}

//----------------------------------------------------------------------------------------
//
void CHighlightedSearchableTextEdit::SlotSearchFilterChanged(
    CEditorSearchBar::ESearhDirection direction, const QString& sText)
{
  QTextDocument* pDocument = Document();

  if (!sText.isEmpty())
  {
    QTextCursor highlightCursor = m_highlightCursor;
    if (m_highlightCursor.isNull() || m_sLastSearch.isEmpty() || m_sLastSearch != sText)
    {
      highlightCursor = QTextCursor(pDocument);
    }

    // update highlighting
    if (m_sLastSearch != sText)
    {
      m_pHighlighter->SetSearchExpression(sText);
    }

    m_sLastSearch = sText;

    if (direction != CEditorSearchBar::eNone)
    {
      highlightCursor =
          pDocument->find(sText, highlightCursor,
                          direction == CEditorSearchBar::eForward ?
                            QTextDocument::FindFlags() :
                            QTextDocument::FindBackward);
      if (!highlightCursor.isNull())
      {
        SetTextCursor(highlightCursor);
        m_highlightCursor = highlightCursor;
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
bool CHighlightedSearchableTextEdit::eventFilter(QObject* pTarget, QEvent* pEvent)
{
  if (nullptr != pEvent && nullptr != pTarget)
  {
    if (QEvent::KeyPress == pEvent->type() &&
        (
          (std::holds_alternative<QPointer<QTextEdit>>(m_pEditor) &&
          std::get<QPointer<QTextEdit>>(m_pEditor) == pTarget) ||
          (std::holds_alternative<QPointer<QPlainTextEdit>>(m_pEditor) &&
          std::get<QPointer<QPlainTextEdit>>(m_pEditor) == pTarget)
        ))
    {
      QKeyEvent* pKeyEvent = static_cast<QKeyEvent*>(pEvent);
      // Ctrl+F
      if (pKeyEvent->modifiers().testFlag(Qt::ControlModifier) && pKeyEvent->key() == Qt::Key_F)
      {
        SlotShowHideSearchFilter();
      }
      // Enter
      else if (pKeyEvent->key() == Qt::Key_Enter || pKeyEvent->key() == Qt::Key_Return)
      {
        if (m_pSearchBar->isVisible() &&
            !m_sLastSearch.isEmpty() &&
            qobject_cast<QWidget*>(parent())->focusWidget() != qobject_cast<QWidget*>(parent()))
        {
          SlotSearchFilterChanged(m_pSearchBar->SearchDirection(), m_sLastSearch);
          pEvent->ignore();
          return true;
        }
      }
    }
    if (QEvent::ContextMenu == pEvent->type() &&
        (
          (std::holds_alternative<QPointer<QTextEdit>>(m_pEditor) &&
          std::get<QPointer<QTextEdit>>(m_pEditor)->viewport() == pTarget) ||
          (std::holds_alternative<QPointer<QPlainTextEdit>>(m_pEditor) &&
          std::get<QPointer<QPlainTextEdit>>(m_pEditor)->viewport() == pTarget)
        ))
    {
      QContextMenuEvent* pCEvent = static_cast<QContextMenuEvent*>(pEvent);

      QPointer<CHighlightedSearchableTextEdit> pThisGuard(this);
      QMenu* pMenu = CreateContextMenu();
      pMenu->exec(pCEvent->globalPos());
      if (nullptr == pThisGuard) { return true; }
      delete pMenu;

      return true;
    }
  }

  return false;
}

//----------------------------------------------------------------------------------------
//
QMenu* CHighlightedSearchableTextEdit::CreateStandardContextMenu()
{
  if (std::holds_alternative<QPointer<QTextEdit>>(m_pEditor))
  {
    return std::get<QPointer<QTextEdit>>(m_pEditor)->createStandardContextMenu();
  }
  else if (std::holds_alternative<QPointer<QPlainTextEdit>>(m_pEditor))
  {
    return std::get<QPointer<QPlainTextEdit>>(m_pEditor)->createStandardContextMenu();
  }
  return nullptr;
}

//----------------------------------------------------------------------------------------
//
QTextDocument* CHighlightedSearchableTextEdit::Document() const
{
  if (std::holds_alternative<QPointer<QTextEdit>>(m_pEditor))
  {
    return std::get<QPointer<QTextEdit>>(m_pEditor)->document();
  }
  else if (std::holds_alternative<QPointer<QPlainTextEdit>>(m_pEditor))
  {
    return std::get<QPointer<QPlainTextEdit>>(m_pEditor)->document();
  }
  return nullptr;
}

//----------------------------------------------------------------------------------------
//
void CHighlightedSearchableTextEdit::Initalize(QTextDocument* pDoc, QWidget* pParent)
{
  assert(nullptr != pParent && nullptr != pDoc);
  if (nullptr != pParent)
  {
    pParent->installEventFilter(this);
  }

  m_pHighlighter = new CEditorHighlighter(pDoc);
  m_pSearchBar = new CEditorSearchBar(pParent);
  m_pSearchBar->Climb();
  m_pSearchBar->Resize();

  // reset things after closing search bar
  connect(m_pSearchBar, &CEditorSearchBar::SignalHidden,
          this, &CHighlightedSearchableTextEdit::SlotSearchAreaHidden);
  connect(m_pSearchBar, &CEditorSearchBar::SignalFilterChanged,
          this, &CHighlightedSearchableTextEdit::SlotSearchFilterChanged);
}

//----------------------------------------------------------------------------------------
//
void CHighlightedSearchableTextEdit::SetTextCursor(QTextCursor cursor)
{
  if (std::holds_alternative<QPointer<QTextEdit>>(m_pEditor))
  {
    std::get<QPointer<QTextEdit>>(m_pEditor)->setTextCursor(cursor);
  }
  else if (std::holds_alternative<QPointer<QPlainTextEdit>>(m_pEditor))
  {
    std::get<QPointer<QPlainTextEdit>>(m_pEditor)->setTextCursor(cursor);
  }
}

//----------------------------------------------------------------------------------------
//
QTextCursor CHighlightedSearchableTextEdit::TextCursor() const
{
  if (std::holds_alternative<QPointer<QTextEdit>>(m_pEditor))
  {
    return std::get<QPointer<QTextEdit>>(m_pEditor)->textCursor();
  }
  else if (std::holds_alternative<QPointer<QPlainTextEdit>>(m_pEditor))
  {
    return std::get<QPointer<QPlainTextEdit>>(m_pEditor)->textCursor();
  }
  return QTextCursor();
}
