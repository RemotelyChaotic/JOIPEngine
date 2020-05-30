#include "TimerWidget.h"

#include "Application.h"
#include "Constants.h"
#include "Settings.h"

#include <QLabel>
#include <QPainter>
#include <QResizeEvent>
#include <QTime>
#include <QtSvg>

namespace  {
  const qint32 c_iBorderWidth = 2;
  const qint32 c_iGroveWidth = 10;

  //--------------------------------------------------------------------------------------
  //
  void paintTimer(QPainter* pPainter, QColor primaryColor, QColor secondaryColor, QColor tertiaryColor,
                  qint32 iBorderWidth, qint32 iGroveWidth,
                  qint32 iWidth, qint32 iHeight, QRect contentsRect,
                  qint32 iTimeMsMax, qint32 iTimeMsCurrent, qint32 iUpdateCounter, bool bVisibleCounter)
  {
    // variables
    QColor progressFront = primaryColor;
    progressFront.setAlpha(0);

    qint32 iMinimalDimension = std::min(iWidth, iHeight);

    pPainter->setBackgroundMode(Qt::BGMode::TransparentMode);
    pPainter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform |
                             QPainter::TextAntialiasing, true);

    // draw background
    pPainter->fillRect(contentsRect, Qt::transparent);

    // draw segments
    pPainter->save();
    pPainter->translate(iWidth / 2, iHeight / 2);
    pPainter->setPen(tertiaryColor);
    for (int i = 0; i < 12; ++i)
    {
      pPainter->drawLine(iMinimalDimension / 2 - iBorderWidth - iGroveWidth, 0, iMinimalDimension / 2 - c_iBorderWidth, 0);
      pPainter->rotate(30.0);
    }
    pPainter->restore();

    // draw "hand"
    pPainter->save();
    double currentPosition = 0.0;
    if (0 != iTimeMsMax)
    {
      qint32 iConeAngle = 0;
      if (bVisibleCounter)
      {
        currentPosition = 360 -
            360 * static_cast<double>(iTimeMsCurrent) /
            static_cast<double>(iTimeMsMax);
      }
      else
      {
        currentPosition = 360 + iUpdateCounter % 360;
      }
      iConeAngle = static_cast<qint32>(360 + 90 - currentPosition) % 360;
      QConicalGradient conGrad(QPointF(static_cast<double>(iMinimalDimension) / 2.0,
                                       static_cast<double>(iMinimalDimension) / 2.0),
                               iConeAngle);
      conGrad.setColorAt(1, progressFront);
      conGrad.setColorAt(0, tertiaryColor);

      pPainter->setBrush(conGrad);
      pPainter->setPen(Qt::NoPen);
      if (bVisibleCounter)
      {
        pPainter->drawPie(QRect(iBorderWidth, iBorderWidth,
                                iMinimalDimension - iBorderWidth * 2,
                                iHeight - iBorderWidth * 2),
                        90 * 16, -static_cast<qint32>(currentPosition) * 16);
      }
      else
      {
        pPainter->drawPie(QRect(iBorderWidth, iBorderWidth,
                              iMinimalDimension - iBorderWidth * 2,
                              iHeight - iBorderWidth * 2),
                        -static_cast<qint32>(currentPosition - 90) * 16, -static_cast<qint32>(currentPosition) * 16);
      }
    }

    // draw overlay
    pPainter->setBrush(QColor(BLACK));
    pPainter->setPen(Qt::NoPen);
    pPainter->drawEllipse(QRect(iBorderWidth + iGroveWidth, iBorderWidth + iGroveWidth,
                                iMinimalDimension - (iBorderWidth + iGroveWidth) * 2,
                                iMinimalDimension - (iBorderWidth + iGroveWidth) * 2));

    pPainter->restore();

    // draw progress dot
    if (0 != iTimeMsMax)
    {
      pPainter->save();
      pPainter->translate(iMinimalDimension / 2, iMinimalDimension / 2);
      pPainter->setPen(secondaryColor);
      pPainter->setBrush(secondaryColor);
      pPainter->rotate(currentPosition - 90 - 2);
      pPainter->drawEllipse(QRect(iMinimalDimension / 2 - (iBorderWidth + iGroveWidth), 0, iGroveWidth, iGroveWidth));
      pPainter->restore();
    }
  }
}

