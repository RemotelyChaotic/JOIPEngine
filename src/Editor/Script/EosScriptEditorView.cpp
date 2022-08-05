#include "EosScriptEditorView.h"
#include "EosCommandWidgets.h"
#include "EosScriptModel.h"

#include <QAbstractTextDocumentLayout>
#include <QApplication>
#include <QFocusEvent>
#include <QPainter>
#include <QTextDocument>
#include <QStyledItemDelegate>
#include <QSortFilterProxyModel>

class CEosScriptEditorViewDelegate : public QStyledItemDelegate
{
  Q_OBJECT

public:
  inline CEosScriptEditorViewDelegate(CEosScriptEditorView* pView) :
    QStyledItemDelegate(pView),
    m_pView(pView)
  {
  }

  //--------------------------------------------------------------------------------------
  //
  void paint(QPainter* pPainter,
       const QStyleOptionViewItem& option,
       const QModelIndex& index) const override
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

  //--------------------------------------------------------------------------------------
  //
  QSize sizeHint(
      const QStyleOptionViewItem& option, const QModelIndex& index) const override
  {
    return QStyledItemDelegate::sizeHint(option, index);
  }

private:
  CEosScriptEditorView* m_pView;
};

//----------------------------------------------------------------------------------------
//
CEosScriptEditorView::CEosScriptEditorView(QWidget* pParent) :
  QTreeView(pParent)
{
  qApp->installEventFilter(this);
  setItemDelegate(new CEosScriptEditorViewDelegate(this));
}
CEosScriptEditorView::~CEosScriptEditorView()
{

}

//----------------------------------------------------------------------------------------
//
void CEosScriptEditorView::setModel(QAbstractItemModel* pModel)
{
  QTreeView::setModel(pModel);
  if (nullptr != pModel)
  {
    m_pModel =
        dynamic_cast<CEosScriptModel*>(pModel);
    QSortFilterProxyModel* pProxy = dynamic_cast<QSortFilterProxyModel*>(pModel);
    if (nullptr != pProxy)
    {
      m_pModel = dynamic_cast<CEosScriptModel*>(pProxy->sourceModel());
    }
    if (nullptr != m_pModel)
    {
      connect(pModel, &QAbstractItemModel::modelReset, this,
              &CEosScriptEditorView::SlotModelReset, Qt::UniqueConnection);
      SlotModelReset();
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CEosScriptEditorView::ExpandAll()
{
  IterateItems(QModelIndex(), [this](const QModelIndex& idx) {
    expand(idx);
  });
}

//----------------------------------------------------------------------------------------
//
void CEosScriptEditorView::SlotModelReset()
{
  if (nullptr != m_pModel)
  {
    ExpandAll();
  }
}

//----------------------------------------------------------------------------------------
//
bool CEosScriptEditorView::eventFilter(QObject* pObj, QEvent* pEvt)
{
  if (nullptr != pObj && nullptr != pEvt)
  {
    if (pEvt->type() == QEvent::MouseButtonRelease ||
        pEvt->type() == QEvent::MouseButtonDblClick)
    {
      QMouseEvent* pMouseEvt = static_cast<QMouseEvent*>(pEvt);
      if (!geometry().contains(parentWidget()->mapFromGlobal(pMouseEvt->globalPos())))
      {
        emit SignalClickedOutside();
      }
    }
  }
  return false;
}

//----------------------------------------------------------------------------------------
//
void CEosScriptEditorView::focusInEvent(QFocusEvent* event)
{
  if (nullptr != event && event->gotFocus())
  {
    if (!indexAt(QCursor::pos()).isValid())
    {
      emit SignalClickedOutside();
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CEosScriptEditorView::IterateItems(const QModelIndex& index,
                                        const std::function<void(const QModelIndex&)>& fnToCall)
{
  if (index.isValid())
  {
    fnToCall(index);
  }

  if (!model()->hasChildren(index) || (index.flags() & Qt::ItemNeverHasChildren))
  {
    return;
  }
  qint32 iRows = model()->rowCount(index);
  for (qint32 i = 0; i < iRows; ++i)
  {
    IterateItems(model()->index(i, 0, index), fnToCall);
  }
}

// CResourceDetailViewDelegate
#include "EosScriptEditorView.moc"
