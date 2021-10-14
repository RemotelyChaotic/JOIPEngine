#include "ProgressBar.h"

#include <QPropertyAnimation>
#include <QStyleOption>
#include <QStylePainter>

namespace
{
  const double c_dDegToRad  =  0.01745329251994; // PI/180
}

//----------------------------------------------------------------------------------------
//
void PaintProgress(QPainter* pPainter, QColor primaryColor, QColor secondaryColor, QColor tertiaryColor,
                   qint32 iBorderWidth, qint32 iGroveWidth,
                   qint32 iWidth, qint32 iHeight, QRect contentsRect,
                   qint32 iTimeMsMax, qint32 iTimeMsCurrent, qint32 iUpdateCounter,
                   bool bVisibleCounter, bool bDrawDecoration)
{
  // variables
  QColor progressFront = primaryColor;
  progressFront.setAlpha(0);

  const bool bLongerThanHigh = iWidth > iHeight;
  const qint32 iMinimalDimension = std::min(iWidth, iHeight);
  const QSize offset{!bLongerThanHigh ? 0 : (iWidth - iMinimalDimension) / 2,
                      bLongerThanHigh ? 0 : (iHeight - iMinimalDimension) / 2};

  const double dOuterDim = iMinimalDimension - iBorderWidth * 2;
  const double dInnerDim = iMinimalDimension - iBorderWidth * 2 - iGroveWidth * 2;

  pPainter->setBackgroundMode(Qt::BGMode::TransparentMode);
  pPainter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform |
                           QPainter::TextAntialiasing, true);

  // draw background
  pPainter->fillRect(contentsRect, Qt::transparent);

  // draw segments
  pPainter->save();
  if (bDrawDecoration)
  {
    pPainter->translate(iWidth / 2, iHeight / 2);
    pPainter->setPen(tertiaryColor);
    for (int i = 0; i < 12; ++i)
    {
      pPainter->drawLine(iMinimalDimension / 2 - iBorderWidth - iGroveWidth, 0,
                         iMinimalDimension / 2 - iBorderWidth, 0);
      pPainter->rotate(30.0);
    }
  }
  pPainter->restore();

  // draw "hand"
  pPainter->save();
  pPainter->translate(offset.width(), offset.height());
  {
    const QPointF center(static_cast<double>(iMinimalDimension) / 2.0,
                         static_cast<double>(iMinimalDimension) / 2.0);
    const QRectF rOuter(iBorderWidth, iBorderWidth,
                        dOuterDim, dOuterDim);
    const QRectF rInner(iBorderWidth + iGroveWidth, iBorderWidth + iGroveWidth,
                        dInnerDim, dInnerDim);
    const double dInnerRadius = static_cast<double>(rInner.width()) / 2;
    // top-midle
    QPointF startIn;

    double dStartAngle = 0;
    double dSpanAngle = 0;
    double dConAngle = 0;
    double dConTailPoint = 0;
    if (bVisibleCounter)
    {
      dStartAngle = 90;
      const double dX = center.x() + dInnerRadius * std::cos(dStartAngle * c_dDegToRad);
      const double dY = center.y() - dInnerRadius * std::sin(dStartAngle * c_dDegToRad);
      startIn = QPointF(dX, dY);
      dSpanAngle = 360 * -static_cast<double>(iTimeMsCurrent) / iTimeMsMax;
      dConAngle = dStartAngle+dSpanAngle;
      dConTailPoint = 1.0;
    }
    else
    {
      dStartAngle = 90 - static_cast<double>(iUpdateCounter);
      const double dX = center.x() + dInnerRadius * std::cos(dStartAngle * c_dDegToRad);
      const double dY = center.y() - dInnerRadius * std::sin(dStartAngle * c_dDegToRad);
      startIn = QPointF(dX, dY);
      dSpanAngle = 90;
      dConAngle = dStartAngle;
      dConTailPoint = 0.25;
    }

    QConicalGradient conGrad(center, dConAngle);
    conGrad.setColorAt(dConTailPoint, progressFront);
    conGrad.setColorAt(0, tertiaryColor);
    pPainter->setBrush(conGrad);
    pPainter->setPen(Qt::NoPen);

    QPainterPath pathRing;
    pathRing.moveTo(startIn);
    pathRing.arcTo(rOuter, dStartAngle, dSpanAngle);
    pathRing.arcTo(rInner, dStartAngle + dSpanAngle, -dSpanAngle);
    pathRing.closeSubpath();
    pPainter->fillPath(pathRing, conGrad);
  }
  pPainter->restore();

  // draw progress dot
  pPainter->save();
  {
    double dCurrentPosition = 0.0;
    if (bVisibleCounter)
    {
      dCurrentPosition = 360 - 360 * iTimeMsCurrent / iTimeMsMax;
    }
    else
    {
      dCurrentPosition = -(360 + iUpdateCounter) % 360;
    }
    pPainter->translate(offset.width() + iMinimalDimension / 2,
                        offset.height() + iMinimalDimension / 2);
    pPainter->setPen(secondaryColor);
    pPainter->setBrush(secondaryColor);
    pPainter->rotate(-dCurrentPosition - 2);
    pPainter->drawEllipse(0, -dInnerDim / 2.0 -static_cast<double>(iGroveWidth),
                          iGroveWidth, iGroveWidth);
  }
  pPainter->restore();
}

