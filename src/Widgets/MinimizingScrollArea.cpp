#include "MinimizingScrollArea.h"
#include <QScrollBar>

CMinimizingScrollArea::CMinimizingScrollArea(QWidget* pParent)
  : QScrollArea{pParent}
{

}

//----------------------------------------------------------------------------------------
//
QSize CMinimizingScrollArea::minimumSizeHint() const
{
  QSize hint = QScrollArea::minimumSizeHint();
  if (horizontalScrollBarPolicy() == Qt::ScrollBarAlwaysOff &&
      nullptr != horizontalScrollBar())
  {
    hint.setHeight(hint.height()-horizontalScrollBar()->minimumSizeHint().height());
  }
  if (verticalScrollBarPolicy() == Qt::ScrollBarAlwaysOff &&
      nullptr != verticalScrollBar())
  {
    hint.setWidth(hint.width()-verticalScrollBar()->minimumSizeHint().width());
  }
  return hint;
}

//----------------------------------------------------------------------------------------
//
QSize CMinimizingScrollArea::sizeHint() const
{
  QSize hint = QScrollArea::sizeHint();
  if (horizontalScrollBarPolicy() == Qt::ScrollBarAlwaysOff &&
      nullptr != horizontalScrollBar())
  {
    hint.setHeight(hint.height()-horizontalScrollBar()->sizeHint().height());
  }
  if (verticalScrollBarPolicy() == Qt::ScrollBarAlwaysOff &&
      nullptr != verticalScrollBar())
  {
    hint.setWidth(hint.width()-verticalScrollBar()->sizeHint().width());
  }
  return hint;
}
