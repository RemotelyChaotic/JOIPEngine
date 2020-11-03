#include "ScriptHighlighter.h"


//----------------------------------------------------------------------------------------
//
CScriptHighlighter::CScriptHighlighter(QTextDocument* pParent) :
  KSyntaxHighlighting::SyntaxHighlighter(pParent)
{
}

CScriptHighlighter::~CScriptHighlighter()
{}

//----------------------------------------------------------------------------------------
//
void CScriptHighlighter::SetSearchColors(const QColor& background,
                                         const QColor& foreground)
{
  m_searchFormat.setBackground(background);
  m_searchFormat.setForeground(foreground);
}

//----------------------------------------------------------------------------------------
//
void CScriptHighlighter::SetSearchExpression(const QString& sExpresion)
{
  m_searchExpression = QRegularExpression(sExpresion);
  rehighlight(); // Restart the backlight
}

//----------------------------------------------------------------------------------------
//
void CScriptHighlighter::highlightBlock(const QString& sText)
{
  // default highlighting
  KSyntaxHighlighting::SyntaxHighlighter::highlightBlock(sText);

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
