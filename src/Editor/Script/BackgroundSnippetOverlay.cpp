#include "BackgroundSnippetOverlay.h"
#include "Editor/Resources/ResourceTreeItem.h"
#include "Editor/Resources/ResourceTreeItemModel.h"
#include "Editor/Resources/ResourceTreeItemSortFilterProxyModel.h"
#include "ui_BackgroundSnippetOverlay.h"
#include <QScrollBar>

CBackgroundSnippetOverlay::CBackgroundSnippetOverlay(QWidget* pParent) :
  CCodeSnippetOverlayBase(pParent),
  m_spUi(new Ui::CBackgroundSnippetOverlay),
  m_data()
{
  m_spUi->setupUi(this);
  m_spUi->pScrollArea->setWidgetResizable(true);
  m_preferredSize = size();
}

CBackgroundSnippetOverlay::~CBackgroundSnippetOverlay()
{
  dynamic_cast<CResourceTreeItemSortFilterProxyModel*>(m_spUi->pResourceSelectTree->model())
      ->setSourceModel(nullptr);
}

//----------------------------------------------------------------------------------------
//
void CBackgroundSnippetOverlay::Initialize(CResourceTreeItemModel* pResourceTreeModel)
{
  SetInitialized(false);

  CResourceTreeItemSortFilterProxyModel* pProxyModel =
      new CResourceTreeItemSortFilterProxyModel(m_spUi->pResourceSelectTree);
  pProxyModel->setSourceModel(pResourceTreeModel);
  pProxyModel->FilterForTypes({EResourceType::eImage});
  m_spUi->pResourceSelectTree->setModel(pProxyModel);

  QItemSelectionModel* pSelectionModel = m_spUi->pResourceSelectTree->selectionModel();
  connect(pSelectionModel, &QItemSelectionModel::currentChanged,
          this, &CBackgroundSnippetOverlay::SlotCurrentChanged);

  pProxyModel->sort(resource_item::c_iColumnName, Qt::AscendingOrder);
  pProxyModel->setFilterRegExp(QRegExp(".*", Qt::CaseInsensitive, QRegExp::RegExp));

  // setup Tree
  m_spUi->pResourceSelectTree->setColumnHidden(resource_item::c_iColumnType, true);
  m_spUi->pResourceSelectTree->setColumnHidden(resource_item::c_iColumnPath, true);
  m_spUi->pResourceSelectTree->header()->setSectionResizeMode(resource_item::c_iColumnName, QHeaderView::Stretch);
  m_spUi->pResourceSelectTree->header()->setSectionResizeMode(resource_item::c_iColumnType, QHeaderView::Interactive);

  SetInitialized(true);
}

//----------------------------------------------------------------------------------------
//
void CBackgroundSnippetOverlay::Resize()
{
  QSize newSize = m_preferredSize;
  if (m_pTargetWidget->geometry().width() < m_preferredSize.width())
  {
    newSize.setWidth(m_pTargetWidget->geometry().width());
  }
  if (m_pTargetWidget->geometry().height() < m_preferredSize.height())
  {
    newSize.setHeight(m_pTargetWidget->geometry().height());
  }

  QPoint newPos =
      QPoint(m_pTargetWidget->geometry().width() / 2, m_pTargetWidget->geometry().height() / 2) -
      QPoint(newSize.width() / 2, newSize.height() / 2);

  move(newPos.x(), newPos.y());
  resize(newSize);

  m_spUi->pScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_spUi->pScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  m_spUi->pScrollArea->widget()->setMinimumWidth(
        newSize.width() - m_spUi->pScrollArea->verticalScrollBar()->width() -
        m_spUi->pScrollArea->widget()->layout()->spacing());
}

//----------------------------------------------------------------------------------------
//
void CBackgroundSnippetOverlay::on_pResourceCheckBox_toggled(bool bState)
{
  if (!m_bInitialized) { return; }
  m_data.m_bUseResource = bState;
}

//----------------------------------------------------------------------------------------
//
void CBackgroundSnippetOverlay::on_pResourceLineEdit_editingFinished()
{
  if (!m_bInitialized) { return; }
  m_data.m_sCurrentResource = m_spUi->pResourceLineEdit->text();
}

//----------------------------------------------------------------------------------------
//
void CBackgroundSnippetOverlay::on_pColorCheckBox_toggled(bool bState)
{
  if (!m_bInitialized) { return; }
  m_data.m_bUseColor = bState;
}

//----------------------------------------------------------------------------------------
//
void CBackgroundSnippetOverlay::on_pColorWidget_SignalColorChanged(const QColor& color)
{
  if (!m_bInitialized) { return; }
  m_data.m_color = color;
}

//----------------------------------------------------------------------------------------
//
void CBackgroundSnippetOverlay::on_pFilter_SignalFilterChanged(const QString& sText)
{
  if (!m_bInitialized) { return; }

  CResourceTreeItemSortFilterProxyModel* pProxyModel =
      dynamic_cast<CResourceTreeItemSortFilterProxyModel*>(m_spUi->pResourceSelectTree->model());

  if (sText.isNull() || sText.isEmpty())
  {
    pProxyModel->setFilterRegExp(QRegExp(".*", Qt::CaseInsensitive, QRegExp::RegExp));
  }
  else
  {
    pProxyModel->setFilterRegExp(QRegExp(sText, Qt::CaseInsensitive, QRegExp::RegExp));
  }
}

//----------------------------------------------------------------------------------------
//
void CBackgroundSnippetOverlay::on_pConfirmButton_clicked()
{
  auto spGenerator = CodeGenerator();
  if (nullptr != spGenerator)
  {
    emit SignalCodeGenerated(spGenerator->Generate(m_data, nullptr));
  }
  Hide();
}

//----------------------------------------------------------------------------------------
//
void CBackgroundSnippetOverlay::on_pCancelButton_clicked()
{
  Hide();
}

//----------------------------------------------------------------------------------------
//
void CBackgroundSnippetOverlay::SlotCurrentChanged(const QModelIndex& current,
                                                   const QModelIndex& previous)
{
  if (!m_bInitialized) { return; }
  Q_UNUSED(previous);

  QSortFilterProxyModel* pProxyModel =
    dynamic_cast<QSortFilterProxyModel*>(m_spUi->pResourceSelectTree->model());
  CResourceTreeItemModel* pModel =
    dynamic_cast<CResourceTreeItemModel*>(pProxyModel->sourceModel());

  if (nullptr != pModel)
  {
    const QString sName =
        pModel->data(pProxyModel->mapToSource(current), Qt::DisplayRole, resource_item::c_iColumnName).toString();
    m_spUi->pResourceLineEdit->setText(sName);
    on_pResourceLineEdit_editingFinished();
  }
}
