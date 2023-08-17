#include "PushNotification.h"

#include <QLabel>
#include <QPropertyAnimation>
#include <QTimer>
#include <QHBoxLayout>

#include <QPushButton>
#include "Widgets/SearchWidget.h"

CPushNotification::CPushNotification(const QString& sMsg,
                                     std::optional<std::chrono::milliseconds> displayTime,
                                     bool bSingleShot,
                                     QWidget* pParent) :
  COverlayBase(INT_MAX, pParent),
  m_pPopInOutAnim(new QPropertyAnimation(this, "iYOffset", this)),
  m_pMsg(new QLabel(sMsg, this)),
  m_iYOffset(0)
{
  setObjectName(QStringLiteral("PushNotification"));
  setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred));

  m_pPopInOutAnim->setEasingCurve(QEasingCurve::Linear);
  m_pPopInOutAnim->setDuration(500);
  m_pPopInOutAnim->setStartValue(0);
  m_pPopInOutAnim->setEndValue(0);
  m_pPopInOutAnim->setLoopCount(1);

  connect(m_pPopInOutAnim, &QPropertyAnimation::valueChanged,
          this, [this]() { repaint(); });

  QLayout* pLayout = new QHBoxLayout(this);
  m_pMsg->setText(sMsg);
  m_pMsg->setAlignment(Qt::AlignCenter);
  m_pMsg->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
  pLayout->addWidget(m_pMsg.data());
  setLayout(pLayout);

  m_timer.setSingleShot(true);
  connect(&m_timer, &QTimer::timeout, this, [this](){ Hide(); });

  if (displayTime.has_value())
  {
    if (bSingleShot)
    {
      connect(&m_timer, &QTimer::timeout,
              this, [this]() { if (!m_bShowCalled) { deleteLater(); } });
    }
    m_timer.start(*displayTime + std::chrono::milliseconds(500));
  }
}

CPushNotification::~CPushNotification()
{

}

//----------------------------------------------------------------------------------------
//
void CPushNotification::Climb()
{
  ClimbToTop();
}

//----------------------------------------------------------------------------------------
//
void CPushNotification::Hide()
{
  m_bShowCalled = false;
  if (QAbstractAnimation::State::Stopped != m_pPopInOutAnim->state())
  {
    m_pPopInOutAnim->stop();
  }
  m_pPopInOutAnim->setStartValue(m_iTargetPosition.value_or(0));
  m_pPopInOutAnim->setEndValue(-height());
  m_pPopInOutAnim->start();
}

//----------------------------------------------------------------------------------------
//
void CPushNotification::Hide(std::chrono::milliseconds in)
{
  m_timer.start(in);
}

//----------------------------------------------------------------------------------------
//
void CPushNotification::Resize()
{
  if (nullptr != m_pTargetWidget)
  {
    resize(sizeHint());
    QSize target = m_pTargetWidget->size();
    qint32 iXOffset = (target.width() - sizeHint().width()) / 2;
    move(iXOffset, y());
  }
}

//----------------------------------------------------------------------------------------
//
void CPushNotification::Show()
{
  COverlayBase::Show();
  if (QAbstractAnimation::State::Stopped != m_pPopInOutAnim->state())
  {
    m_pPopInOutAnim->stop();
  }
  m_pPopInOutAnim->setStartValue(-height());
  m_pPopInOutAnim->setEndValue(m_iTargetPosition.value_or(0));
  m_pPopInOutAnim->start();
}

//----------------------------------------------------------------------------------------
//
void CPushNotification::Move(qint32 iX, qint32 iY)
{
  m_iYOffset = iY;
  move(iX, iY);
}

//----------------------------------------------------------------------------------------
//
void CPushNotification::SetMessage(const QString& sMsg)
{
  if (nullptr != m_pMsg)
  {
    m_pMsg->setText(sMsg);
    if (isVisible())
    {
      Resize();
    }
  }
}

//----------------------------------------------------------------------------------------
//
QString CPushNotification::Message() const
{
  if (nullptr != m_pMsg)
  {
    return m_pMsg->text();
  }
  return QString();
}

//----------------------------------------------------------------------------------------
//
void CPushNotification::SetTargetPosition(qint32 iYOffsetTarget)
{
  m_iTargetPosition = iYOffsetTarget;
}

//----------------------------------------------------------------------------------------
//
void CPushNotification::SetYOffset(qint32 iValue)
{
  Move(x(), iValue);
}

//----------------------------------------------------------------------------------------
//
qint32 CPushNotification::YOffset() const
{
  return m_iYOffset;
}
