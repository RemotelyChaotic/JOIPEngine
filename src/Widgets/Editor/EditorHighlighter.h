#ifndef SCRIPTHIGHLIGHTER_H
#define SCRIPTHIGHLIGHTER_H

#include <syntaxhighlighter.h>
#include <QTextCharFormat>
#include <QRegularExpression>
#include <vector>

class QTextDocument;

class CEditorHighlighter : public KSyntaxHighlighting::SyntaxHighlighter
{
  Q_OBJECT

public:
  CEditorHighlighter(QTextDocument* pParent = nullptr);
  ~CEditorHighlighter() override;

  void SetSearchColors(const QColor& background, const QColor& foreground);
  void SetSearchExpression(const QString& sExpresion);

protected:
  void highlightBlock(const QString& sText) override;

private:
  QRegularExpression m_searchExpression;
  QTextCharFormat    m_searchFormat;
};

#endif // SCRIPTHIGHLIGHTER_H
