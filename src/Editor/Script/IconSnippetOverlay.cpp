#include "IconSnippetOverlay.h"
#include "Editor/Resources/ResourceTreeItem.h"
#include "Editor/Resources/ResourceTreeItemModel.h"
#include "Editor/Resources/ResourceTreeItemSortFilterProxyModel.h"
#include "ui_IconSnippetOverlay.h"
#include <QScrollBar>

CIconSnippetOverlay::CIconSnippetOverlay(QWidget* pParent) :
  CCodeSnippetOverlayBase(pParent),
  m_spUi(new Ui::CIconSnippetOverlay),
  m_data()
{
  m_spUi->setupUi(this);
  m_spUi->pScrollArea->setWidgetResizable(true);
  m_preferredSize = size();
}

CIconSnippetOverlay::~CIconSnippetOverlay()
{
  dynamic_cast<CResourceTreeItemSortFilterProxyModel*>(m_spUi->pResourceSelectTree->model())
      ->setSourceModel(nullptr);
}

//----------------------------------------------------------------------------------------
//
void CIconSnippetOverlay::Initialize(CResourceTreeItemModel* pResourceTreeModel)
{
  SetInitialized(false);

  CResourceTreeItemSortFilterProxyModel* pProxyModel =
      new CResourceTreeItemSortFilterProxyModel(m_spUi->pResourceSelectTree);
  pProxyModel->setSourceModel(pResourceTreeModel);
  pProxyModel->FilterForTypes({EResourceType::eImage, EResourceType::eMovie});
  m_spUi->pResourceSelectTree->setModel(pProxyModel);

  QItemSelectionModel* pSelectionModel = m_spUi->pResourceSelectTree->selectionModel();
  connect(pSelectionModel, &QItemSelectionModel::currentChanged,
          this, &CIconSnippetOverlay::SlotCurrentChanged);

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
void CIconSnippetOverlay::Resize()
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
void CIconSnippetOverlay::on_pResourceLineEdit_editingFinished()
{
  if (!m_bInitialized) { return; }
  m_data.m_sCurrentResource = m_spUi->pResourceLineEdit->text();
}

//----------------------------------------------------------------------------------------
//
void CIconSnippetOverlay::on_pShowCheckBox_toggled(bool bState)
{
  if (!m_bInitialized) { return; }
  m_data.m_bShow = bState;

  m_spUi->pHideCheckBox->blockSignals(true);
  m_spUi->pHideCheckBox->setChecked(!bState);
  m_spUi->pHideCheckBox->blockSignals(false);
}

//----------------------------------------------------------------------------------------
//
void CIconSnippetOverlay::on_pHideCheckBox_toggled(bool bState)
{
  if (!m_bInitialized) { return; }
  m_data.m_bShow = !bState;

  m_spUi->pShowCheckBox->blockSignals(true);
  m_spUi->pShowCheckBox->setChecked(!bState);
  m_spUi->pShowCheckBox->blockSignals(false);
}

//----------------------------------------------------------------------------------------
//
void CIconSnippetOverlay::on_pFilter_SignalFilterChanged(const QString& sText)
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
void CIconSnippetOverlay::on_CloseButton_clicked()
{
  if (!m_bInitialized) { return; }

  m_data.m_sCurrentResource = QString();
  m_spUi->pResourceLineEdit->setText(QString());
}

//----------------------------------------------------------------------------------------
//
void CIconSnippetOverlay::on_pConfirmButton_clicked()
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
void CIconSnippetOverlay::on_pCancelButton_clicked()
{
  Hide();
}

//----------------------------------------------------------------------------------------
//
void CIconSnippetOverlay::SlotCurrentChanged(const QModelIndex& current,
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
