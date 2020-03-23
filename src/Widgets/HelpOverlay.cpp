#include "HelpOverlay.h"
#include "Application.h"
#include "ui_HelpOverlay.h"
#include "Systems/HelpFactory.h"

#include <QCursor>
#include <QDateTime>
#include <QDebug>
#include <QDesktopWidget>
#include <QPainter>
#include <QPushButton>
#include <QPropertyAnimation>
#include <QVBoxLayout>
#include <QTextBrowser>

namespace  {
  const qint32 c_iOffsetOfHelpWindow = 100;
  const qint32 c_iAnimationTime = 1500;
  const qint32 c_iUpdateInterval = 5;

  const QColor c_backgroundColor = QColor(30, 30, 30, 200);

  //--------------------------------------------------------------------------------------
  //
  void FindWidgetsForHelp(QWidget* pRootToSearch, std::vector<QPointer<QWidget>>& vpWidgetMap)
  {
    if (nullptr != pRootToSearch)
    {
      // get children
      auto vpChildren = pRootToSearch->children();
      for (auto pChild : qAsConst(vpChildren))
      {
        QWidget* pWidget = dynamic_cast<QWidget*>(pChild);
        if (nullptr != pWidget)
        {
          // only search for visible widgets
          if (pWidget->isVisible())
          {
            if (pWidget->property(helpOverlay::c_sHelpPagePropertyName).isValid())
            {
              vpWidgetMap.push_back(pWidget);
            }
            // search it's children
            FindWidgetsForHelp(pWidget, vpWidgetMap);
          }
        }
      }
    }
  }
}

const char* const helpOverlay::c_sHelpPagePropertyName = "HelpPageId";
QPointer<CHelpOverlay> CHelpOverlay::m_pHelpOverlay = nullptr;


//----------------------------------------------------------------------------------------
//
CHelpOverlayBackGround::CHelpOverlayBackGround(QWidget* pParent) :
  QWidget(pParent),
  m_bOpened(false),
  m_circleOrigin(0, 0),
  m_iCircleRadius(0),
  m_shownIconImages(),
  m_cursor(-1, -1),
  m_updateTimer()
{
  m_pCircleAnimation = new QPropertyAnimation(this, "circleRadius");
  m_pCircleAnimation->setDuration(c_iAnimationTime);
  m_pCircleAnimation->setEasingCurve(QEasingCurve::InQuart);
  connect(m_pCircleAnimation.data(), &QPropertyAnimation::finished,
          this, &CHelpOverlayBackGround::SlotCircleAnimationFinished, Qt::DirectConnection);
  connect(m_pCircleAnimation.data(), &QPropertyAnimation::finished,
          this, &CHelpOverlayBackGround::SignalCircleAnimationFinished, Qt::DirectConnection);

  m_updateTimer.setInterval(c_iUpdateInterval);
  m_updateTimer.setSingleShot(false);
  connect(&m_updateTimer, &QTimer::timeout,
          this, &CHelpOverlayBackGround::SlotUpdate, Qt::DirectConnection);

  m_circleOrigin = QPoint(width() / 2, height() / 2);
  setMouseTracking(true);
}
CHelpOverlayBackGround::~CHelpOverlayBackGround()
{
  Reset();
}

//----------------------------------------------------------------------------------------
//
void CHelpOverlayBackGround::Reset()
{
  m_pCircleAnimation->stop();
  m_updateTimer.stop();
  m_shownIconImages.clear();
  m_iCircleRadius = 0;
  m_bOpened = false;
}

//----------------------------------------------------------------------------------------
//
void CHelpOverlayBackGround::StartAnimation(const QPoint& origin, qint32 iEndValue)
{
  m_circleOrigin = origin;
  m_pCircleAnimation->setStartValue(m_iCircleRadius);
  m_pCircleAnimation->setEndValue(iEndValue);
  m_pCircleAnimation->start();
  m_updateTimer.start();
}

//----------------------------------------------------------------------------------------
//
void CHelpOverlayBackGround::SlotCircleAnimationFinished()
{
  // do a last update
  SlotUpdate();

  m_pCircleAnimation->stop();
  m_updateTimer.stop();
  m_bOpened = m_iCircleRadius != 0;
  if (!m_bOpened)
  {
    m_shownIconImages.clear();
  }

  MousePositionCheck();
}

