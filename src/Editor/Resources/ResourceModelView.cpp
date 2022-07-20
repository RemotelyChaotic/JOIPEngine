#include "ResourceModelView.h"
#include "CommandChangeCurrentResource.h"
#include "ResourceToolTip.h"
#include "ResourceTreeItem.h"
#include "ResourceTreeItemModel.h"
#include "ResourceTreeItemSortFilterProxyModel.h"
#include "ui_ResourceModelView.h"

#include "Utils/UndoRedoFilter.h"

#include <QClipboard>
#include <QContextMenuEvent>
#include <QHelpEvent>
#include <QItemSelectionModel>
#include <QListView>
#include <QUndoStack>
#include <QToolTip>

#include <limits>

CResourceModelView::CResourceModelView(QWidget *parent) :
  QWidget(parent),
  m_spUi(std::make_unique<Ui::CResourceModelView>()),
  m_spCurrentProject(nullptr),
  m_pProxy(new CResourceTreeItemSortFilterProxyModel(this)),
  m_pModel(nullptr),
  m_pStack(nullptr),
  m_bInitializing(false),
  m_bLandscape(false)
{
  m_spUi->setupUi(this);
}

CResourceModelView::~CResourceModelView()
{
  dynamic_cast<CResourceTreeItemSortFilterProxyModel*>(m_spUi->pTreeView->model())
      ->setSourceModel(nullptr);
  dynamic_cast<CResourceTreeItemSortFilterProxyModel*>(m_spUi->pDetailView->model())
      ->setSourceModel(nullptr);
}

//----------------------------------------------------------------------------------------
//
void CResourceModelView::Initialize(QUndoStack* pStack,
                                    CResourceTreeItemModel* pModel)
{
  m_bInitializing = true;

  m_pStack = pStack;
  m_pModel = pModel;

  m_pProxy->sort(resource_item::c_iColumnName, Qt::AscendingOrder);
  m_pProxy->setFilterRegExp(QRegExp(".*", Qt::CaseInsensitive, QRegExp::RegExp));

  m_pProxy->setSourceModel(pModel);

  // prevents flickering because of recalculation of scrollbar-size
  m_spUi->pDetailView->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOn);

  m_spUi->pTreeView->installEventFilter(this);
  m_spUi->pDetailView->installEventFilter(this);

  m_spUi->pTreeView->setModel(m_pProxy);
  m_spUi->pDetailView->setModel(m_pProxy);

  m_spUi->pTreeView->header()->setSectionHidden(resource_item::c_iColumnType, true);
  m_spUi->pTreeView->header()->setSectionHidden(resource_item::c_iColumnPath, true);
  m_spUi->pTreeView->header()->setSectionResizeMode(resource_item::c_iColumnName, QHeaderView::Stretch);
  m_spUi->pTreeView->header()->setSectionResizeMode(resource_item::c_iColumnType, QHeaderView::Interactive);

  QItemSelectionModel* pSelectionModel = m_spUi->pTreeView->selectionModel();
  connect(pSelectionModel, &QItemSelectionModel::currentChanged,
          this, &CResourceModelView::SlotCurrentChanged);
  pSelectionModel = m_spUi->pDetailView->selectionModel();
  connect(pSelectionModel, &QItemSelectionModel::currentChanged,
          this, &CResourceModelView::SlotCurrentChanged);

  connect(m_spUi->pDetailView, &CResourceDetailView::Expanded,
          this, &CResourceModelView::SlotExpanded);
  connect(m_spUi->pDetailView, &CResourceDetailView::Collapsed,
          this, &CResourceModelView::SlotCollapsed);

  auto pFilter =
      new CUndoRedoFilter(m_spUi->pTreeView, nullptr);
  connect(pFilter, &CUndoRedoFilter::UndoTriggered, this, [this]()
  { if (nullptr != m_pStack) { m_pStack->undo(); } });
  connect(pFilter, &CUndoRedoFilter::RedoTriggered, this, [this]()
  { if (nullptr != m_pStack) { m_pStack->redo(); } });
  pFilter =
      new CUndoRedoFilter(m_spUi->pDetailView, nullptr);
  connect(pFilter, &CUndoRedoFilter::UndoTriggered, this, [this]()
  { if (nullptr != m_pStack) { m_pStack->undo(); } });
  connect(pFilter, &CUndoRedoFilter::RedoTriggered, this, [this]()
  { if (nullptr != m_pStack) { m_pStack->redo(); } });


  m_bInitializing = false;
}

