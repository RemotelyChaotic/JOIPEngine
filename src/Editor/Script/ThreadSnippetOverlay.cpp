#include "ThreadSnippetOverlay.h"
#include "Application.h"
#include "Editor/Resources/ResourceTreeItem.h"
#include "Editor/Resources/ResourceTreeItemModel.h"
#include "Editor/Resources/ResourceTreeItemSortFilterProxyModel.h"
#include "Systems/DatabaseManager.h"
#include "Systems/Database/Resource.h"
#include "ui_ThreadSnippetOverlay.h"

CThreadSnippetOverlay::CThreadSnippetOverlay(QWidget* pParent) :
  CCodeSnippetOverlayBase(pParent),
  m_spUi(std::make_unique<Ui::CThreadSnippetOverlay>()),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>()),
  m_data()
{
  m_spUi->setupUi(this);
  CResourceTreeItemSortFilterProxyModel* pProxyModel =
      new CResourceTreeItemSortFilterProxyModel(m_spUi->pResourceSelectTree);
  m_spUi->pResourceSelectTree->setModel(pProxyModel);
  m_preferredSize = size();
  m_bInitialized = true;
}

CThreadSnippetOverlay::~CThreadSnippetOverlay()
{
  dynamic_cast<CResourceTreeItemSortFilterProxyModel*>(m_spUi->pResourceSelectTree->model())
      ->setSourceModel(nullptr);
}

//----------------------------------------------------------------------------------------
//
void CThreadSnippetOverlay::Initialize(CResourceTreeItemModel* pResourceTreeModel)
{
  SetInitialized(false);

  CResourceTreeItemSortFilterProxyModel* pProxyModel =
      dynamic_cast<CResourceTreeItemSortFilterProxyModel*>(m_spUi->pResourceSelectTree->model());
  pProxyModel->FilterForTypes({EResourceType::eScript, EResourceType::eSequence});
  pProxyModel->setSourceModel(pResourceTreeModel);

  QItemSelectionModel* pSelectionModel = m_spUi->pResourceSelectTree->selectionModel();
  connect(pSelectionModel, &QItemSelectionModel::currentChanged,
          this, &CThreadSnippetOverlay::SlotCurrentChanged);

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
void CThreadSnippetOverlay::Resize()
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
}

//----------------------------------------------------------------------------------------
//
void CThreadSnippetOverlay::on_pSleepCheckBox_toggled(bool bStatus)
{
  if (!m_bInitialized) { return; }
  m_data.m_bSleep = bStatus;
}

//----------------------------------------------------------------------------------------
//
void CThreadSnippetOverlay::on_pSleepSpinBox_valueChanged(double dValue)
{
  if (!m_bInitialized) { return; }
  m_data.m_bSleepTimeS = dValue;
}

//----------------------------------------------------------------------------------------
//
void CThreadSnippetOverlay::on_pSkippableCheckBox_toggled(bool bStatus)
{
  if (!m_bInitialized) { return; }
  m_data.m_bSkippable = bStatus;
}

//----------------------------------------------------------------------------------------
//
void CThreadSnippetOverlay::on_pKillThreadTextBox_toggled(bool bStatus)
{
  if (!m_bInitialized) { return; }
  m_data.m_bKill = bStatus;
}

//----------------------------------------------------------------------------------------
//
void CThreadSnippetOverlay::on_pKillThreadName_editingFinished()
{
  if (!m_bInitialized) { return; }
  m_data.m_sKillId = m_spUi->pKillThreadName->text();
}

//----------------------------------------------------------------------------------------
//
void CThreadSnippetOverlay::on_pRunAsynchCheckBox_toggled(bool bStatus)
{
  if (!m_bInitialized) { return; }
  m_data.m_bAsynch = bStatus;
}

//----------------------------------------------------------------------------------------
//
void CThreadSnippetOverlay::on_pIdLineEdit_editingFinished()
{
  if (!m_bInitialized) { return; }
  m_data.m_sAsynchId = m_spUi->pIdLineEdit->text();
}

//----------------------------------------------------------------------------------------
//
void CThreadSnippetOverlay::on_pRunAsynchLineEdit_editingFinished()
{
  if (!m_bInitialized) { return; }
  m_data.m_sAsynchScript = m_spUi->pRunAsynchLineEdit->text();
}

//----------------------------------------------------------------------------------------
//
void CThreadSnippetOverlay::on_pFilter_SignalFilterChanged(const QString& sText)
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
void CThreadSnippetOverlay::on_pConfirmButton_clicked()
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
void CThreadSnippetOverlay::on_pCancelButton_clicked()
{
  Hide();
}

//----------------------------------------------------------------------------------------
//
void CThreadSnippetOverlay::SlotCurrentChanged(const QModelIndex &current,
                                               const QModelIndex &previous)
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
    m_spUi->pRunAsynchLineEdit->setText(sName);
    on_pRunAsynchLineEdit_editingFinished();
  }
}