//----------------------------------------------------------------------------------------
//
void CHelpOverlayBackGround::SlotUpdate()
{
  // get "pinged" widgets
  QVector2D center(m_circleOrigin);
  auto pHelpwidget = CHelpOverlay::Instance();
  if (nullptr != pHelpwidget)
  {
    for (auto pWidget : pHelpwidget->m_vpHelpWidgets)
    {
      QString sKey = pWidget->property(helpOverlay::c_sHelpPagePropertyName).toString();
      auto it = m_shownIconImages.find(sKey);
      QRect widgetGeometry = pWidget->geometry();
      widgetGeometry.moveTopLeft(pWidget->parentWidget()->mapToGlobal(widgetGeometry.topLeft()));
      float fCircleRadius = static_cast<float>(m_iCircleRadius);
      QVector2D tl(widgetGeometry.topLeft());
      QVector2D tr(widgetGeometry.topRight());
      QVector2D bl(widgetGeometry.bottomLeft());
      QVector2D br(widgetGeometry.bottomRight());
      if (center.distanceToPoint(tl) < fCircleRadius &&
          center.distanceToPoint(tr) < fCircleRadius &&
          center.distanceToPoint(bl) < fCircleRadius &&
          center.distanceToPoint(br) < fCircleRadius)
      {
        if (m_shownIconImages.end() == it)
        {
          m_shownIconImages.insert({sKey,
                                    { QRect(tl.toPoint(), br.toPoint()),
                                      pWidget->grab().toImage()}});
        }
      }
      else
      {
        if (m_shownIconImages.end() != it)
        {
          m_shownIconImages.erase(it);
        }
      }
    }
  }

  // repaint
  repaint();
}

//----------------------------------------------------------------------------------------
//
void CHelpOverlayBackGround::mouseMoveEvent(QMouseEvent* pEvt)
{
  if (nullptr != pEvt)
  {
    m_cursor = pEvt->globalPos();
    MousePositionCheck();
  }
}

//----------------------------------------------------------------------------------------
//
void CHelpOverlayBackGround::paintEvent(QPaintEvent* pEvt)
{
  Q_UNUSED(pEvt)

  QPainter painter(this);
  painter.setBackgroundMode(Qt::BGMode::TransparentMode);
  painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform |
                         QPainter::TextAntialiasing, true);

  // draw background
  painter.save();
  painter.setBrush(c_backgroundColor);
  painter.setPen(Qt::NoPen);
  painter.drawEllipse(m_circleOrigin, m_iCircleRadius, m_iCircleRadius);
  painter.restore();

  // draw widgets on top with border to highlight them
  QStyleOptionFrame option;
  QPalette originalpalette = option.palette;
  option.initFrom(this);
  option.state = QStyle::State_Sunken;
  option.frameShape = QFrame::HLine;
  for (auto it = m_shownIconImages.begin(); m_shownIconImages.end() != it; ++it)
  {
    painter.save();
    painter.drawImage(it->second.first, it->second.second, it->second.second.rect());
    option.rect = it->second.first;
    style()->drawPrimitive(QStyle::PE_Frame, &option, &painter, this);

    if (it->second.first.contains(m_cursor))
    {
      painter.save();
      QPainterPath path;
      double dRad =
          static_cast<double>(static_cast<qint32>(static_cast<double>
             (QDateTime::currentDateTime().toMSecsSinceEpoch() % 1000000) / 5.0) % 360) / 180.0 * 3.1415;
      double dMultiplyer = std::sin(dRad);
      path.addRect(it->second.first.adjusted(static_cast<qint32>(-20 -10 * dMultiplyer),
                                             static_cast<qint32>(-20 -10 * dMultiplyer),
                                             static_cast<qint32>(20 + 10 * dMultiplyer),
                                             static_cast<qint32>(20 + 10 * dMultiplyer)));

      QPen pen(QColor(255, 0, 0), 5, Qt::DashLine, Qt::FlatCap, Qt::MiterJoin);
      pen.setDashOffset(std::cos(dRad) * 10);
      painter.setPen(pen);
      painter.setBrush(Qt::transparent);
      painter.drawPath(path);
      painter.restore();
    }
    painter.restore();
  }
}