//----------------------------------------------------------------------------------------
//
CTimerCanvasQml::CTimerCanvasQml(QQuickItem* pParent) :
  QQuickPaintedItem(pParent),
  m_primaryColor(Qt::white),
  m_secondaryColor(Qt::white),
  m_tertiaryColor(Qt::white),
  m_iTimeMsMax(0),
  m_iTimeMsCurrent(0),
  m_bVisibleCounter(true)
{
}

CTimerCanvasQml::~CTimerCanvasQml()
{

}

//----------------------------------------------------------------------------------------
//
void CTimerCanvasQml::paint(QPainter* pPainter)
{
  paintTimer(pPainter, m_primaryColor, m_secondaryColor, m_tertiaryColor,
             m_iBorderWidth, m_iGroveWidth,
             width(), height(), QRect({0,0}, textureSize()),
             m_iTimeMsMax, m_iTimeMsCurrent, m_iUpdateCounter, m_bVisibleCounter);
}

//----------------------------------------------------------------------------------------
//
void CTimerCanvasQml::SetPrimaryColor(const QColor& color)
{
  if (m_primaryColor != color)
  {
    m_primaryColor = color;
    emit primaryColorChanged();
  }
}

//----------------------------------------------------------------------------------------
//
const QColor& CTimerCanvasQml::PrimaryColor()
{
  return m_primaryColor;
}

//----------------------------------------------------------------------------------------
//
void CTimerCanvasQml::SetSecondaryColor(const QColor& color)
{
  if (m_secondaryColor != color)
  {
    m_secondaryColor = color;
    emit secondaryColorChanged();
  }
}

//----------------------------------------------------------------------------------------
//
const QColor& CTimerCanvasQml::SecondaryColor()
{
  return m_secondaryColor;
}

//----------------------------------------------------------------------------------------
//
void CTimerCanvasQml::SetTertiaryColor(const QColor& color)
{
  if (m_tertiaryColor != color)
  {
    m_tertiaryColor = color;
    emit tertiaryColorColorChanged();
  }
}

//----------------------------------------------------------------------------------------
//
const QColor& CTimerCanvasQml::TertiaryColor()
{
  return m_tertiaryColor;
}


//----------------------------------------------------------------------------------------
//
CTimerCanvas::CTimerCanvas(CTimerWidget* pParent) :
  QWidget(pParent),
  m_pParent(pParent)
{
}
CTimerCanvas::~CTimerCanvas()
{
}

//----------------------------------------------------------------------------------------
//
void CTimerCanvas::paintEvent(QPaintEvent* /*pEvent*/)
{
  QPainter painter(this);
  paintTimer(&painter, m_pParent->m_primaryColor, m_pParent->m_secondaryColor, m_pParent->m_tertiaryColor,
             c_iBorderWidth, c_iGroveWidth,
             width(), height(), contentsRect(),
             m_pParent->m_iTimeMsMax, m_pParent->m_iTimeMsCurrent, m_pParent->m_iUpdateCounter,
             m_pParent->m_bVisible);
}

//----------------------------------------------------------------------------------------
//
CTimerWidget::CTimerWidget(QWidget* pParent) :
  QWidget(pParent),
  m_spSettings(CApplication::Instance()->Settings()),
  m_pTimerBackGround(new QLabel("", this)),
  m_pCanvas(new CTimerCanvas(this)),
  m_pTimeLabel(new QLabel("00:00", this)),
  m_bgImage("://resources/img/TimerBg.svg"),
  m_doubleBuffer(128, 128),
  m_primaryColor(Qt::white),
  m_secondaryColor(Qt::white),
  m_tertiaryColor(Qt::white),
  m_iTimeMsMax(0),
  m_iTimeMsCurrent(0),
  m_iUpdateInterval(200),
  m_bVisible(true)
{
  m_pTimerBackGround->setFixedSize(width(), height());
  m_pTimerBackGround->setPixmap(m_bgImage);
  m_pTimerBackGround->setScaledContents(true);
  m_pTimerBackGround->setStyleSheet("background-color: transparent;");

  m_pCanvas->setFixedSize(width(), height());

  m_pTimeLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  SlotFontChanged();
  m_pTimeLabel->setStyleSheet("QLabel { background-color : black; color : " WHITE_QML "; }");

  connect(m_spSettings.get(), &CSettings::fontChanged,
          this, &CTimerWidget::SlotFontChanged, Qt::QueuedConnection);
}