//----------------------------------------------------------------------------------------
//
void CResourceModelView::ProjectLoaded(tspProject spCurrentProject, bool bReadOnly)
{
  m_spCurrentProject = spCurrentProject;
  if (nullptr != m_pModel)
  {
    m_spUi->pTreeView->setEditTriggers(
          bReadOnly ? QAbstractItemView::NoEditTriggers :
                      QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
    m_spUi->pDetailView->SetReadOnly(bReadOnly);
    m_spUi->pDetailView->Initialize(m_spCurrentProject);
  }
}

//----------------------------------------------------------------------------------------
//
void CResourceModelView::ProjectUnloaded()
{
  m_spUi->pTreeView->setEditTriggers(
        QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
  m_spUi->pDetailView->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
  m_spUi->pDetailView->SetReadOnly(false);
  m_spUi->pDetailView->DeInitilaze();
}

//----------------------------------------------------------------------------------------
//
void CResourceModelView::CdUp()
{
  for (auto pModel : SelectionModels()) { pModel->clearSelection(); }
  m_spUi->pDetailView->Collapse(m_spUi->pDetailView->rootIndex());
}

//----------------------------------------------------------------------------------------
//
std::vector<QPointer<QItemSelectionModel>> CResourceModelView::SelectionModels() const
{
  return {m_spUi->pTreeView->selectionModel(), m_spUi->pDetailView->selectionModel()};
}

//----------------------------------------------------------------------------------------
//
QStringList CResourceModelView::SelectedResources() const
{
  QStringList vsResources;
  for (QPointer<QItemSelectionModel> pSelectionModel : SelectionModels())
  {
    if (nullptr != m_pProxy && nullptr != m_pModel && nullptr != pSelectionModel)
    {
      QModelIndexList indexes = pSelectionModel->selectedIndexes();
      pSelectionModel->clearSelection();
      foreach (QModelIndex index, indexes)
      {
        if (m_pModel->IsResourceType(m_pProxy->mapToSource(index)))
        {
          const QString sName = m_pModel->data(m_pProxy->mapToSource(index), Qt::DisplayRole).toString();
          if (!vsResources.contains(sName))
          {
            vsResources << sName;
          }
          // only interrested in first item which is the actual item we need
          break;
        }
      }
    }
  }
  return vsResources;
}

//----------------------------------------------------------------------------------------
//
void CResourceModelView::SetView(CResourceModelView::EView view)
{
  switch(view)
  {
    case CResourceModelView::eTree:
      m_spUi->pTreeView->setVisible(true);
      m_spUi->pDetailView->setVisible(false);
      break;
    case CResourceModelView::eExplorer:
      m_spUi->pTreeView->setVisible(false);
      m_spUi->pDetailView->setVisible(true);
      break;
    case CResourceModelView::eBoth:
      m_spUi->pTreeView->setVisible(true);
      m_spUi->pDetailView->setVisible(true);
      break;
  }
}

//----------------------------------------------------------------------------------------
//
CResourceModelView::EView CResourceModelView::View() const
{
  if (m_spUi->pTreeView->isVisible() && !m_spUi->pDetailView->isVisible())
  {
    return CResourceModelView::eTree;
  }
  else if (m_spUi->pDetailView->isVisible() && !m_spUi->pTreeView->isVisible())
  {
    return CResourceModelView::eExplorer;
  }
  else
  {
    return CResourceModelView::eBoth;
  }
}

//----------------------------------------------------------------------------------------
//
void CResourceModelView::SetLandscape(bool bLandscape)
{
  if (m_bLandscape != bLandscape)
  {
    m_bLandscape = bLandscape;
    QLayout* pLayout = layout();
    if (nullptr != pLayout)
    {
      QList<QWidget*> vpWidgets;
      while (auto pItm = pLayout->takeAt(0))
      {
        if (pItm->widget())
        {
          vpWidgets << pItm->widget();
        }
        delete pItm;
      }

      delete pLayout;
      if (m_bLandscape)
      {
        pLayout = new QHBoxLayout(this);
      }
      else
      {
        pLayout = new QVBoxLayout(this);
      }
      pLayout->setContentsMargins(0, 0, 0, 0);
      setLayout(pLayout);
      for (QWidget* pWidget : vpWidgets)
      {
        pLayout->addWidget(pWidget);
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CResourceModelView::ShowContextMenu(CResourceTreeItemModel* pModel, const QModelIndex& idx,
                                         const QPoint& globalPos)
{
  if (nullptr != pModel && pModel == dynamic_cast<const CResourceTreeItemModel*>(idx.model()))
  {
    CResourceTreeItem* item = static_cast<CResourceTreeItem*>(idx.internalPointer());
    if (nullptr != item && nullptr != item->Resource())
    {
      QMenu menu(this);

      QAction* pAction = new QAction("Copy Name", &menu);
      connect(pAction, &QAction::triggered, pModel, [idx]() {
        CResourceTreeItem* item = static_cast<CResourceTreeItem*>(idx.internalPointer());
        if (nullptr != item)
        {
          QClipboard* pClipboard = QGuiApplication::clipboard();
          tspResource spResource = item->Resource();
          QReadLocker locker(&spResource->m_rwLock);
          pClipboard->setText(spResource->m_sName);
        }
      });
      menu.addAction(pAction);
      pAction = new QAction("Copy Path", &menu);
      connect(pAction, &QAction::triggered, pModel, [idx]() {
        CResourceTreeItem* item = static_cast<CResourceTreeItem*>(idx.internalPointer());
        if (nullptr != item)
        {
          QClipboard* pClipboard = QGuiApplication::clipboard();
          tspResource spResource = item->Resource();
          QReadLocker locker(&spResource->m_rwLock);
          pClipboard->setText(spResource->m_sPath.toString());
        }
      });
      menu.addAction(pAction);
      pAction = new QAction("Copy System Path", &menu);
      connect(pAction, &QAction::triggered, pModel, [idx]() {
        CResourceTreeItem* item = static_cast<CResourceTreeItem*>(idx.internalPointer());
        if (nullptr != item)
        {
          QClipboard* pClipboard = QGuiApplication::clipboard();
          tspResource spResource = item->Resource();
          pClipboard->setText(PhysicalResourcePath(spResource));
        }
      });
      menu.addAction(pAction);
      pAction = new QAction("Copy Source", &menu);
      connect(pAction, &QAction::triggered, pModel, [idx]() {
        CResourceTreeItem* item = static_cast<CResourceTreeItem*>(idx.internalPointer());
        if (nullptr != item)
        {
          QClipboard* pClipboard = QGuiApplication::clipboard();
          tspResource spResource = item->Resource();
          QReadLocker locker(&spResource->m_rwLock);
          pClipboard->setText(spResource->m_sSource.toString());
        }
      });
      menu.addAction(pAction);

      menu.exec(globalPos);
    }
  }
}

//----------------------------------------------------------------------------------------
//
bool CResourceModelView::Landscape()
{
  return m_bLandscape;
}

//----------------------------------------------------------------------------------------
//
void CResourceModelView::SlotExpanded(const QModelIndex& index)
{
  // handle selection internally
  for (auto pModel : SelectionModels()) { pModel->blockSignals(true); }
  auto parent = index;
  while (parent.isValid())
  {
    m_spUi->pTreeView->expand(parent);
    parent = parent.parent();
  }
  m_spUi->pTreeView->selectionModel()->select(
        index, QItemSelectionModel::SelectionFlag::ClearAndSelect);
  for (auto pModel : SelectionModels()) { pModel->blockSignals(false); }
}

//----------------------------------------------------------------------------------------
//
void CResourceModelView::SlotCollapsed(const QModelIndex& index)
{
  Q_UNUSED(index)
}

//----------------------------------------------------------------------------------------
//
void CResourceModelView::SlotCurrentChanged(const QModelIndex& current,
                                            const QModelIndex& previous)
{
  if (m_bInitializing) { return; }
  Q_UNUSED(previous);

  // handle selection internally
  for (auto pModel : SelectionModels()) { pModel->blockSignals(true); }
  if (sender() == m_spUi->pTreeView->selectionModel())
  {
    if (m_spUi->pTreeView->model()->hasChildren(current))
    {
      m_spUi->pDetailView->Expand(current);
    }
    else
    {
      m_spUi->pDetailView->Expand(current.parent());
    }
    m_spUi->pDetailView->selectionModel()->select(
          current, QItemSelectionModel::SelectionFlag::ClearAndSelect);
  }
  else if (sender() == m_spUi->pDetailView->selectionModel())
  {
    auto parent = current.parent();
    while (parent.isValid())
    {
      m_spUi->pTreeView->expand(parent);
      parent = parent.parent();
    }
    m_spUi->pTreeView->selectionModel()->select(
          current, QItemSelectionModel::SelectionFlag::ClearAndSelect);
  }
  for (auto pModel : SelectionModels()) { pModel->blockSignals(false); }

  m_spUi->pTreeView->viewport()->repaint();
  m_spUi->pDetailView->viewport()->repaint();

  // create undo command
  if (nullptr != m_pModel && nullptr != m_pProxy && nullptr != m_pStack)
  {
    const QString sPrevious =
        m_pModel->data(m_pProxy->mapToSource(previous), Qt::DisplayRole, resource_item::c_iColumnName).toString();
    const QString sName =
        m_pModel->data(m_pProxy->mapToSource(current), Qt::DisplayRole, resource_item::c_iColumnName).toString();

    QPointer<CResourceModelView> pThis(this);
    m_pStack->push(
          new CCommandChangeCurrentResource(m_spCurrentProject,
                                            { m_spUi->pTreeView->selectionModel(),
                                              m_spUi->pDetailView->selectionModel() },
                                            m_pProxy.data(), sPrevious, sName,
                                            [this, pThis, current](const QString& sName){
      if (nullptr != pThis)
      {
        m_spUi->pDetailView->RequestResource(current);
        emit SignalResourceSelected(sName);
      }
    }));
  }
}

//----------------------------------------------------------------------------------------
//
bool CResourceModelView::eventFilter(QObject* pObj, QEvent* pEvt)
{
  if (nullptr == pObj || nullptr == pEvt) { return QWidget::eventFilter(pObj, pEvt); }
  if (m_spUi->pTreeView == pObj || m_spUi->pDetailView == pObj)
  {
    if (QEvent::ToolTip == pEvt->type())
    {
      QHelpEvent* pHelpEvent = static_cast<QHelpEvent*>(pEvt);

      QModelIndex index;
      if (m_spUi->pTreeView == pObj)
      {
        index = m_spUi->pTreeView->indexAt(
              m_spUi->pTreeView->viewport()->mapFromGlobal(pHelpEvent->globalPos()));
        if (auto pProxy = dynamic_cast<QSortFilterProxyModel*>(m_spUi->pTreeView->model()))
        {
          index = pProxy->mapToSource(index);
        }
      }
      else if (m_spUi->pDetailView == pObj)
      {
        index = m_spUi->pDetailView->indexAt(
              m_spUi->pDetailView->viewport()->mapFromGlobal(pHelpEvent->globalPos()));
        if (auto pProxy = dynamic_cast<QSortFilterProxyModel*>(m_spUi->pDetailView->model()))
        {
          index = pProxy->mapToSource(index);
        }
      }

      CResourceTreeItem* pItem = static_cast<CResourceTreeItem*>(index.internalPointer());
      if (nullptr != pItem)
      {
        if (EResourceTreeItemType::eResource == pItem->Type()._to_integral())
        {
          CResourceToolTip::showResource(pHelpEvent->globalPos(),
                                         pItem->Resource(),
                                         qobject_cast<QWidget*>(pObj));
        }
        else
        {
          QToolTip::showText(pHelpEvent->globalPos(),
                             pItem->Data(resource_item::c_iColumnToolTip).toString(),
                             qobject_cast<QWidget*>(pObj));
        }
        pEvt->ignore();
        return true;
      }
    }
    else if (QEvent::ContextMenu == pEvt->type())
    {
      QContextMenuEvent* pContextEvt = static_cast<QContextMenuEvent*>(pEvt);
      CResourceTreeItemModel* pModel = nullptr;
      QModelIndex index;
      if (m_spUi->pTreeView == pObj)
      {
        index = m_spUi->pTreeView->indexAt(
              m_spUi->pTreeView->viewport()->mapFromGlobal(pContextEvt->globalPos()));
        if (auto pProxy = dynamic_cast<QSortFilterProxyModel*>(m_spUi->pTreeView->model()))
        {
          pModel = dynamic_cast<CResourceTreeItemModel*>(pProxy->sourceModel());
          index = pProxy->mapToSource(index);
        }
      }
      else if (m_spUi->pDetailView == pObj)
      {
        index = m_spUi->pDetailView->indexAt(
              m_spUi->pDetailView->viewport()->mapFromGlobal(pContextEvt->globalPos()));
        if (auto pProxy = dynamic_cast<QSortFilterProxyModel*>(m_spUi->pDetailView->model()))
        {
          pModel = dynamic_cast<CResourceTreeItemModel*>(pProxy->sourceModel());
          index = pProxy->mapToSource(index);
        }
      }
      ShowContextMenu(pModel, index, pContextEvt->globalPos());
    }
  }
  return QWidget::eventFilter(pObj, pEvt);
}

//----------------------------------------------------------------------------------------
//
void CResourceModelView::resizeEvent(QResizeEvent* pEvent)
{
  QWidget::resizeEvent(pEvent);
  if (m_bLandscape)
  {
    m_spUi->pTreeView->setMinimumWidth(0);
    m_spUi->pTreeView->setMaximumWidth(std::max(200,width()/3));
  }
  else
  {
    m_spUi->pTreeView->setMinimumWidth(0);
    m_spUi->pTreeView->setMaximumWidth(QWIDGETSIZE_MAX);
  }
}
