#include "HtmlViewDelegate.h"

#include <QAbstractTextDocumentLayout>
#include <QApplication>
#include <QFocusEvent>
#include <QMenu>
#include <QPainter>
#include <QTextDocument>
#include <QStyledItemDelegate>
#include <QSortFilterProxyModel>

CHtmlViewDelegate::CHtmlViewDelegate(QTreeView* pTree) :
  QStyledItemDelegate(pTree)
{
}

//----------------------------------------------------------------------------------------
//
void CHtmlViewDelegate::paint(QPainter* pPainter,
                              const QStyleOptionViewItem& option,
                              const QModelIndex& index) const
{
  QStyleOptionViewItem options = option;
  initStyleOption(&options, index);

  QStyle* pStyle = options.widget ? options.widget->style() : QApplication::style();

  QTextDocument doc;
  doc.setHtml("&nbsp;&nbsp;" + options.text);
  doc.setTextWidth(options.rect.width());
  doc.setDocumentMargin(0);
  doc.setDefaultFont(options.font);

  /// Painting item without text
  options.text = QString();
  pStyle->drawControl(QStyle::CE_ItemViewItem, &options, pPainter, options.widget);

  QAbstractTextDocumentLayout::PaintContext ctx;

  // Highlighting text if item is selected
  ctx.palette = options.palette;
  if (options.state & QStyle::State_Selected)
  {
    ctx.palette.setColor(QPalette::Text, options.palette.color(QPalette::Active, QPalette::HighlightedText));
  }
  else
  {
    ctx.palette.setColor(QPalette::Text, options.palette.color(QPalette::Active, QPalette::Text));
  }
  ctx.palette.setColor(QPalette::Window, Qt::transparent);
  ctx.palette.setColor(QPalette::Base, Qt::transparent);
  ctx.palette.setColor(QPalette::AlternateBase, Qt::transparent);

  QRect textRect = pStyle->subElementRect(QStyle::SE_ItemViewItemText, &options);
  pPainter->save();
  pPainter->translate(textRect.topLeft());
  pPainter->setClipRect(textRect.translated(-textRect.topLeft()));
  doc.documentLayout()->draw(pPainter, ctx);
  pPainter->restore();
}

//----------------------------------------------------------------------------------------
//
QSize CHtmlViewDelegate::sizeHint(
    const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  return QStyledItemDelegate::sizeHint(option, index);
}
