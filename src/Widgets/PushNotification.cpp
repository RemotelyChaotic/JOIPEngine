#include "PushNotification.h"

#include <QLabel>
#include <QPropertyAnimation>
#include <QTimer>
#include <QHBoxLayout>

#include <QPushButton>
#include "Widgets/SearchWidget.h"

CPushNotification::CPushNotification(const QString& sMsg,
                                     std::chrono::milliseconds displayTime,
                                     QWidget* pParent) :
  COverlayBase(INT_MAX, pParent),
  m_pPopInOutAnim(new QPropertyAnimation(this, "iYOffset", this)),
  m_pMsg(new QLabel(sMsg, this)),
  m_iYOffset(0)
{
  setObjectName(QStringLiteral("DownloadCounter"));
  setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred));

  m_pPopInOutAnim->setEasingCurve(QEasingCurve::Linear);
  m_pPopInOutAnim->setDuration(500);
  m_pPopInOutAnim->setStartValue(0);
  m_pPopInOutAnim->setEndValue(0);
  m_pPopInOutAnim->setLoopCount(1);

  connect(m_pPopInOutAnim, &QPropertyAnimation::finished,
          this, [this]() { if (!m_bShowCalled) { Hide(); deleteLater(); } });
  connect(m_pPopInOutAnim, &QPropertyAnimation::valueChanged,
          this, [this]() { repaint(); });

  QLayout* pLayout = new QHBoxLayout(this);
  m_pMsg->setText(sMsg);
  m_pMsg->setAlignment(Qt::AlignCenter);
  m_pMsg->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
  pLayout->addWidget(m_pMsg.data());
  setLayout(pLayout);

  QTimer* pTimer = new QTimer(this);
  connect(pTimer, &QTimer::timeout, this, &CPushNotification::Hide);
  pTimer->start(displayTime + std::chrono::milliseconds(500));
}

CPushNotification::~CPushNotification()
{

}

//----------------------------------------------------------------------------------------
//
void CPushNotification::Initialize()
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
  m_pPopInOutAnim->setStartValue(0);
  m_pPopInOutAnim->setEndValue(-height());
  m_pPopInOutAnim->start();
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
  m_pPopInOutAnim->setStartValue(-height());
  m_pPopInOutAnim->setEndValue(0);
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
