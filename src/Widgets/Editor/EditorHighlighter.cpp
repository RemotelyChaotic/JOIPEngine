#include "EditorHighlighter.h"
#include "EditorCustomBlockUserData.h"

//----------------------------------------------------------------------------------------
//
CEditorHighlighter::CEditorHighlighter(QTextDocument* pParent) :
  KSyntaxHighlighting::SyntaxHighlighter(pParent),
  m_bSyntaxHighlightingEnabled(true)
{
}

CEditorHighlighter::~CEditorHighlighter()
{}

//----------------------------------------------------------------------------------------
//
void CEditorHighlighter::SetSearchColors(const QColor& background,
                                         const QColor& foreground)
{
  m_searchFormat.setBackground(background);
  m_searchFormat.setForeground(foreground);
}

//----------------------------------------------------------------------------------------
//
void CEditorHighlighter::SetActiveWordExpression(const QString& sWord)
{
  if (m_sWord != sWord)
  {
    m_sWord = sWord;
    if (!sWord.isEmpty())
    {
      m_activeWord = QRegularExpression("(^|\\W)(" + sWord + ")(\\W|$)");
    }
    else
    {
      m_activeWord = QRegularExpression();
    }
    rehighlight(); // Restart the backlight
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorHighlighter::SetSearchExpression(const QString& sExpresion)
{
  if (m_sSearch != sExpresion)
  {
    m_sSearch = sExpresion;
    m_searchExpression = QRegularExpression(sExpresion);
    rehighlight(); // Restart the backlight
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorHighlighter::SetSyntaxHighlightingEnabled(bool bEnabled)
{
  if (m_bSyntaxHighlightingEnabled != bEnabled)
  {
    m_bSyntaxHighlightingEnabled = bEnabled;
    rehighlight();
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorHighlighter::highlightBlock(const QString& sText)
{
  // default highlighting
  if (m_bSyntaxHighlightingEnabled)
  {
    KSyntaxHighlighting::SyntaxHighlighter::highlightBlock(sText);
  }

  CCustomBlockUserData* pUserData =
      dynamic_cast<CCustomBlockUserData*>(currentBlockUserData());
  if (nullptr == pUserData)
  {
    // first time we highlight this
    pUserData = new CCustomBlockUserData(currentBlockUserData());
    setCurrentBlockUserData(pUserData);
  }
  pUserData->m_vMatchedWordData.clear();

  if (m_activeWord.isValid() && !m_sWord.isEmpty())
  {
    // search text highlighting
    // Using a regular expression, we find all matches.
    QRegularExpressionMatchIterator matchIterator = m_activeWord.globalMatch(sText);
    while (matchIterator.hasNext())
    {
      // Highlight all matches
      QRegularExpressionMatch match = matchIterator.next();
      //const QString sMatch = match.captured(2);

      pUserData->m_vMatchedWordData.push_back(
          {match.capturedStart(2), match.capturedLength(2)});
    }
  }
  if (m_searchExpression.isValid() && !m_sSearch.isEmpty())
  {
    // search text highlighting
    // Using a regular expression, we find all matches.
    QRegularExpressionMatchIterator matchIterator = m_searchExpression.globalMatch(sText);
    while (matchIterator.hasNext())
    {
      // Highlight all matches
      QRegularExpressionMatch match = matchIterator.next();
      setFormat(match.capturedStart(), match.capturedLength(), m_searchFormat);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorHighlighter::applyFormat(int offset, int length, const KSyntaxHighlighting::Format &format)
{
  if (m_bSyntaxHighlightingEnabled)
  {
    KSyntaxHighlighting::SyntaxHighlighter::applyFormat(offset, length, format);
  }
}
