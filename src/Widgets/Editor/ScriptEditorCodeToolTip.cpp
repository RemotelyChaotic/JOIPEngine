#include "ScriptEditorCodeToolTip.h"

#include <QApplication>
#include <QLabel>
#include <QToolTip>
#include <QTextDocument>

//----------------------------------------------------------------------------------------
//
void CScriptEditorCodeToolTip::showToolTip(const QPoint& pos, const QString& sString,
                                           QSyntaxHighlighter* pSyntaxHighLighter, QWidget* pW)
{
  showToolTip(pos, sString, pSyntaxHighLighter, pW, QRect());
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorCodeToolTip::showToolTip(const QPoint& pos, const QString& sString,
                                           QSyntaxHighlighter* pSyntaxHighLighter, QWidget* pW,
                                           const QRect& rect)
{
  showToolTip(pos, sString, pSyntaxHighLighter, pW, rect, -1);
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorCodeToolTip::showToolTip(const QPoint& pos, const QString& sString,
                                           QSyntaxHighlighter* pSyntaxHighLighter, QWidget* pW,
                                           const QRect& rect, qint32 iMsecShowTime)
{
  QToolTip::showText(pos, sString, pW, rect, iMsecShowTime);
  Q_UNUSED(pSyntaxHighLighter)
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorCodeToolTip::hideToolTip()
{
  showToolTip(QPoint(), QString(), nullptr);
}

//----------------------------------------------------------------------------------------
//
bool CScriptEditorCodeToolTip::isVisible()
{
  return QToolTip::isVisible();
}

//----------------------------------------------------------------------------------------
//
QPalette CScriptEditorCodeToolTip::palette()
{
  return QToolTip::palette();
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorCodeToolTip::setPalette(const QPalette& pal)
{
  QToolTip::setPalette(pal);
}

//----------------------------------------------------------------------------------------
//
QFont CScriptEditorCodeToolTip::font()
{
  return QApplication::font("QTipLabel");
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorCodeToolTip::setFont(const QFont& font)
{
  QApplication::setFont(font, "QTipLabel");
}
