#include "OverlayBase.h"
#include "Application.h"
#include "MainWindow.h"
#include "Systems/OverlayManager.h"

COverlayBase::COverlayBase(qint32 iZOrder, QWidget* pParent) :
  QFrame(pParent, Qt::FramelessWindowHint | Qt::WindowSystemMenuHint),
  m_wpOverlayManager(CApplication::Instance()->System<COverlayManager>()),
  m_pOriginalParent(pParent),
  m_pTargetWidget(pParent),
  m_iZOrder(iZOrder),
  m_bReachedDestination(false),
  m_bShowCalled(false)
{
  setVisible(false);
  if (auto spManager = m_wpOverlayManager.lock())
  {
    spManager->RegisterOverlay(this);
  }
}

COverlayBase::~COverlayBase()
{
  if (auto spManager = m_wpOverlayManager.lock())
  {
    spManager->RemoveOverlay(this);
  }
}

//----------------------------------------------------------------------------------------
//
void COverlayBase::Hide()
{
  m_bShowCalled = false;

  hide();
}

//----------------------------------------------------------------------------------------
//
void COverlayBase::Resize()
{
  move(QPoint(0, 0));
  resize(m_pTargetWidget->size());
}

//----------------------------------------------------------------------------------------
//
void COverlayBase::Show()
{
  m_bShowCalled = true;

  setFocus();

  Climb();

  if (nullptr != m_pTargetWidget)
  {
    Resize();
    if (m_pTargetWidget->isVisible())
    {
      show();
    }
    else
    {
      Hide();
    }
  }

  m_bShowCalled = true;
}

//----------------------------------------------------------------------------------------
//
bool COverlayBase::IsInPlace() const
{
  return m_bReachedDestination;
}

//---------------------------------------------------------------------------------------
//
qint32 COverlayBase::ZOrder() const
{
  return m_iZOrder;
}

//---------------------------------------------------------------------------------------
//
void COverlayBase::RebuildZOrder()
{
  if (auto spManager = m_wpOverlayManager.lock())
  {
    spManager->RebuildOverlayOrder();
  }
}

//---------------------------------------------------------------------------------------
//
void COverlayBase::ClimbToCentralWidget()
{
  if (nullptr != parent())
  {
    parent()->removeEventFilter(this);
  }

  QWidget* pParentW = nullptr;
  QMainWindow* pMainWindow = FindMainWindow();
  if(nullptr != pMainWindow)
  {
    pParentW = pMainWindow->centralWidget();
  }

  FinishClimb(pParentW);
}

//----------------------------------------------------------------------------------------
//
void COverlayBase::ClimbToFirstInstanceOf(const QString& sClassName, bool bToParentOfFound)
{
  if (nullptr != parent())
  {
    parent()->removeEventFilter(this);
  }

  QWidget* pFoundParent = m_pOriginalParent;
  QWidget* pCurrentWidget = m_pOriginalParent;
  while (nullptr != pCurrentWidget)
  {
    if (nullptr != pCurrentWidget->metaObject() &&
        nullptr != pCurrentWidget->parentWidget())
    {
      const QString sFoundClassName = QString(pCurrentWidget->metaObject()->className());
      if (0 == sFoundClassName.compare(sClassName))
      {
        // found! -> abort
        pFoundParent = bToParentOfFound? pCurrentWidget->parentWidget() : pCurrentWidget;
        m_bReachedDestination = true;
        break;
      }
    }
    pFoundParent = pCurrentWidget;
    pCurrentWidget = pFoundParent->parentWidget();
  }

  FinishClimb(pFoundParent);
}

//----------------------------------------------------------------------------------------
//
void COverlayBase::ClimbToTop()
{
  if (nullptr != parent())
  {
    parent()->removeEventFilter(this);
  }
  FinishClimb(FindMainWindow());
}

//----------------------------------------------------------------------------------------
//
bool COverlayBase::eventFilter(QObject* pObject, QEvent* pEvent)
{
  if (m_pTargetWidget == pObject)
  {
    switch (pEvent->type())
    {
    case QEvent::Move:
    case QEvent::Resize:
      if (m_bShowCalled)
      {
        Resize();
      }
      break;
    case QEvent::Show:
      if (m_bShowCalled)
      {
        Show();
      }
      break;
    case QEvent::Hide:
      hide();
      break;
    default:
      break;
    }
  }

  // all events should propagate
  return false;
}

//----------------------------------------------------------------------------------------
//
void COverlayBase::FinishClimb(QWidget* pParentW)
{
  setParent(pParentW);
  m_pTargetWidget = pParentW;

  if (nullptr != m_pTargetWidget)
  {
    m_pTargetWidget->installEventFilter(this);
  }

  raise();
  RebuildZOrder();
}

//----------------------------------------------------------------------------------------
//
QMainWindow* COverlayBase::FindMainWindow()
{
  QWidget* pParentW = m_pOriginalParent;
  while (nullptr != pParentW)
  {
    QMainWindow* pMainWindow = qobject_cast<QMainWindow*>(pParentW);
    if (nullptr != pMainWindow)
    {
      // found! -> abort
      m_bReachedDestination = true;
      return pMainWindow;
    }
    pParentW = pParentW->parentWidget();
  }
  return nullptr;
}
