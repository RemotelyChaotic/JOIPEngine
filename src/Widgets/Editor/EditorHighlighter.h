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
  void SetSyntaxHighlightingEnabled(bool bEnabled);

protected:
  void highlightBlock(const QString& sText) override;
  void applyFormat(int offset, int length, const KSyntaxHighlighting::Format &format) override;

private:
  QRegularExpression m_searchExpression;
  QTextCharFormat    m_searchFormat;
  bool               m_bSyntaxHighlightingEnabled;
};

#endif // SCRIPTHIGHLIGHTER_H