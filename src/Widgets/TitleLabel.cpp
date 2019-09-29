#include "TitleLabel.h"

#include <QFont>
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QProxyStyle>

namespace  {
  const double c_iOffsetBorder = 5.0;
  const double c_diagOffsetBorder = 5.0 / 1.41421356237;
}

class CTitleProxyStyle : public QProxyStyle
{
protected:
  virtual void drawItemText(QPainter* pPainter, const QRect &rect,
      int flags, const QPalette &pal, bool enabled,
      const QString &text, QPalette::ColorRole textRole) const
  {
    QProxyStyle::drawItemText(pPainter, rect.translated(static_cast<qint32>(c_iOffsetBorder), 0), flags, Qt::black, enabled, text, textRole);
    QProxyStyle::drawItemText(pPainter, rect.translated(-static_cast<qint32>(c_iOffsetBorder), 0), flags, Qt::black, enabled, text, textRole);
    QProxyStyle::drawItemText(pPainter, rect.translated(0, static_cast<qint32>(c_iOffsetBorder)), flags, Qt::black, enabled, text, textRole);
    QProxyStyle::drawItemText(pPainter, rect.translated(0, -static_cast<qint32>(c_iOffsetBorder)), flags, Qt::black, enabled, text, textRole);
    QProxyStyle::drawItemText(pPainter, rect.translated(-static_cast<qint32>(c_diagOffsetBorder), -static_cast<qint32>(c_diagOffsetBorder)), flags, Qt::black, enabled, text, textRole);
    QProxyStyle::drawItemText(pPainter, rect.translated(static_cast<qint32>(c_diagOffsetBorder), static_cast<qint32>(c_diagOffsetBorder)), flags, Qt::black, enabled, text, textRole);
    QProxyStyle::drawItemText(pPainter, rect.translated(static_cast<qint32>(c_diagOffsetBorder), -static_cast<qint32>(c_diagOffsetBorder)), flags, Qt::black, enabled, text, textRole);
    QProxyStyle::drawItemText(pPainter, rect.translated(-static_cast<qint32>(c_diagOffsetBorder), static_cast<qint32>(c_diagOffsetBorder)), flags, Qt::black, enabled, text, textRole);

    QProxyStyle::drawItemText(pPainter, rect.translated(0, 0), flags, pal, enabled, text, textRole);
  }
};

//----------------------------------------------------------------------------------------
//
CTitleLabel::CTitleLabel(QWidget* pParent) :
  QLabel(pParent)
{
  QFont thisFont = font();
  thisFont.setPixelSize(60);
  setFont(thisFont);
  setStyle(new CTitleProxyStyle());
  AddEffects();
}

CTitleLabel::CTitleLabel(QString sText, QWidget* pParent) :
  QLabel(sText, pParent)
{
  QFont thisFont = font();
  thisFont.setPixelSize(60);
  setFont(thisFont);
  setStyle(new CTitleProxyStyle());
  AddEffects();
}

//----------------------------------------------------------------------------------------
//
void CTitleLabel::AddEffects()
{
  QGraphicsDropShadowEffect* pShadow = new QGraphicsDropShadowEffect(this);
  pShadow->setBlurRadius(10);
  pShadow->setXOffset(5);
  pShadow->setYOffset(5);
  pShadow->setColor(Qt::black);
  setGraphicsEffect(pShadow);
}
