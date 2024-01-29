#include "TimelineWidgetOverlay.h"
#include "TimelineWidget.h"

#include <QLayout>
#include <QPainter>

CTimelineWidgetOverlay::CTimelineWidgetOverlay(CTimelineWidget* pParent) :
  QWidget{pParent},
  m_pParent(pParent)
{
  setAttribute(Qt::WA_TranslucentBackground);
  setAttribute(Qt::WA_TransparentForMouseEvents);
  setAutoFillBackground(false);
}

CTimelineWidgetOverlay::~CTimelineWidgetOverlay() = default;

//----------------------------------------------------------------------------------------
//
const QColor& CTimelineWidgetOverlay::DropIndicationColor() const
{
  return m_pDropIndicationColor;
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidgetOverlay::SetCurrentTimePosIndicator(qint32 iPos)
{
  if (m_iCurrentTimePosIndicator != iPos)
  {
    m_iCurrentTimePosIndicator = iPos;
    repaint();
  }
}

//----------------------------------------------------------------------------------------
//
qint32 CTimelineWidgetOverlay::CurrentDropIndex() const
{
  return m_iCurrentDropIndex;
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidgetOverlay::SetShowDropIndicator(bool bShow)
{
  m_bShowDropIndicator = bShow;
  m_iCurrentDropIndex = -1;
  repaint();
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidgetOverlay::SetDropIndicationColor(const QColor& col)
{
  m_pDropIndicationColor = col;
  repaint();
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidgetOverlay::UpdateDropLine()
{
  QPoint cursorLocal = mapFromGlobal(QCursor::pos());
  auto item = ItemAt(cursorLocal);
  m_iCurrentDropIndex = item.first;
  if (-1 != item.first)
  {
    bool bBefore = false;
    m_lineDrop = LineFrom(item.second, cursorLocal, &bBefore);

    if (!bBefore)
    {
      m_iCurrentDropIndex++;
    }
  }

  repaint();
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidgetOverlay::paintEvent(QPaintEvent* pEvent)
{
  QPainter painter(this);
  QPen pen(m_pDropIndicationColor);
  if (m_bShowDropIndicator)
  {
    if (-1 != m_iCurrentDropIndex)
    {
      painter.save();
      pen.setWidth(4);
      QColor col = m_pDropIndicationColor.lighter();
      col.setAlphaF(0.5);
      pen.setColor(col);
      painter.setPen(pen);
      painter.drawLine(m_lineDrop);

      pen.setWidth(3);
      pen.setColor(m_pDropIndicationColor);
      painter.setPen(pen);
      painter.drawLine(m_lineDrop);
      painter.restore();
    }
  }

  if (-1 != m_iCurrentTimePosIndicator)
  {
    painter.save();
    pen.setWidth(4);
    QColor col = m_pDropIndicationColor.lighter();
    col.setAlphaF(0.5);
    pen.setColor(col);
    painter.setPen(pen);
    painter.drawLine(QLine(m_iCurrentTimePosIndicator, 0, m_iCurrentTimePosIndicator, height()));

    pen.setWidth(3);
    pen.setColor(m_pDropIndicationColor);
    painter.setPen(pen);
    painter.drawLine(QLine(m_iCurrentTimePosIndicator, 0, m_iCurrentTimePosIndicator, height()));
    painter.restore();
  }

}

//----------------------------------------------------------------------------------------
//
std::pair<qint32, QRect> CTimelineWidgetOverlay::ItemAt(QPoint p)
{
  if (nullptr != m_pParent && nullptr != m_pParent->widget())
  {
    if (auto pLayout = m_pParent->widget()->layout())
    {
      QWidget* pLastWidget = nullptr;
      for (qint32 i = 0; m_pParent->LayerCount() > i; ++i)
      {
        if (nullptr != pLayout->itemAt(i) && nullptr != pLayout->itemAt(i)->widget())
        {
          auto pWidget = pLayout->itemAt(i)->widget();
          if (nullptr != pWidget->parentWidget())
          {
            // are we below the first item?
            QPoint tl = mapFromGlobal(pWidget->parentWidget()->mapToGlobal(pWidget->geometry().topLeft()));
            if (p.y() >= 0 && p.y() <= tl.y())
            {
              QPoint br = mapFromGlobal(pWidget->parentWidget()->mapToGlobal(pWidget->geometry().bottomRight()));
              return {i, QRect(tl, br)};
            }

            // are we hovering an item?
            QPoint tlW = mapFromGlobal(pWidget->parentWidget()->mapToGlobal(pWidget->geometry().topLeft()));
            QPoint brW = mapFromGlobal(pWidget->parentWidget()->mapToGlobal(pWidget->geometry().bottomRight()));
            if (p.y() >= tlW.y() && p.y() <= brW.y())
            {
              return {i, QRect(tlW, brW)};
            }

            // are we between two items?
            if (nullptr != pLastWidget)
            {
              tl = mapFromGlobal(pLastWidget->parentWidget()->mapToGlobal(pLastWidget->geometry().bottomRight()));
              QPoint br = mapFromGlobal(pWidget->parentWidget()->mapToGlobal(pWidget->geometry().topLeft()));
              if (p.y() >= tl.y() && p.y() <= br.y())
              {
                return {i, QRect(tlW, brW)};
              }
            }
          }
          pLastWidget = pWidget;
        }
      }

      // are we at the bottom?
      if (nullptr != pLastWidget)
      {
        QPoint tl = mapFromGlobal(pLastWidget->parentWidget()->mapToGlobal(pLastWidget->geometry().bottomRight()));
        if (p.y() >= tl.y() && p.y() <= height())
        {
          tl = mapFromGlobal(pLastWidget->parentWidget()->mapToGlobal(pLastWidget->geometry().topLeft()));
          QPoint br = mapFromGlobal(pLastWidget->parentWidget()->mapToGlobal(pLastWidget->geometry().bottomRight()));
          return {m_pParent->LayerCount()-1, QRect(tl, br)};
        }
      }
      else
      {
        return {0, QRect(QPoint(width(), height()), QPoint(width(), height()))};
      }
    }
  }

  return {-1, QRect()};
}

//----------------------------------------------------------------------------------------
//
QLine CTimelineWidgetOverlay::LineFrom(QRect rect, QPoint p, bool* bBefore)
{
  qint32 iSpacing = 6;
  if (nullptr != m_pParent && nullptr != m_pParent->widget())
  {
    if (auto pLayout = m_pParent->widget()->layout())
    {
      iSpacing = pLayout->spacing();
    }
  }

  if (rect.center().y() < p.y())
  {
    if (nullptr != bBefore)
    {
      *bBefore = false;
    }

    qint32 y = rect.bottomLeft().y() + iSpacing / 2;
    if (y > height())
    {
      y = rect.topLeft().y() - iSpacing / 2;

      if (nullptr != bBefore)
      {
        *bBefore = true;
      }
    }
    return QLine(0, y, width(), y);
  }
  else
  {
    if (nullptr != bBefore)
    {
      *bBefore = true;
    }

    qint32 y = rect.topLeft().y() - iSpacing / 2;
    if (y > height())
    {
      y = rect.bottomLeft().y() + iSpacing / 2;

      if (nullptr != bBefore)
      {
        *bBefore = false;
      }
    }
    return QLine(0, y, width(), y);
  }
}
