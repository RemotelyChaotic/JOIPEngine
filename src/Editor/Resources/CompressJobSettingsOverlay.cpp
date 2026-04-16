#include "CompressJobSettingsOverlay.h"
#include "ui_CompressJobSettingsOverlay.h"

#include "Editor/Resources/ResourceTreeItem.h"
#include "Editor/Resources/ResourceTreeItemModel.h"

CCompressResourceModel::CCompressResourceModel(QObject* pParent) :
  CResourceTreeItemSortFilterProxyModel(pParent)
{}
CCompressResourceModel::~CCompressResourceModel() = default;

//----------------------------------------------------------------------------------------
//
const std::set<QString>& CCompressResourceModel::CheckedResources() const
{
  return m_checked;
}

//----------------------------------------------------------------------------------------
//
void CCompressResourceModel::Reset()
{
  m_checked.clear();
  CResourceTreeItemModel* pModel = dynamic_cast<CResourceTreeItemModel*>(sourceModel());
  if (nullptr != pModel)
  {
    auto spProject = pModel->Project();
    QReadLocker l(&spProject->m_rwLock);
    for (const auto& [ sName, spRes ] : spProject->m_baseData.m_spResourcesMap)
    {
      QReadLocker lr(&spRes->m_rwLock);
      if (EResourceType::eImage == spRes->m_type._to_integral())
      {
        m_checked.insert(sName);
      }
    }
  }

  invalidateFilter();
}

//----------------------------------------------------------------------------------------
//
QVariant CCompressResourceModel::data(const QModelIndex& proxyIndex, qint32 role) const
{
  if (proxyIndex.column() == resource_item::c_iColumnName && Qt::CheckStateRole == role &&
      nullptr != proxyIndex.model())
  {
    const QModelIndex indexType = proxyIndex.model()->index(proxyIndex.row(), resource_item::c_iColumnType, proxyIndex.parent());
    const QVariant vItemType = proxyIndex.model()->data(indexType, CResourceTreeItemModel::eItemTypeRole);
    if (vItemType.toInt() == EResourceTreeItemType::eResource)
    {
      const QString sResource = proxyIndex.data().toString();
      return m_checked.find(sResource) != m_checked.end() ? Qt::Checked : Qt::Unchecked;
    }
    else
    {
      bool bPartial = false;
      qint32 iNumChecked = 0;
      const qint32 iRowCount = proxyIndex.model()->rowCount(proxyIndex);
      for (qint32 i = 0; iRowCount > i; ++i)
      {
        QModelIndex idxChild = proxyIndex.model()->index(i, proxyIndex.column(), proxyIndex);
        qint32 iChildState = idxChild.data(Qt::CheckStateRole).toInt();
        if (Qt::Checked == iChildState)
        {
          iNumChecked++;
        }
        else if (Qt::PartiallyChecked == iChildState)
        {
          bPartial = true;
        }
      }
      if (iNumChecked == iRowCount)
      {
        return Qt::Checked;
      }
      else if (0 < iNumChecked)
      {
        return Qt::PartiallyChecked;
      }
      else
      {
        return bPartial ? Qt::PartiallyChecked : Qt::Unchecked;
      }
    }
  }
  return CResourceTreeItemSortFilterProxyModel::data(proxyIndex, role);
}

//----------------------------------------------------------------------------------------
//
Qt::ItemFlags CCompressResourceModel::flags(const QModelIndex &index) const
{
  Qt::ItemFlags flags = CResourceTreeItemSortFilterProxyModel::flags(index);
  if (index.isValid())
  {
    flags |= Qt::ItemIsUserCheckable;
    if (index.column() == resource_item::c_iColumnName &&
        nullptr != index.model())
    {
      const QModelIndex indexType = index.model()->index(index.row(), resource_item::c_iColumnType, index.parent());
      const QVariant vItemType = index.model()->data(indexType, CResourceTreeItemModel::eItemTypeRole);
      if (!vItemType.isValid() || vItemType.toInt() != EResourceTreeItemType::eResource)
      {
        flags |= Qt::ItemIsAutoTristate;
      }
    }
  }
  return flags;
}