//----------------------------------------------------------------------------------------
//
qint32 CProgressBar::c_iBorderWidth = 2;
qint32 CProgressBar::c_iGroveWidth = 10;

//----------------------------------------------------------------------------------------
//
CProgressBar::CProgressBar(QWidget* pParent) :
  QProgressBar(pParent),
  m_pIdleAnim(new QPropertyAnimation(this, "iUpdateCounter", this)),
  m_primaryColor(Qt::white),
  m_secondaryColor(Qt::white),
  m_tertiaryColor(Qt::white),
  m_iUpdateCounter(0),
  m_bAnimationRunning(false)
{
  setFormat(QString("%p"));

  setAttribute(Qt::WA_TranslucentBackground);

  m_pIdleAnim->setEasingCurve(QEasingCurve::Linear);
  m_pIdleAnim->setDuration(3600);
  m_pIdleAnim->setStartValue(0);
  m_pIdleAnim->setEndValue(360);
  m_pIdleAnim->setLoopCount(-1);

  connect(m_pIdleAnim, &QPropertyAnimation::valueChanged,
          this, &CProgressBar::SlotAnimationUpdate);

  // test progress bar graphics
  //auto test = new QPropertyAnimation(this, "value", this);
  //test->setEasingCurve(QEasingCurve::Linear);
  //test->setDuration(3600);
  //test->setStartValue(0);
  //test->setEndValue(100);
  //test->setLoopCount(-1);
  //test->start();
}
CProgressBar::~CProgressBar()
{
}

//----------------------------------------------------------------------------------------
//
void CProgressBar::SetPrimaryColor(const QColor& color)
{
  if (m_primaryColor != color)
  {
    m_primaryColor = color;
    emit primaryColorChanged();
  }
}

//----------------------------------------------------------------------------------------
//
const QColor& CProgressBar::PrimaryColor()
{
  return m_primaryColor;
}

//----------------------------------------------------------------------------------------
//
void CProgressBar::SetSecondaryColor(const QColor& color)
{
  if (m_secondaryColor != color)
  {
    m_secondaryColor = color;
    emit secondaryColorChanged();
  }
}

//----------------------------------------------------------------------------------------
//
const QColor& CProgressBar::SecondaryColor()
{
  return m_secondaryColor;
}

//----------------------------------------------------------------------------------------
//
void CProgressBar::SetTertiaryColor(const QColor& color)
{
  if (m_tertiaryColor != color)
  {
    m_tertiaryColor = color;
    emit tertiaryColorColorChanged();
  }
}

