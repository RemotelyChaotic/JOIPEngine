#include "ScriptHighlighter.h"

namespace {
  const QString asKeywordPatterns[] =
  {
    QStringLiteral("\\bbreak\\b"),
    QStringLiteral("\\bcase\\b"),
    QStringLiteral("\\bcatch\\b"),
    QStringLiteral("\\bcontinue\\b"),
    QStringLiteral("\\bdebugger\\b"),
    QStringLiteral("\\bdefault\\b"),
    QStringLiteral("\\bdelete\\b"),
    QStringLiteral("\\bdo\\b"),
    QStringLiteral("\\belse\\b"),
    QStringLiteral("\\bfinally\\b"),
    QStringLiteral("\\bfor\\b"),
    QStringLiteral("\\bfunction\\b"),
    QStringLiteral("\\bif\\b"),
    QStringLiteral("\\bin\\b"),
    QStringLiteral("\\binstanceof\\b"),
    QStringLiteral("\\bnew\\b"),
    QStringLiteral("\\breturn\\b"),
    QStringLiteral("\\bswitch\\b"),
    QStringLiteral("\\bthis\\b"),
    QStringLiteral("\\bthrow\\b"),
    QStringLiteral("\\btry\\b"),
    QStringLiteral("\\btypeof\\b"),
    QStringLiteral("\\bvar\\b"),
    QStringLiteral("\\bvoid\\b"),
    QStringLiteral("\\bwhile\\b"),
    QStringLiteral("\\bwith\\b"),
    QStringLiteral("\\blass\\b"),
    QStringLiteral("\\bconst\\b"),
    QStringLiteral("\\benum\\b"),
    QStringLiteral("\\bexport\\b"),
    QStringLiteral("\\bextends\\b"),
    QStringLiteral("\\bimport\\b"),
    QStringLiteral("\\bsuper\\b")
  };
}

//----------------------------------------------------------------------------------------
//
CScriptHighlighter::CScriptHighlighter(QTextDocument* pParent) :
  QSyntaxHighlighter(pParent)
{
  SHighlightingRule rule;

  m_keywordFormat.setForeground(QColor(255, 121, 198));
  m_keywordFormat.setFontWeight(QFont::Bold);

  for (const QString& sPattern : asKeywordPatterns)
  {
    rule.m_pattern = QRegularExpression(sPattern);
    rule.m_format = m_keywordFormat;
    m_vHighlightingRules.push_back(rule);
  }


  m_singleLineCommentFormat.setForeground(QColor(98, 114, 164));
  m_singleLineCommentFormat.setFontWeight(QFont::Bold);
  rule.m_pattern = QRegularExpression(QStringLiteral("//[^\n]*"));
  rule.m_format = m_singleLineCommentFormat;
  m_vHighlightingRules.push_back(rule);

  m_multiLineCommentFormat.setForeground(QColor(98, 114, 164));
  m_singleLineCommentFormat.setFontWeight(QFont::Bold);


  m_quotationFormat.setForeground(QColor(98, 114, 164));
  rule.m_pattern = QRegularExpression(QStringLiteral("\".*\""));
  rule.m_format = m_quotationFormat;
  m_vHighlightingRules.push_back(rule);


  m_functionFormat.setFontItalic(true);
  m_functionFormat.setForeground(QColor(80, 250, 123));
  rule.m_pattern = QRegularExpression(QStringLiteral("\\b[A-Za-z0-9_]+(?=\\()"));
  rule.m_format = m_functionFormat;
  m_vHighlightingRules.push_back(rule);


  m_variableFormat.setForeground(QColor(255, 91, 79));
  rule.m_pattern = QRegularExpression(QStringLiteral("\\bvar\\s[A-Za-z0-9_]\\b"));
  rule.m_format = m_functionFormat;


  m_commentStartExpression = QRegularExpression(QStringLiteral("/\\*"));
  m_commentEndExpression = QRegularExpression(QStringLiteral("\\*/"));
}

//----------------------------------------------------------------------------------------
//
void CScriptHighlighter::highlightBlock(const QString& sText)
{
  // generic rules
  for (const SHighlightingRule& rule : qAsConst(m_vHighlightingRules))
  {
    QRegularExpressionMatchIterator matchIterator = rule.m_pattern.globalMatch(sText);
    while (matchIterator.hasNext())
    {
      QRegularExpressionMatch match = matchIterator.next();
      setFormat(match.capturedStart(), match.capturedLength(), rule.m_format);
    }
  }

  // comments
  setCurrentBlockState(0);

  qint32 iStartIndex = 0;
  if (previousBlockState() != 1)
  {
    iStartIndex = sText.indexOf(m_commentStartExpression);
  }

  while (iStartIndex >= 0)
  {
    QRegularExpressionMatch match = m_commentEndExpression.match(sText, iStartIndex);
    qint32 iEndIndex = match.capturedStart();
    qint32 iCommentLength = 0;
    if (iEndIndex == -1)
    {
      setCurrentBlockState(1);
      iCommentLength = sText.length() - iStartIndex;
    } else
    {
      iCommentLength = iEndIndex - iStartIndex
                      + match.capturedLength();
    }
    setFormat(iStartIndex, iCommentLength, m_multiLineCommentFormat);
    iStartIndex = sText.indexOf(m_commentStartExpression, iStartIndex + iCommentLength);
  }
}
