#include "EditorHighlighter.h"


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
void CEditorHighlighter::SetSearchExpression(const QString& sExpresion)
{
  m_searchExpression = QRegularExpression(sExpresion);
  rehighlight(); // Restart the backlight
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

  if (m_searchExpression.isValid())
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
