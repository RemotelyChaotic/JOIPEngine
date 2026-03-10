#ifndef CSCRIPTEDITORCODETOOLTIP_H
#define CSCRIPTEDITORCODETOOLTIP_H

#include <QObject>
#include <QSyntaxHighlighter>

class CScriptEditorCodeToolTip : public QObject
{
  CScriptEditorCodeToolTip() = delete;

public:
  static void showToolTip(const QPoint& pos, const QString& sString, QSyntaxHighlighter* pSyntaxHighLighter, QWidget* pW = nullptr);
  static void showToolTip(const QPoint& pos, const QString& sString, QSyntaxHighlighter* pSyntaxHighLighter, QWidget* pW, const QRect& rect);
  static void showToolTip(const QPoint& pos, const QString& sString, QSyntaxHighlighter* pSyntaxHighLighter, QWidget* pW, const QRect& rect, qint32 iMsecShowTime);
  static void hideToolTip();

  static bool isVisible();

  static QPalette palette();
  static void setPalette(const QPalette&);
  static QFont font();
  static void setFont(const QFont&);
};

#endif // CSCRIPTEDITORCODETOOLTIP_H
