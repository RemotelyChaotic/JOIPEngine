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
  // variables
  static const QColor segmentColor(BRIGHT_PURPLE);
  static const QColor progressFront(DARK_PURPLE, 0);
  static const QColor progressBack(BRIGHT_PURPLE);
  static const QColor progressBubble(MEDIUM_PURPLE);

  qint32 iMinimalDimension = std::min(width(), height());

  QPainter painterDb(this);
  painterDb.setBackgroundMode(Qt::BGMode::TransparentMode);
  painterDb.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform |
                           QPainter::TextAntialiasing, true);

  //// draw background
  QRect cr = contentsRect();
  painterDb.fillRect(cr, Qt::transparent);

  // draw segments
  painterDb.save();
  painterDb.translate(width() / 2, height() / 2);
  painterDb.setPen(segmentColor);
  for (int i = 0; i < 12; ++i)
  {
    painterDb.drawLine(iMinimalDimension / 2 - c_iBorderWidth - c_iGroveWidth, 0, iMinimalDimension / 2 - c_iBorderWidth, 0);
    painterDb.rotate(30.0);
  }
  painterDb.restore();

  // draw "hand"
  painterDb.save();
  double currentPosition = 0.0;
  if (0 != m_pParent->m_iTimeMsMax)
  {
    qint32 iConeAngle = 0;
    if (m_pParent->m_bVisible)
    {
      currentPosition = 360 -
          360 * static_cast<double>(m_pParent->m_iTimeMsCurrent) /
          static_cast<double>(m_pParent->m_iTimeMsMax);
    }
    else
    {
      currentPosition = 360 + m_pParent->m_iUpdateCounter % 360;
    }
    iConeAngle = static_cast<qint32>(360 + 90 - currentPosition) % 360;
    QConicalGradient conGrad(QPointF(static_cast<double>(iMinimalDimension) / 2.0,
                                     static_cast<double>(iMinimalDimension) / 2.0),
                             iConeAngle);
    conGrad.setColorAt(1, progressFront);
    conGrad.setColorAt(0, progressBack);

    painterDb.setBrush(conGrad);
    painterDb.setPen(Qt::NoPen);
    if (m_pParent->m_bVisible)
    {
      painterDb.drawPie(QRect(c_iBorderWidth, c_iBorderWidth,
                              iMinimalDimension - c_iBorderWidth * 2,
                              height() - c_iBorderWidth * 2),
                      90 * 16, -static_cast<qint32>(currentPosition) * 16);
    }
    else
    {
      painterDb.drawPie(QRect(c_iBorderWidth, c_iBorderWidth,
                            iMinimalDimension - c_iBorderWidth * 2,
                            height() - c_iBorderWidth * 2),
                      -static_cast<qint32>(currentPosition - 90) * 16, -static_cast<qint32>(currentPosition) * 16);
    }
  }

  // draw overlay
  painterDb.setBrush(QColor(BLACK));
  painterDb.setPen(Qt::NoPen);
  painterDb.drawEllipse(QRect(c_iBorderWidth + c_iGroveWidth, c_iBorderWidth + c_iGroveWidth,
                              iMinimalDimension - (c_iBorderWidth + c_iGroveWidth) * 2,
                              iMinimalDimension - (c_iBorderWidth + c_iGroveWidth) * 2));

  painterDb.restore();

  // draw progress dot
  if (0 != m_pParent->m_iTimeMsMax)
  {
    painterDb.save();
    painterDb.translate(iMinimalDimension / 2, iMinimalDimension / 2);
    painterDb.setPen(progressBubble);
    painterDb.setBrush(progressBubble);
    painterDb.rotate(currentPosition - 90 - 2);
    painterDb.drawEllipse(QRect(iMinimalDimension / 2 - (c_iBorderWidth + c_iGroveWidth), 0, c_iGroveWidth, c_iGroveWidth));
    painterDb.restore();
  }
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

  connect(m_spSettings.get(), &CSettings::FontChanged,
          this, &CTimerWidget::SlotFontChanged, Qt::QueuedConnection);
}

CTimerWidget::~CTimerWidget()
{}

//----------------------------------------------------------------------------------------
//
void CTimerWidget::SetTimer(qint32 iTimeMs)
{
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
void CTimerWidget::Update()
{
  m_iUpdateCounter++;
  if (0 < m_iTimeMsCurrent)
  {
    m_iTimeMsCurrent -= m_iUpdateInterval;
    QTime time(0, 0, 0, 0);
    m_pTimeLabel->setText(m_bVisible ? time.addMSecs(m_iTimeMsCurrent).toString("mm:ss") : "?");
    m_pCanvas->repaint();
  }

  if (0 >= m_iTimeMsCurrent)
  {
    emit SignalTimerFinished();
  }
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
