#ifndef SCRIPTHIGHLIGHTER_H
#define SCRIPTHIGHLIGHTER_H

#include <syntaxhighlighter.h>
#include <QTextCharFormat>
#include <QRegularExpression>
#include <vector>

class QTextDocument;

class CScriptHighlighter : public KSyntaxHighlighting::SyntaxHighlighter
{
  Q_OBJECT

public:
  CScriptHighlighter(QTextDocument* pParent = nullptr);
  ~CScriptHighlighter() override;

  void SetSearchColors(const QColor& background, const QColor& foreground);
  void SetSearchExpression(const QString& sExpresion);

protected:
  void highlightBlock(const QString& sText) override;

private:
  QRegularExpression m_searchExpression;
  QTextCharFormat    m_searchFormat;
};

#endif // SCRIPTHIGHLIGHTER_H