//----------------------------------------------------------------------------------------
//
bool CCompressResourceModel::setData(const QModelIndex& index, const QVariant& value, qint32 role)
{
  if (index.column() == resource_item::c_iColumnName && Qt::CheckStateRole == role &&
      nullptr != index.model())
  {
    const QModelIndex indexType = index.model()->index(index.row(), resource_item::c_iColumnType, index.parent());
    const QVariant vItemType = index.model()->data(indexType, CResourceTreeItemModel::eItemTypeRole);
    if (vItemType.toInt() == EResourceTreeItemType::eResource)
    {
      const QString sResource = index.data().toString();
      if (value.value<Qt::CheckState>() == Qt::Checked)
      {
        m_checked.insert(sResource);
      }
      else
      {
        auto it = m_checked.find(sResource);
        if (m_checked.end() != it)
        {
          m_checked.erase(it);
        }
      }
      emit dataChanged(index, index, {Qt::CheckStateRole});
      QModelIndex parentIdx = index.parent();
      while (parentIdx.isValid())
      {
        emit dataChanged(parentIdx, parentIdx, {Qt::CheckStateRole});
        parentIdx = parentIdx.parent();
      }
      return true;
    }
    else
    {
      Qt::CheckState checkedState = data(index, Qt::CheckStateRole).value<Qt::CheckState>();
      const qint32 iRowCount = index.model()->rowCount(index);
      for (qint32 i = 0; iRowCount > i; ++i)
      {
        QModelIndex idxChild = index.model()->index(i, index.column(), index);
        setData(idxChild, checkedState != Qt::Checked ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole);
      }
      return true;
    }
  }
  return CResourceTreeItemSortFilterProxyModel::setData(index, value, role);
}

//----------------------------------------------------------------------------------------
//
CCompressJobSettingsOverlay::CCompressJobSettingsOverlay(QWidget* pParent) :
    COverlayBase(100, pParent),
    m_spUi(std::make_unique<Ui::CCompressJobSettingsOverlay>())
{
  qRegisterMetaType<CCompressJobSettingsOverlay::SCompressJobSettings>();
  qRegisterMetaType<std::set<QString>>();

  m_spUi->setupUi(this);

  m_pProxyModel =
      new CCompressResourceModel(m_spUi->pResourceSelectTree);
  m_spUi->pResourceSelectTree->setModel(m_pProxyModel);

  QItemSelectionModel* pSelectionModel = m_spUi->pResourceSelectTree->selectionModel();
  connect(pSelectionModel, &QItemSelectionModel::currentChanged,
          this, [](){});

  m_pProxyModel->FilterForTypes({EResourceType::eImage});
  m_pProxyModel->setFilterRegExp(QRegExp(".*", Qt::CaseInsensitive, QRegExp::RegExp));
}

CCompressJobSettingsOverlay::~CCompressJobSettingsOverlay()
{
}

//----------------------------------------------------------------------------------------
//
void CCompressJobSettingsOverlay::Climb()
{
  ClimbToFirstInstanceOf("CEditorMainScreen", false);
}

//----------------------------------------------------------------------------------------
//
void CCompressJobSettingsOverlay::Hide()
{
  m_pProxyModel->setSourceModel(nullptr);
  m_pProxyModel->Reset();
  COverlayBase::Hide();
}

//----------------------------------------------------------------------------------------
//
void CCompressJobSettingsOverlay::Show(const tspProject& spProject,
                                       QPointer<CResourceTreeItemModel> pModel)
{
  Q_UNUSED(spProject)
  m_pProxyModel->setSourceModel(pModel);
  m_pProxyModel->Reset();

  m_pProxyModel->sort(resource_item::c_iColumnName, Qt::AscendingOrder);

  m_spUi->pResourceSelectTree->setColumnHidden(resource_item::c_iColumnType, true);
  m_spUi->pResourceSelectTree->setColumnHidden(resource_item::c_iColumnPath, true);
  m_spUi->pResourceSelectTree->header()->setSectionResizeMode(resource_item::c_iColumnName, QHeaderView::Stretch);
  m_spUi->pResourceSelectTree->header()->setSectionResizeMode(resource_item::c_iColumnType, QHeaderView::Interactive);

  COverlayBase::Show();
}

//----------------------------------------------------------------------------------------
//
void CCompressJobSettingsOverlay::Resize()
{
  QPoint newPos =
      QPoint(m_pTargetWidget->geometry().width() / 2, m_pTargetWidget->geometry().height() / 2) -
      QPoint(width() / 2, height() / 2);

  move(newPos.x(), newPos.y());
  resize(width(), height());
}

//----------------------------------------------------------------------------------------
//
void CCompressJobSettingsOverlay::on_pFilter_SignalFilterChanged(const QString& sText)
{
  if (sText.isNull() || sText.isEmpty())
  {
    m_pProxyModel->setFilterRegExp(QRegExp(".*", Qt::CaseInsensitive, QRegExp::RegExp));
  }
  else
  {
    m_pProxyModel->setFilterRegExp(QRegExp(sText, Qt::CaseInsensitive, QRegExp::RegExp));
  }
}

//----------------------------------------------------------------------------------------
//
void CCompressJobSettingsOverlay::on_pConfirmButton_clicked()
{
  SCompressJobSettings settings{ m_spUi->pCompressionSlider->value(), m_pProxyModel->CheckedResources() };
  emit SignalJobSettingsConfirmed(settings);
  Hide();
}

//----------------------------------------------------------------------------------------
//
void CCompressJobSettingsOverlay::on_pCancelButton_clicked()
{
  Hide();
}
