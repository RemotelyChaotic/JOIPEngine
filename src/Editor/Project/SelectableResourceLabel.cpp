#include "SelectableResourceLabel.h"
#include "Application.h"

#include "Editor/Resources/ResourceTreeItem.h"
#include "Editor/Resources/ResourceTreeItemModel.h"
#include "Editor/Resources/ResourceTreeItemSortFilterProxyModel.h"
#include "Editor/Resources/ResourceDetailViewFetcherThread.h"

#include "Systems/DatabaseManager.h"

#include "Widgets/SearchWidget.h"

#include <QEvent>
#include <QHeaderView>
#include <QListWidget>
#include <QMenu>
#include <QMouseEvent>
#include <QTreeView>
#include <QWidgetAction>

CSelectableResourceLabel::CSelectableResourceLabel(QWidget* pParent) :
  QLabel{pParent},
  m_spThreadedLoader(std::make_unique<CThreadedSystem>("ResourceToolTipFetcher"))
{
  m_spThreadedLoader->RegisterObject<CResourceDetailViewFetcherThread>();
  connect(ResourceFetcher().get(),
          qOverload<const QString&,const QPixmap&>(&CResourceDetailViewFetcherThread::LoadFinished),
          this, &CSelectableResourceLabel::SlotResourceLoadFinished);

  installEventFilter(this);
  setScaledContents(false);
}
CSelectableResourceLabel::~CSelectableResourceLabel() = default;

//----------------------------------------------------------------------------------------
//
void CSelectableResourceLabel::SetCurrentProject(const tspProject& spProj)
{
  m_spCurrentProject = spProj;
}

//----------------------------------------------------------------------------------------
//
void CSelectableResourceLabel::SetResourceModel(QPointer<CResourceTreeItemModel> pResourceModel)
{
  m_pResourceModel = pResourceModel;
}

//----------------------------------------------------------------------------------------
//
QString CSelectableResourceLabel::CurrentResource() const
{
  return m_sCurrentResource;
}

//----------------------------------------------------------------------------------------
//
void CSelectableResourceLabel::SetCurrentResource(const QString& sResource)
{
  if (m_sCurrentResource != sResource)
  {
    m_sCurrentResource = sResource;
    UpdateResource();
  }
}

//----------------------------------------------------------------------------------------
//
const QIcon& CSelectableResourceLabel::UnsetIcon() const
{
  return m_unsetIcon;
}

//----------------------------------------------------------------------------------------
//
void CSelectableResourceLabel::SetUnsetIcon(const QIcon& icon)
{
  m_unsetIcon = icon;
  UpdateResource();
  emit SignalUnsetIconChanged();
}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<CResourceDetailViewFetcherThread> CSelectableResourceLabel::ResourceFetcher() const
{
  return std::static_pointer_cast<CResourceDetailViewFetcherThread>(m_spThreadedLoader->Get());
}

//----------------------------------------------------------------------------------------
//
void CSelectableResourceLabel::SlotResourceLoadFinished(const QString& sName,
                                                        const QPixmap& pixmap)
{
  Q_UNUSED(sName)
  if (!pixmap.isNull())
  {
    setPixmap(pixmap);
  }
}

//----------------------------------------------------------------------------------------
//
bool CSelectableResourceLabel::eventFilter(QObject* pObj, QEvent* pEvt)
{
  if (nullptr != pObj && nullptr != pEvt)
  {
    if (pEvt->type() == QEvent::MouseButtonRelease)
    {
      QMouseEvent* pMEvt = static_cast<QMouseEvent*>(pEvt);
      OpenSelectResource(pMEvt->globalPos());
    }
  }
  return false;
}

//----------------------------------------------------------------------------------------
//
void CSelectableResourceLabel::OpenSelectResource(const QPoint& createPoint)
{
  QMenu resourceMenu;

  //Add filterbox to the context menu
  auto *txtBox = new CSearchWidget(&resourceMenu);

  auto* txtBoxAction = new QWidgetAction(&resourceMenu);
  txtBoxAction->setDefaultWidget(txtBox);

  resourceMenu.addAction(txtBoxAction);

  //Add result treeview to the context menu
  auto* pListView = new QTreeView(&resourceMenu);
  pListView->setAlternatingRowColors(true);
  pListView->setSortingEnabled(true);
  pListView->setAnimated(true);

  CResourceTreeItemSortFilterProxyModel* pProxyModel =
      new CResourceTreeItemSortFilterProxyModel(pListView);
  pProxyModel->setSourceModel(m_pResourceModel);
  pProxyModel->FilterForTypes({EResourceType::eImage, EResourceType::eMovie});
  pListView->setModel(pProxyModel);

  pProxyModel->sort(resource_item::c_iColumnName, Qt::AscendingOrder);
  pProxyModel->setFilterRegExp(QRegExp(".*", Qt::CaseInsensitive, QRegExp::RegExp));

  pListView->setColumnHidden(resource_item::c_iColumnType, true);
  pListView->setColumnHidden(resource_item::c_iColumnPath, true);
  pListView->header()->setSectionResizeMode(resource_item::c_iColumnName, QHeaderView::Stretch);
  pListView->header()->setSectionResizeMode(resource_item::c_iColumnType, QHeaderView::Interactive);

  //Setup filtering
  QObject::connect(txtBox, &CSearchWidget::SignalFilterChanged, txtBox,
                   [&](const QString& sText)
                   {
                     if (sText.isNull() || sText.isEmpty())
                     {
                       pProxyModel->setFilterRegExp(QRegExp(".*", Qt::CaseInsensitive, QRegExp::RegExp));
                     }
                     else
                     {
                       pProxyModel->setFilterRegExp(QRegExp(sText, Qt::CaseInsensitive, QRegExp::RegExp));
                     }
                   });

  QItemSelectionModel* pSelectionModel = pListView->selectionModel();
  connect(pSelectionModel, &QItemSelectionModel::currentChanged,
          pSelectionModel, [&](const QModelIndex& current, const QModelIndex& previous) {
            QModelIndex idx = pProxyModel->mapToSource(current);
            const QString sName =
                m_pResourceModel->data(idx, Qt::DisplayRole,
                                       resource_item::c_iColumnName).toString();
            qint32 iType =
                m_pResourceModel->data(idx, CResourceTreeItemModel::eItemTypeRole).toInt();
            if (EResourceTreeItemType::eResource == iType)
            {
              resourceMenu.close();
              emit SignalResourcePicked(m_sCurrentResource, sName);
            }
          });

  auto* pViewAction = new QWidgetAction(&resourceMenu);
  pViewAction->setDefaultWidget(pListView);

  resourceMenu.addAction(pViewAction);

  // show menu
  QPointer<CSelectableResourceLabel> pPtr(this);
  resourceMenu.exec(createPoint);
  if (nullptr == pPtr) { return; }

  pProxyModel->setSourceModel(nullptr);
}

//----------------------------------------------------------------------------------------
//
void CSelectableResourceLabel::UpdateResource()
{
  qint32 iId = -1;
  {
    QReadLocker l(&m_spCurrentProject->m_rwLock);
    iId = m_spCurrentProject->m_iId;
  }

  if (-1 != iId && !m_sCurrentResource.isEmpty())
  {
    if (ResourceFetcher()->IsLoading())
    {
      ResourceFetcher()->AbortLoading();
    }
    ResourceFetcher()->RequestResources(iId, QStringList() << m_sCurrentResource,
                                        size());
    return;
  }

  setPixmap(m_unsetIcon.pixmap(size()));
}