CTimerWidget::~CTimerWidget()
{}

//----------------------------------------------------------------------------------------
//
void CTimerWidget::SetTimer(qint32 iTimeMs)
{
  m_lastUpdateTime = std::chrono::steady_clock::now();
  m_iTimeMsMax = iTimeMs;
  m_iTimeMsCurrent = iTimeMs;
  m_iUpdateCounter = 0;
  m_pCanvas->repaint();
}

//----------------------------------------------------------------------------------------
//
void CTimerWidget::SetTimerVisible(bool bVisible)
{
  m_bVisible = bVisible;
  m_pCanvas->repaint();
}

//----------------------------------------------------------------------------------------
//
void CTimerWidget::SetUpdateInterval(qint32 iTimeMs)
{
  m_iUpdateInterval = iTimeMs;
}

//----------------------------------------------------------------------------------------
//
void CTimerWidget::SetPrimaryColor(const QColor& color)
{
  if (m_primaryColor != color)
  {
    m_primaryColor = color;
  }
}

//----------------------------------------------------------------------------------------
//
const QColor& CTimerWidget::PrimaryColor()
{
  return m_primaryColor;
}

//----------------------------------------------------------------------------------------
//
void CTimerWidget::SetSecondaryColor(const QColor& color)
{
  if (m_secondaryColor != color)
  {
    m_secondaryColor = color;
  }
}

//----------------------------------------------------------------------------------------
//
const QColor& CTimerWidget::SecondaryColor()
{
  return m_secondaryColor;
}

//----------------------------------------------------------------------------------------
//
void CTimerWidget::SetTertiaryColor(const QColor& color)
{
  if (m_tertiaryColor != color)
  {
    m_tertiaryColor = color;
  }
}

//----------------------------------------------------------------------------------------
//
const QColor& CTimerWidget::TertiaryColor()
{
  return m_tertiaryColor;
}

//----------------------------------------------------------------------------------------
//
void CTimerWidget::Update()
{
  m_iUpdateCounter++;
  if (0 < m_iTimeMsCurrent)
  {
    auto timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::steady_clock::now() - m_lastUpdateTime);
    m_iTimeMsCurrent -= static_cast<qint32>(timeElapsed.count());
    QTime time(0, 0, 0, 0);
    m_pTimeLabel->setText(m_bVisible ? time.addMSecs(m_iTimeMsCurrent).toString("mm:ss") : "?");
    m_pCanvas->repaint();
  }

  if (0 >= m_iTimeMsCurrent)
  {
    emit SignalTimerFinished();
  }

  m_lastUpdateTime = std::chrono::steady_clock::now();
}

//----------------------------------------------------------------------------------------
//
void CTimerWidget::resizeEvent(QResizeEvent* pEvent)
{
  static qint32 iMarginText = 5;

  m_pTimerBackGround->setFixedSize(pEvent->size().width(), pEvent->size().height());
  m_pCanvas->setFixedSize(width(), height());

  m_pTimeLabel->setGeometry(c_iBorderWidth + c_iGroveWidth + iMarginText, pEvent->size().height() / 3,
                            pEvent->size().width(), pEvent->size().height());
  m_pTimeLabel->setFixedSize(pEvent->size().width() - (c_iBorderWidth + c_iGroveWidth + iMarginText) * 2,
                             pEvent->size().height() / 3);

  m_doubleBuffer = m_doubleBuffer.scaled(pEvent->size().width(), pEvent->size().height());
}

//----------------------------------------------------------------------------------------
//
void CTimerWidget::SlotFontChanged()
{
  QFont font = m_pTimeLabel->font();
  font.setPointSize(24);
  font.setBold(true);
  font.setFamily(m_spSettings->Font());
  m_pTimeLabel->setFont(font);
}
