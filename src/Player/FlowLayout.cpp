#include "FlowLayout.h"
#include <QtWidgets>


CFlowLayout::CFlowLayout(QWidget* pParent, int iMargin, int iHSpacing, int iVSpacing) :
  QLayout(pParent), m_iHSpace(iHSpacing), m_iVSpace(iVSpacing)
{
  setContentsMargins(iMargin, iMargin, iMargin, iMargin);
}

CFlowLayout::CFlowLayout(int iMargin, int iHSpacing, int iVSpacing) :
  m_iHSpace(iHSpacing), m_iVSpace(iVSpacing)
{
  setContentsMargins(iMargin, iMargin, iMargin, iMargin);
}

CFlowLayout::~CFlowLayout()
{
  QLayoutItem *item;
  while ((item = takeAt(0)))
  {
    delete item;
  }
}

//----------------------------------------------------------------------------------------
//
void CFlowLayout::addItem(QLayoutItem* pItem)
{
  m_vpItemList.append(pItem);
}

//----------------------------------------------------------------------------------------
//
int CFlowLayout::horizontalSpacing() const
{
  if (m_iHSpace >= 0)
  {
    return m_iHSpace;
  }
  else
  {
    return smartSpacing(QStyle::PM_LayoutHorizontalSpacing);
  }
}

//----------------------------------------------------------------------------------------
//
int CFlowLayout::verticalSpacing() const
{
  if (m_iVSpace >= 0)
  {
    return m_iVSpace;
  }
  else
  {
    return smartSpacing(QStyle::PM_LayoutVerticalSpacing);
  }
}

//----------------------------------------------------------------------------------------
//
int CFlowLayout::count() const
{
  return m_vpItemList.size();
}

//----------------------------------------------------------------------------------------
//
QLayoutItem* CFlowLayout::itemAt(int index) const
{
  return m_vpItemList.value(index);
}

//----------------------------------------------------------------------------------------
//
QLayoutItem* CFlowLayout::takeAt(int index)
{
  if (index >= 0 && index < m_vpItemList.size())
  {
    return m_vpItemList.takeAt(index);
  }
  return nullptr;
}

//----------------------------------------------------------------------------------------
//
Qt::Orientations CFlowLayout::expandingDirections() const
{
  return 0;
}

//----------------------------------------------------------------------------------------
//
bool CFlowLayout::hasHeightForWidth() const
{
  return true;
}

//----------------------------------------------------------------------------------------
//
int CFlowLayout::heightForWidth(int iWidth) const
{
  qint32 iHeight = doLayout(QRect(0, 0, iWidth, 0), true);
  return iHeight;
}

//----------------------------------------------------------------------------------------
//
void CFlowLayout::setGeometry(const QRect& rect)
{
  QLayout::setGeometry(rect);
  doLayout(rect, false);
}

//----------------------------------------------------------------------------------------
//
QSize CFlowLayout::sizeHint() const
{
  return minimumSize();
}

//----------------------------------------------------------------------------------------
//
QSize CFlowLayout::minimumSize() const
{
  QSize size;
  for (const QLayoutItem* pItem : qAsConst(m_vpItemList))
  {
    size = size.expandedTo(pItem->minimumSize());
  }

  const QMargins margins = contentsMargins();
  size += QSize(margins.left() + margins.right(), margins.top() + margins.bottom());
  return size;
}

//----------------------------------------------------------------------------------------
//
int CFlowLayout::doLayout(const QRect &rect, bool testOnly) const
{
  qint32 iLeft, iTop, iRight, iBottom;
  getContentsMargins(&iLeft, &iTop, &iRight, &iBottom);
  QRect effectiveRect = rect.adjusted(+iLeft, +iTop, -iRight, -iBottom);
  qint32 x = effectiveRect.x();
  qint32 y = effectiveRect.y();
  qint32 iLineHeight = 0;

  std::vector<qint32> viTotalWidth;
  viTotalWidth.push_back(0);
  size_t uiCurrentIndex = 0;
  for (QLayoutItem* pItem : qAsConst(m_vpItemList))
  {
    const QWidget* pWid = pItem->widget();
    qint32 iSpaceX = horizontalSpacing();
    if (iSpaceX == -1)
    {
      iSpaceX = pWid->style()->layoutSpacing(
          QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Horizontal);
    }
    qint32 iSpaceY = verticalSpacing();
    if (iSpaceY == -1)
    {
      iSpaceY = pWid->style()->layoutSpacing(
          QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Vertical);
    }

    viTotalWidth[uiCurrentIndex] += pItem->sizeHint().width() + iSpaceX;
    qint32 iNextX = x + pItem->sizeHint().width() + iSpaceX;
    if (iNextX - iSpaceX > effectiveRect.right() && iLineHeight > 0)
    {
      viTotalWidth.push_back(0);
      uiCurrentIndex++;
      x = effectiveRect.x();
      y = y + iLineHeight + iSpaceY;
      viTotalWidth[uiCurrentIndex] += pItem->sizeHint().width() + iSpaceX;
      iNextX = x + pItem->sizeHint().width() + iSpaceX;
      iLineHeight = 0;
    }

    x = iNextX;
    iLineHeight = qMax(iLineHeight, pItem->sizeHint().height());
  }

  uiCurrentIndex = 0;
  x = effectiveRect.x() + effectiveRect.width() / 2 - viTotalWidth[uiCurrentIndex] / 2;
  y = effectiveRect.y();
  for (QLayoutItem* pItem : qAsConst(m_vpItemList))
  {
    const QWidget* pWid = pItem->widget();
    qint32 iSpaceX = horizontalSpacing();
    if (iSpaceX == -1)
    {
      iSpaceX = pWid->style()->layoutSpacing(
          QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Horizontal);
    }
    qint32 iSpaceY = verticalSpacing();
    if (iSpaceY == -1)
    {
      iSpaceY = pWid->style()->layoutSpacing(
          QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Vertical);
    }

    qint32 iNextX = x + pItem->sizeHint().width() + iSpaceX;
    if (iNextX - iSpaceX > effectiveRect.right() && iLineHeight > 0)
    {
      uiCurrentIndex++;
      if (m_vpItemList.size() > static_cast<qint32>(uiCurrentIndex))
      {
        x = effectiveRect.x() + effectiveRect.width() / 2 - viTotalWidth[uiCurrentIndex] / 2;
        y = y + iLineHeight + iSpaceY;
        iNextX = x + pItem->sizeHint().width() + iSpaceX;
        iLineHeight = 0;
      }
    }

    if (!testOnly)
    {
      pItem->setGeometry(QRect(QPoint(x, y), pItem->sizeHint()));
    }

    x = iNextX;
    iLineHeight = qMax(iLineHeight, pItem->sizeHint().height());
  }

  return y + iLineHeight - rect.y() + iBottom;
}

//----------------------------------------------------------------------------------------
//
int CFlowLayout::smartSpacing(QStyle::PixelMetric pm) const
{
  QObject* pParent = this->parent();
  if (nullptr == pParent)
  {
    return -1;
  }
  else if (pParent->isWidgetType())
  {
    QWidget* pW = static_cast<QWidget*>(pParent);
    return pW->style()->pixelMetric(pm, nullptr, pW);
  }
  else
  {
    return static_cast<QLayout*>(pParent)->spacing();
  }
}