//----------------------------------------------------------------------------------------
//
void CHelpOverlayBackGround::MousePositionCheck()
{
   bool bStartTimer = false;
  for (auto it = m_shownIconImages.begin(); m_shownIconImages.end() != it; ++it)
  {
    if (it->second.first.contains(m_cursor))
    {
      bStartTimer = true;
      break;
    }
  }

  if (!bStartTimer)
  {
    m_cursor = QPoint(-1, -1);
  }

  if (bStartTimer)
  {
    m_updateTimer.start();
  }
  else if (m_pCircleAnimation->state() != QAbstractAnimation::State::Running)
  {
    m_updateTimer.stop();
    repaint();
  }
}

//----------------------------------------------------------------------------------------
//
CHelpOverlay::CHelpOverlay(QPointer<CHelpButtonOverlay> pHelpButton, QWidget* pParent) :
  COverlayBase(1, pParent),
  m_vpHelpWidgets(),
  m_spUi(std::make_unique<Ui::CHelpOverlay>()),
  m_wpHelpFactory(CApplication::Instance()->System<CHelpFactory>()),
  m_pHelpButton(pHelpButton)
{
  setAttribute(Qt::WA_TranslucentBackground);

  m_spUi->setupUi(this);
  m_pHelpOverlay = this;

  m_pBackground = new CHelpOverlayBackGround(this);
  m_pBackground->installEventFilter(this);
  m_spUi->pHtmlBrowserBox->hide();
  m_spUi->pHtmlBrowserBox->raise();

  connect(m_pBackground, &CHelpOverlayBackGround::SignalCircleAnimationFinished,
          this, &CHelpOverlay::SlotCircleAnimationFinished, Qt::DirectConnection);
}

CHelpOverlay::~CHelpOverlay()
{
  m_pHelpOverlay = nullptr;
}

//----------------------------------------------------------------------------------------
//
QPointer<CHelpOverlay> CHelpOverlay::Instance()
{
  return m_pHelpOverlay;
}

//----------------------------------------------------------------------------------------
//
void CHelpOverlay::Climb()
{
  ClimbToTop();
}

//----------------------------------------------------------------------------------------
//
void CHelpOverlay::Hide()
{
  COverlayBase::Hide();
  m_spUi->pHtmlBrowserBox->hide();
  m_pBackground->Reset();
  m_vpHelpWidgets.clear();
}

//----------------------------------------------------------------------------------------
//
void CHelpOverlay::Resize()
{
  COverlayBase::Resize();
  QSize parentSize = m_pTargetWidget->size();
  m_pBackground->resize(parentSize);
  m_spUi->pHtmlBrowserBox->resize(parentSize.width() - c_iOffsetOfHelpWindow * 2,
                                  parentSize.height() - c_iOffsetOfHelpWindow * 2);
  m_spUi->pHtmlBrowserBox->move(c_iOffsetOfHelpWindow, c_iOffsetOfHelpWindow);
}

//----------------------------------------------------------------------------------------
//
void CHelpOverlay::Show()
{
  COverlayBase::Show();
  m_spUi->pHtmlBrowserBox->hide();
  m_spUi->pHtmlBrowserBox->raise();
}

//----------------------------------------------------------------------------------------
//
void CHelpOverlay::Show(const QPoint& animationOrigin, QWidget* pRootToSearch)
{
  Show();

  QSize parentSize = m_pTargetWidget->size();
  m_pBackground->StartAnimation(animationOrigin,
        static_cast<qint32>(std::sqrt(parentSize.width() * parentSize.width() +
                                      parentSize.height() * parentSize.height())));

  m_vpHelpWidgets.clear();
  FindWidgetsForHelp(pRootToSearch, m_vpHelpWidgets);
}

//----------------------------------------------------------------------------------------
//
void CHelpOverlay::SlotCircleAnimationFinished()
{
  if (!m_pBackground->m_bOpened)
  {
    Hide();
  }
}

//----------------------------------------------------------------------------------------
//
void CHelpOverlay::on_pCloseButton_clicked()
{
  m_spUi->pHtmlBrowserBox->hide();
}

