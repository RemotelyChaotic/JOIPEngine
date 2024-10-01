#include "EosScriptEditorView.h"
#include "EosScriptModel.h"

#include "Widgets/HtmlViewDelegate.h"

#include <QApplication>
#include <QFocusEvent>
#include <QMenu>

//----------------------------------------------------------------------------------------
//
CEosScriptEditorView::CEosScriptEditorView(QWidget* pParent) :
  QTreeView(pParent),
  m_pModel(nullptr)
{
  qApp->installEventFilter(this);
  setItemDelegate(new CHtmlViewDelegate(this));

  setContextMenuPolicy(Qt::CustomContextMenu);
  connect(this, &QTreeView::customContextMenuRequested,
          this, &CEosScriptEditorView::SlotCreateContextMenu);

  m_pSearchBar = new CEditorSearchBar(this);
  m_pSearchBar->Climb();
  m_pSearchBar->Resize();

  // reset things after closing search bar
  connect(m_pSearchBar, &CEditorSearchBar::SignalHidden,
          this, &CEosScriptEditorView::SlotSearchAreaHidden);
  connect(m_pSearchBar, &CEditorSearchBar::SignalFilterChanged,
          this, &CEosScriptEditorView::SlotSearchFilterChanged);

  QAction* pSearchAction = new QAction(tr("Search"), this);
  pSearchAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
  pSearchAction->setShortcut(QKeySequence::Find);
  connect(pSearchAction, &QAction::triggered,
          this, &CEosScriptEditorView::SlotShowHideSearchFilter);
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
      connect(m_pModel, &CEosScriptModel::SignalContentsChange,
              this, &CEosScriptEditorView::SignalContentsChange, Qt::UniqueConnection);
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
void CEosScriptEditorView::SlotCreateContextMenu(QPoint p)
{
  QModelIndex index = indexAt(p);
  Q_UNUSED(index)

  QMenu* pMenu = new QMenu(this);
  pMenu->addAction(tr("Search"), this, SLOT(SlotShowHideSearchFilter()),
                   QKeySequence::Find);
  pMenu->exec(viewport()->mapToGlobal(p));
  pMenu->deleteLater();
}

//----------------------------------------------------------------------------------------
//
void CEosScriptEditorView::SlotSearchFilterChanged(
    CEditorSearchBar::ESearhDirection, const QString& sText)
{
  QSortFilterProxyModel* pFilterModel =
      dynamic_cast<QSortFilterProxyModel*>(model());
  if (nullptr != pFilterModel)
  {
    pFilterModel->setFilterRegExp(QRegExp(sText));
    ExpandAll();
  }
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
void CEosScriptEditorView::SlotShowHideSearchFilter()
{
  if (m_pSearchBar->isVisible())
  {
    m_pSearchBar->Hide();
  }
  else
  {
    m_pSearchBar->Show();
  }
}

//----------------------------------------------------------------------------------------
//
void CEosScriptEditorView::SlotSearchAreaHidden()
{
  QSortFilterProxyModel* pFilterModel =
      dynamic_cast<QSortFilterProxyModel*>(model());
  if (nullptr != pFilterModel)
  {
    pFilterModel->setFilterRegExp(".*");
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
