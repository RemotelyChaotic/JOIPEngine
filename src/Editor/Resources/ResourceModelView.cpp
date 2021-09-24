#include "ResourceModelView.h"
#include "CommandChangeCurrentResource.h"
#include "ResourceTreeItem.h"
#include "ResourceTreeItemModel.h"
#include "ResourceTreeItemSortFilterProxyModel.h"
#include "ui_ResourceModelView.h"
#include "Utils/UndoRedoFilter.h"
#include <QItemSelectionModel>
#include <QListView>
#include <QUndoStack>

CResourceModelView::CResourceModelView(QWidget *parent) :
  QWidget(parent),
  m_spUi(std::make_unique<Ui::CResourceModelView>()),
  m_spCurrentProject(nullptr),
  m_pProxy(new CResourceTreeItemSortFilterProxyModel(this)),
  m_pModel(nullptr),
  m_pStack(nullptr),
  m_bInitializing(false)
{
  m_spUi->setupUi(this);
  m_spUi->pTabWidget->tabBar()->setVisible(false);
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

  m_spUi->pTreeView->setModel(m_pProxy);
  m_spUi->pDetailView->setModel(m_pProxy);

  m_spUi->pTreeView->header()->setStretchLastSection(true);
  m_spUi->pTreeView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);

  QItemSelectionModel* pSelectionModel = m_spUi->pTreeView->selectionModel();
  connect(pSelectionModel, &QItemSelectionModel::currentChanged,
          this, &CResourceModelView::SlotCurrentChanged);
  pSelectionModel = m_spUi->pDetailView->selectionModel();
  connect(pSelectionModel, &QItemSelectionModel::currentChanged,
          this, &CResourceModelView::SlotCurrentChanged);

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
}

//----------------------------------------------------------------------------------------
//
QItemSelectionModel* CResourceModelView::CurrentSelectionModel() const
{
  if (m_spUi->pTabWidget->currentIndex() == CResourceModelView::eTree)
  {
    return m_spUi->pTreeView->selectionModel();
  }
  else
  {
    return m_spUi->pDetailView->selectionModel();
  }
}

//----------------------------------------------------------------------------------------
//
QStringList CResourceModelView::SelectedResources() const
{
  QStringList vsResources;
  QItemSelectionModel* pSelectionModel = CurrentSelectionModel();
  if (nullptr != m_pProxy && nullptr != m_pModel && nullptr != pSelectionModel)
  {
    QModelIndexList indexes = pSelectionModel->selectedIndexes();
    pSelectionModel->clearSelection();
    foreach (QModelIndex index, indexes)
    {
      if (m_pModel->IsResourceType(m_pProxy->mapToSource(index)))
      {
        const QString sName = m_pModel->data(m_pProxy->mapToSource(index), Qt::DisplayRole).toString();
        vsResources << sName;

        // only interrested in first item which is the actual item we need
        break;
      }
    }
  }
  return vsResources;
}

//----------------------------------------------------------------------------------------
//
void CResourceModelView::SetView(CResourceModelView::EView view)
{
  m_spUi->pTabWidget->setCurrentIndex(view);
}

//----------------------------------------------------------------------------------------
//
CResourceModelView::EView CResourceModelView::View() const
{
  if (m_spUi->pTabWidget->currentIndex() == CResourceModelView::eTree)
  {
    return CResourceModelView::eTree;
  }
  else
  {
    return CResourceModelView::eExplorer;
  }
}

//----------------------------------------------------------------------------------------
//
void CResourceModelView::SlotCurrentChanged(const QModelIndex& current,
                                            const QModelIndex& previous)
{
  if (m_bInitializing) { return; }
  Q_UNUSED(previous);

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
                                            [this, pThis](const QString& sName){
      if (nullptr != pThis)
      {
        emit SignalResourceSelected(sName);
      }
    }));
  }
}