//----------------------------------------------------------------------------------------
//
bool CHelpOverlay::eventFilter(QObject* pObj, QEvent* pEvt)
{
  if (nullptr != pObj && nullptr != pEvt)
  {
    switch (pEvt->type())
    {
      case QEvent::MouseButtonPress:
      {
        QMouseEvent* pMouseEvt = static_cast<QMouseEvent*>(pEvt);
        // if help window is not open, hide overlay
        if (!m_spUi->pHtmlBrowserBox->isVisible())
        {
          // find clicked widget
          for (auto pWidget : m_vpHelpWidgets)
          {
            QVector2D center(m_pBackground->m_circleOrigin);
            QRect widgetGeometry = pWidget->geometry();
            widgetGeometry.moveTopLeft(pWidget->parentWidget()->mapToGlobal(widgetGeometry.topLeft()));
            float fCircleRadius = static_cast<float>(m_pBackground->m_iCircleRadius);
            QVector2D tl(widgetGeometry.topLeft());
            QVector2D tr(widgetGeometry.topRight());
            QVector2D bl(widgetGeometry.bottomLeft());
            QVector2D br(widgetGeometry.bottomRight());
            if (center.distanceToPoint(tl) < fCircleRadius &&
                center.distanceToPoint(tr) < fCircleRadius &&
                center.distanceToPoint(bl) < fCircleRadius &&
                center.distanceToPoint(br) < fCircleRadius &&
                widgetGeometry.contains(pMouseEvt->globalPos()))
            {
              // found -> open ui
              ShowHelp(pWidget->property(helpOverlay::c_sHelpPagePropertyName).toString());
              return COverlayBase::eventFilter(pObj, pEvt);
            }
          }

          m_pBackground->StartAnimation(m_pBackground->m_circleOrigin, 0);
        }
      } break;
    default: break;
    }
  }

  return COverlayBase::eventFilter(pObj, pEvt);
}

//----------------------------------------------------------------------------------------
//
void CHelpOverlay::ShowHelp(const QString sKey)
{
  if (auto spFactory = m_wpHelpFactory.lock())
  {
    m_spUi->pHtmlBrowser->setSource(QUrl::fromLocalFile(spFactory->GetHelp(sKey)));
    m_spUi->pHtmlBrowserBox->show();
  }
}

//----------------------------------------------------------------------------------------
//
CHelpButtonOverlay::CHelpButtonOverlay(QWidget* pParent) :
  COverlayBase(0, pParent)
{
  setObjectName(QStringLiteral("HelpButtonOverlay"));
  QSizePolicy sizePolicyThis(QSizePolicy::Preferred, QSizePolicy::Fixed);
  sizePolicyThis.setHorizontalStretch(0);
  sizePolicyThis.setVerticalStretch(0);
  sizePolicyThis.setHeightForWidth(sizePolicy().hasHeightForWidth());
  setSizePolicy(sizePolicyThis);
  setMinimumSize(QSize(0, 48));
  setMaximumSize(QSize(16777215, 48));
  setAttribute(Qt::WA_TranslucentBackground);

  QVBoxLayout* pLayout = new QVBoxLayout(this);
  pLayout->setSpacing(0);
  pLayout->setContentsMargins(0, 0, 0, 0);

  m_pButton = new QPushButton(this);
  m_pButton->setObjectName(QStringLiteral("HelpButton"));
  QSizePolicy sizePolicy2(QSizePolicy::Fixed, QSizePolicy::Fixed);
  sizePolicy2.setHorizontalStretch(48);
  sizePolicy2.setVerticalStretch(48);
  sizePolicy2.setHeightForWidth(m_pButton->sizePolicy().hasHeightForWidth());
  m_pButton->setSizePolicy(sizePolicy2);
  m_pButton->setMinimumSize(QSize(48, 48));
  m_pButton->setMaximumSize(QSize(48, 48));
  m_pButton->setIconSize(QSize(48, 48));

  pLayout->addWidget(m_pButton);

  retranslateUi(this);
}

//----------------------------------------------------------------------------------------
//
CHelpButtonOverlay::~CHelpButtonOverlay()
{

}

//----------------------------------------------------------------------------------------
//
void CHelpButtonOverlay::Initialize()
{
  connect(m_pButton, &QPushButton::clicked,
          this, &CHelpButtonOverlay::SignalButtonClicked, Qt::UniqueConnection);
}

//----------------------------------------------------------------------------------------
//
void CHelpButtonOverlay::Climb()
{
  ClimbToTop();
}

//----------------------------------------------------------------------------------------
//
void CHelpButtonOverlay::Resize()
{
  QSize parentSize = m_pTargetWidget->size();
  move(parentSize.width() - 50 - width(), 50);
}

//----------------------------------------------------------------------------------------
//
void CHelpButtonOverlay::retranslateUi(QWidget* pHelpOverlay)
{
  Q_UNUSED(pHelpOverlay)
  m_pButton->setToolTip(QApplication::translate("CHelpButtonOverlay", "Open help interface", Q_NULLPTR));
}
