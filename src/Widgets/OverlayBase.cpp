#include "OverlayBase.h"
#include "Application.h"
#include "MainWindow.h"

COverlayBase::COverlayBase(QWidget *parent) :
  QFrame(parent, Qt::FramelessWindowHint | Qt::WindowSystemMenuHint),
  m_pTarget(nullptr)
{
}

COverlayBase::~COverlayBase()
{
}

//----------------------------------------------------------------------------------------
//
void COverlayBase::Hide()
{
  hide();
}

//----------------------------------------------------------------------------------------
//
void COverlayBase::Resize()
{
  move(0, 0);
  resize(m_pTarget->geometry().width(), m_pTarget->geometry().height());
}

//----------------------------------------------------------------------------------------
//
void COverlayBase::Show()
{
  ClimbToTop();
  show();
}

//----------------------------------------------------------------------------------------
//
void COverlayBase::ClimbToTop()
{
  CMainWindow* pMainWindow =
      dynamic_cast<CMainWindow*>(CApplication::Instance()->activeWindow());
  if (nullptr != pMainWindow)
  {
    m_pTarget = pMainWindow;
    this->setParent(pMainWindow);
  }
}

//----------------------------------------------------------------------------------------
//
bool COverlayBase::event(QEvent* pEvent)
{
  switch (pEvent->type())
  {
    case QEvent::Hide: Hide(); break;
    case QEvent::Resize: Resize(); break;
    case QEvent::Show: Show(); break;
    default:break;
  }
  return QWidget::event(pEvent);
}
