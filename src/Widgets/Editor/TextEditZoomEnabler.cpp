#include "TextEditZoomEnabler.h"

#include <QApplication>
#include <QGestureEvent>
#include <QHBoxLayout>
#include <QScroller>
#include <QTimer>
#include <chrono>

CTextEditZoomEnabler::CTextEditZoomEnabler(QPointer<QTextEdit> pEditor) :
  COverlayBase(0, pEditor),
  m_pEditor(pEditor)
{
  QHBoxLayout* pLayout = new QHBoxLayout(this);
  m_pLabel = new QLabel(QString::number(m_iZoomLevel) + " %", this);
  m_pLabel->setAttribute(Qt::WA_TranslucentBackground);
  m_pLabel->setAlignment(Qt::AlignCenter);
  pLayout->addWidget(m_pLabel);
  setLayout(pLayout);

  Climb();
}
CTextEditZoomEnabler::CTextEditZoomEnabler(QPointer<QPlainTextEdit> pEditor) :
  COverlayBase(0, pEditor),
  m_pEditor(pEditor)
{
  QHBoxLayout* pLayout = new QHBoxLayout(this);
  m_pLabel = new QLabel(QString::number(m_iZoomLevel) + " %", this);
  m_pLabel->setAttribute(Qt::WA_TranslucentBackground);
  m_pLabel->setAlignment(Qt::AlignCenter);
  pLayout->addWidget(m_pLabel);
  setLayout(pLayout);

  Climb();
}
CTextEditZoomEnabler::~CTextEditZoomEnabler()
{

}

//----------------------------------------------------------------------------------------
//
void CTextEditZoomEnabler::Climb()
{
  ClimbToFirstInstanceOf(m_pOriginalParent->metaObject()->className(), false);
}

//----------------------------------------------------------------------------------------
//
void CTextEditZoomEnabler::Resize()
{
  QFontMetrics metrics(m_pLabel->font());

  QRect emptyRect(QPoint(0,0), parentWidget()->size());
  QRect hintRect = metrics.boundingRect(emptyRect,
                           Qt::AlignCenter | Qt::TextWordWrap, m_pLabel->text());

  QSize targetSize(hintRect.size() + QSize(m_iInnerMargin*2, m_iInnerMargin*2));
  resize(targetSize);
  move(m_pTargetWidget->width() / 2 - targetSize.width() / 2,
       m_pTargetWidget->height() / 2 - targetSize.height() / 2);
}

//----------------------------------------------------------------------------------------
//
void CTextEditZoomEnabler::Show()
{
  COverlayBase::Show();
  using namespace std::chrono_literals;
  QPointer<CTextEditZoomEnabler> pThis(this);
  QTimer::singleShot(3s, [pThis](){
    if (nullptr != pThis)
    {
      pThis->Hide();
    }
  });
}

//----------------------------------------------------------------------------------------
//
void CTextEditZoomEnabler::SetZoom(qint32 iZoom)
{
  if (m_iZoomLevel != iZoom)
  {
    double dOldSize = 0.0;
    if (std::holds_alternative<QPointer<QTextEdit>>(m_pEditor))
    {
      dOldSize = std::get<QPointer<QTextEdit>>(m_pEditor)->document()->defaultFont().pointSizeF();
    }
    else if (std::holds_alternative<QPointer<QPlainTextEdit>>(m_pEditor))
    {
      dOldSize = std::get<QPointer<QPlainTextEdit>>(m_pEditor)->document()->defaultFont().pointSizeF();
    }

    m_pLabel->setText(QString::number(iZoom) + " %");

    double dNewSize = dOldSize * iZoom / m_iZoomLevel;

    if (std::holds_alternative<QPointer<QTextEdit>>(m_pEditor))
    {
      QPointer<QTextEdit> pEditor = std::get<QPointer<QTextEdit>>(m_pEditor);
      if (m_iZoomLevel > iZoom)
      {
        pEditor->zoomOut(static_cast<qint32>(dOldSize - dNewSize));
      }
      else
      {
        pEditor->zoomIn(static_cast<qint32>(dNewSize - dOldSize));
      }
    }
    else if (std::holds_alternative<QPointer<QPlainTextEdit>>(m_pEditor))
    {
      QPointer<QPlainTextEdit> pEditor = std::get<QPointer<QPlainTextEdit>>(m_pEditor);
      if (m_iZoomLevel > iZoom)
      {
        pEditor->zoomOut(static_cast<qint32>(dOldSize - dNewSize));
      }
      else
      {
        pEditor->zoomIn(static_cast<qint32>(dNewSize - dOldSize));
      }
    }

    m_iZoomLevel = iZoom;
    emit SignalZoomChanged(m_iZoomLevel);
  }
}

//----------------------------------------------------------------------------------------
//
bool CTextEditZoomEnabler::event(QEvent* pEvent)
{
  bool bRetVal = COverlayBase::event(pEvent);
  switch (pEvent->type())
  {
  case QEvent::Polish:
    Resize();
    break;
  default:
    break;
  }

  return bRetVal;
}

//----------------------------------------------------------------------------------------
//
bool CTextEditZoomEnabler::eventFilter(QObject* pObj, QEvent* pEvt)
{
  if (nullptr != pObj && m_pTargetWidget == pObj)
  {
    if (nullptr != pEvt && pEvt->type() == QEvent::Wheel &&
        QApplication::keyboardModifiers().testFlag(Qt::KeyboardModifier::ControlModifier))
    {
      QWheelEvent* pWheelEvent = static_cast<QWheelEvent*>(pEvt);
      if (pWheelEvent->angleDelta().y() > 0)
      {
        SetZoom(std::min(200, m_iZoomLevel + 10));
        Show();
        return true;
      }
      else if (pWheelEvent->angleDelta().y() < 0)
      {
        SetZoom(std::max(10, m_iZoomLevel - 10));
        Show();
        return true;
      }
    }
    else if (nullptr != pEvt && pEvt->type() == QEvent::Gesture)
    {
      QGestureEvent* pGesture = static_cast<QGestureEvent*>(pEvt);
      QList<QGesture*> vpGestures = pGesture->gestures();
      auto it = std::find_if(vpGestures.begin(), vpGestures.end(), [](QGesture* pGesture) {
        return pGesture->gestureType() == Qt::GestureType::PinchGesture;
      });
      if (vpGestures.end() != it)
      {
        QPinchGesture* pPinch = static_cast<QPinchGesture*>(*(it));
        QPinchGesture::ChangeFlags changeFlags = pPinch->changeFlags();
        if (changeFlags & QPinchGesture::ScaleFactorChanged)
        {
          SetZoom(std::min(200, std::max(10, m_iZoomLevel + static_cast<qint32>(100 * pPinch->scaleFactor()))));
          return true;
        }
      }
    }
  }
  return false;
}
