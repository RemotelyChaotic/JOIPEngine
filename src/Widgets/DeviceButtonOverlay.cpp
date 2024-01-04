#include "DeviceButtonOverlay.h"

#include "Application.h"
#include "DeviceSelectorWidget.h"

#include "Systems/DeviceManager.h"
#include "Widgets/ProgressBar.h"
#include "Widgets/TitleLabel.h"

#include <QContextMenuEvent>
#include <QMenu>
#include <QVBoxLayout>
#include <QWidgetAction>

//----------------------------------------------------------------------------------------
//
CDeviceCounterOverlay::CDeviceCounterOverlay(QWidget* pParent) :
  COverlayBase(COverlayButton::c_iOverlayButtonZOrder + 1, pParent),
  m_wpDeviceManager(CApplication::Instance()->System<CDeviceManager>()),
  m_pDlCounter(new CTitleLabel("🔴", this))
{
  setAttribute(Qt::WA_TranslucentBackground);
  setAttribute(Qt::WA_TransparentForMouseEvents);
  setObjectName(QStringLiteral("DeviceCounter"));
  setFrameStyle(QFrame::Plain);
  setFrameShape(QFrame::NoFrame);
  setFrameShadow(QFrame::Plain);

  m_pDlCounter->SetDrawShadow(false);
  m_pDlCounter->SetFontSize(14);
  m_pDlCounter->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred));

  QLayout* pLayout = new QVBoxLayout(this);
  pLayout->setMargin(0);
  pLayout->setContentsMargins(0, 0, 0, 0);
  pLayout->addWidget(m_pDlCounter.data());
  setLayout(pLayout);

  if (auto spDeviceManager = m_wpDeviceManager.lock())
  {
    connect(spDeviceManager.get(), &CDeviceManager::SignalConnected,
            this, &CDeviceCounterOverlay::SlotConnected);
    connect(spDeviceManager.get(), &CDeviceManager::SignalDeviceCountChanged,
            this, &CDeviceCounterOverlay::SlotDeviceCountChanged);
    connect(spDeviceManager.get(), &CDeviceManager::SignalDisconnected,
            this, &CDeviceCounterOverlay::SlotDisconnected);

    if (spDeviceManager->IsConnected())
    {
      SlotConnected();
    }
  }
}
CDeviceCounterOverlay::~CDeviceCounterOverlay() = default;

//----------------------------------------------------------------------------------------
//
void CDeviceCounterOverlay::Initialize()
{

}

//----------------------------------------------------------------------------------------
//
QPointer<CTitleLabel> CDeviceCounterOverlay::Counter()
{
  return m_pDlCounter;
}

//----------------------------------------------------------------------------------------
//
void CDeviceCounterOverlay::Climb()
{
  ClimbToTop();
  Resize();
}

//----------------------------------------------------------------------------------------
//
void CDeviceCounterOverlay::Resize()
{
  resize(m_pDlCounter->SuggestedSize() + QSize(10, 10));
}

//----------------------------------------------------------------------------------------
//
void CDeviceCounterOverlay::SlotConnected()
{
  if (auto spDeviceManager = m_wpDeviceManager.lock())
  {
    m_pDlCounter->setText("🟢 " + QString::number(spDeviceManager->DeviceNames().size()));
    m_pDlCounter->Invalidate();
  }
}

//----------------------------------------------------------------------------------------
//
void CDeviceCounterOverlay::SlotDeviceCountChanged()
{
  if (auto spDeviceManager = m_wpDeviceManager.lock())
  {
    m_pDlCounter->setText("🟢 " +QString::number(spDeviceManager->DeviceNames().size()));
    m_pDlCounter->Invalidate();
  }
}

//----------------------------------------------------------------------------------------
//
void CDeviceCounterOverlay::SlotDisconnected()
{
  m_pDlCounter->setText("🔴");
  m_pDlCounter->Invalidate();
}

//----------------------------------------------------------------------------------------
//
CDeviceButtonOverlay::CDeviceButtonOverlay(QWidget* pParent) :
  COverlayButton(QStringLiteral("DeviceButtonOverlay"),
                 QStringLiteral("DeviceButton"),
                 QStringLiteral("Open Devices"),
                 QStringLiteral("Devices"), pParent),
  m_wpDeviceManager(CApplication::Instance()->System<CDeviceManager>()),
  m_pProgressBar(new CProgressBar(this)),
  m_pCounterOverlay(new CDeviceCounterOverlay(pParent))
{
  m_pProgressBar->raise();
  m_pProgressBar->setAttribute(Qt::WA_TransparentForMouseEvents);
  m_pProgressBar->SetRange(0, 100);
  m_pProgressBar->setValue(0);
  m_pProgressBar->setAlignment(Qt::AlignCenter);
  m_pProgressBar->setTextVisible(false);
  m_pProgressBar->hide();

  m_pCounterOverlay->Climb();
  m_pCounterOverlay->Show();

  if (auto spDeviceManager = m_wpDeviceManager.lock())
  {
    connect(spDeviceManager.get(), &CDeviceManager::SignalStartScanning,
            this, &CDeviceButtonOverlay::SlotStartScanning);
    connect(spDeviceManager.get(), &CDeviceManager::SignalStopScanning,
            this, &CDeviceButtonOverlay::SlotStopScanning);

    if (spDeviceManager->IsScanning())
    {
      SlotStartScanning();
    }
  }

  connect(this, &CDeviceButtonOverlay::SignalButtonClicked,
          this, [this](){
    OpenContextMenuAt({width(), height()},
                      parentWidget()->mapToGlobal(geometry().bottomLeft()));
  });
}

CDeviceButtonOverlay::~CDeviceButtonOverlay() = default;

//----------------------------------------------------------------------------------------
//
void CDeviceButtonOverlay::Hide()
{
  COverlayButton::Hide();
  m_pCounterOverlay->hide();
}

//----------------------------------------------------------------------------------------
//
void CDeviceButtonOverlay::Resize()
{
  QPoint pos{50, 110};
  move(pos);

  m_pProgressBar->move(0, 0);
  m_pProgressBar->resize(sizeHint());

  m_pCounterOverlay->Resize();
  m_pCounterOverlay->move(pos.x() - m_pCounterOverlay->size().width() / 2,
                          pos.y() - m_pCounterOverlay->size().height() / 2);
}

//----------------------------------------------------------------------------------------
//
void CDeviceButtonOverlay::Show()
{
  COverlayButton::Show();
}

//----------------------------------------------------------------------------------------
//
void CDeviceButtonOverlay::contextMenuEvent(QContextMenuEvent* pEvent)
{
  OpenContextMenuAt(pEvent->pos(), pEvent->globalPos());
}

//----------------------------------------------------------------------------------------
//
void CDeviceButtonOverlay::SlotStartScanning()
{
  m_pProgressBar->SetRange(0, 0);
  m_pProgressBar->setValue(0);
  m_pProgressBar->show();
}

//----------------------------------------------------------------------------------------
//
void CDeviceButtonOverlay::SlotStopScanning()
{
  m_pProgressBar->hide();
}

//----------------------------------------------------------------------------------------
//
void CDeviceButtonOverlay::OpenContextMenuAt(const QPoint& localPoint,
                                             const QPoint& createPoint)
{
  QMenu modelMenu;
  auto* pWidget = new CDeviceSelectorWidget(&modelMenu);
  auto* pAction = new QWidgetAction(&modelMenu);
  pAction->setDefaultWidget(pWidget);
  modelMenu.addAction(pAction);
  modelMenu.exec(createPoint);
}
