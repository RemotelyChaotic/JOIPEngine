#ifndef SCRIPTHIGHLIGHTER_H
#define SCRIPTHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>
#include <vector>

class QTextDocument;

class CScriptHighlighter : public QSyntaxHighlighter
{
  Q_OBJECT

public:
  CScriptHighlighter(QTextDocument* pParent = nullptr);

protected:
  void highlightBlock(const QString& sText) override;

private:
  struct SHighlightingRule
  {
    QRegularExpression m_pattern;
    QTextCharFormat m_format;
  };
  std::vector<SHighlightingRule> m_vHighlightingRules;

  QRegularExpression m_commentStartExpression;
  QRegularExpression m_commentEndExpression;

  QTextCharFormat m_keywordFormat;
  QTextCharFormat m_singleLineCommentFormat;
  QTextCharFormat m_multiLineCommentFormat;
  QTextCharFormat m_quotationFormat;
  QTextCharFormat m_functionFormat;
  QTextCharFormat m_variableFormat;
};

#endif // SCRIPTHIGHLIGHTER_H