//----------------------------------------------------------------------------------------
//
const QColor& CProgressBar::TertiaryColor()
{
  return m_tertiaryColor;
}

//----------------------------------------------------------------------------------------
//
void CProgressBar::SetRange(qint32 iMinimum, qint32 iMaximum)
{
  if (0 == iMinimum && 0 == iMaximum)
  {
    if (!m_bAnimationRunning)
    {
      m_pIdleAnim->start();
      m_bAnimationRunning = true;
    }
  }
  else
  {
    if (m_bAnimationRunning)
    {
      m_pIdleAnim->stop();
      m_bAnimationRunning = false;
    }
  }
  setRange(iMinimum, iMaximum);
}

//----------------------------------------------------------------------------------------
//
void CProgressBar::SlotAnimationUpdate()
{
  repaint();
}

#include <QDebug>
//----------------------------------------------------------------------------------------
//
void CProgressBar::paintEvent(QPaintEvent*)
{
  QStylePainter paint(this);

  QStyleOptionProgressBar opt;
  initStyleOption(&opt);
  QRect rect({0,0}, size());

  paint.save();
  if (m_bDrawDecoration)
  {
    paint.setBackgroundMode(Qt::BGMode::TransparentMode);
    paint.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform |
                         QPainter::TextAntialiasing, true);

    // style-aware colors taken from from qdrawutil.cpp qDrawShadePanel
    const QBrush* fill = &opt.palette.brush(QPalette::Window);
    QColor grove = fill->color();

    const bool bLongerThanHigh = width() > height();
    const qint32 iMinimalDimension = std::min(width(), height());
    const QSize offset{!bLongerThanHigh ? 0 : (width() - iMinimalDimension) / 2,
                        bLongerThanHigh ? 0 : (height() - iMinimalDimension) / 2};

    paint.translate(offset.width(), offset.height());

    const double dInnerDim = iMinimalDimension - c_iBorderWidth * 4 - c_iGroveWidth * 2;
    const QRectF rOuter(0, 0, iMinimalDimension, iMinimalDimension);
    const QRectF rInner(c_iBorderWidth*2 + c_iGroveWidth, c_iBorderWidth*2 + c_iGroveWidth,
                        dInnerDim, dInnerDim);

    QPainterPath pathRing;
    paint.setPen(QPen(Qt::gray, c_iBorderWidth));
    pathRing.addEllipse(rOuter);
    pathRing.addEllipse(rInner);
    paint.drawPath(pathRing);

    const double dOuterDim2 = iMinimalDimension - c_iBorderWidth * 2;
    const double dInnerDim2 = iMinimalDimension - c_iBorderWidth * 2 - c_iGroveWidth * 2;
    const QRectF rOuter2(c_iBorderWidth, c_iBorderWidth,
                         dOuterDim2, dOuterDim2);
    const QRectF rInner2(c_iBorderWidth + c_iGroveWidth, c_iBorderWidth + c_iGroveWidth,
                         dInnerDim2, dInnerDim2);

    QPainterPath pathRing2;
    pathRing2.addEllipse(rOuter2);
    pathRing2.addEllipse(rInner2);
    paint.fillPath(pathRing2, grove);
  }
  paint.restore();

  PaintProgress(&paint, m_primaryColor, m_secondaryColor, m_tertiaryColor,
                c_iBorderWidth, c_iGroveWidth,
                width(), height(), rect,
                maximum()-minimum(), value()-minimum(), m_iUpdateCounter,
                !m_bAnimationRunning, m_bDrawDecoration);

  QStyleOptionProgressBar subopt = opt;
  if (isTextVisible())
  {
    subopt.rect = style()->subElementRect(QStyle::SE_ProgressBarLabel, &opt, this);
    paint.drawControl(QStyle::CE_ProgressBarLabel, subopt);
  }
  // debug
  //paint.drawControl(QStyle::CE_ProgressBar, opt);
}
