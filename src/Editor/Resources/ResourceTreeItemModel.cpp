#include "ResourceTreeItemModel.h"
#include "Application.h"
#include "CommandChangeResourceData.h"
#include "ResourceTreeItem.h"
#include "Systems/DatabaseManager.h"
#include "Systems/Database/Resource.h"

#include <QPixmap>
#include <QUndoStack>

using namespace resource_item;

namespace
{
  const qint32 c_iResourceTimerIntervalMs = 100;
}

CResourceTreeItemModel::CResourceTreeItemModel(QPointer<QUndoStack> pUndoStack,
                                               QObject* pParent) :
  QAbstractItemModel(pParent),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>()),
  m_pUndoStack(pUndoStack),
  m_pRootItem(nullptr),
  m_categoryMap(),
  m_spProject(),
  m_cardIcon(),
  m_iIconSize(16)
{
  auto spDbManager = m_wpDbManager.lock();
  connect(spDbManager.get(), &CDatabaseManager::SignalResourceAdded,
          this, &CResourceTreeItemModel::SlotResourceAdded, Qt::QueuedConnection);
  connect(spDbManager.get(), &CDatabaseManager::SignalResourceRemoved,
          this, &CResourceTreeItemModel::SlotResourceRemoved, Qt::QueuedConnection);
  connect(spDbManager.get(), &CDatabaseManager::SignalSceneDataChanged,
          this, &CResourceTreeItemModel::SlotSceneDataChanged, Qt::QueuedConnection);

  m_resourcecheckTimer.setSingleShot(false);
  m_resourcecheckTimer.setInterval(c_iResourceTimerIntervalMs);
  connect(&m_resourcecheckTimer, &QTimer::timeout, this,
          &CResourceTreeItemModel::SlotResourceCheckerTimeout);
}

CResourceTreeItemModel::~CResourceTreeItemModel()
{
  DeInitializeModel();
}

//----------------------------------------------------------------------------------------
//
void CResourceTreeItemModel::InitializeModel(tspProject spProject)
{
  if (nullptr == m_pRootItem)
  {
    m_pRootItem = new CResourceTreeItem(EResourceTreeItemType::eRoot, std::nullopt, "Resources");

    beginInsertRows(QModelIndex(), 0, static_cast<qint32>(EResourceType::_size() - 1));
    for (size_t i = 0; i < EResourceType::_size(); i++)
    {
      CResourceTreeItem* pItem =
          new CResourceTreeItem(EResourceTreeItemType::eCategory,
                                EResourceType::_from_index(i),
                                QString(EResourceType::_from_index(i)._to_string()).remove(0, 1),
                                nullptr, m_pRootItem);
      m_categoryMap.insert({EResourceType::_from_index(i), pItem});
      m_pRootItem->AppendChild(pItem);
    }

    m_spProject = spProject;
    QReadLocker locker(&m_spProject->m_rwLock);
    for (auto it = m_spProject->m_baseData.m_spResourcesMap.begin(); m_spProject->m_baseData.m_spResourcesMap.end() != it; ++it)
    {
      QReadLocker locker(&it->second->m_rwLock);
      SResourcePath path = it->second->m_sPath;
      QStringList sPathParts;
      if (path.IsLocalFile())
      {
        locker.unlock();
        sPathParts = it->second->ResourceToAbsolutePath().split("/");
        sPathParts.removeAt(0); // first element is always the scheme
        locker.relock();
      }
      else
      {
        QUrl url = static_cast<QUrl>(path);
        sPathParts.push_back(url.host());
        sPathParts << url.path().remove(0, 1).split("/");
      }

      if (m_spProject->m_sPlayerLayout == it->first)
      {
        m_sOldProjectLayoutResource = it->first;
      }
      if (m_spProject->m_sTitleCard == it->first)
      {
        m_sOldProjectTitleResource = it->first;
      }

      // insert item
      auto categoryIt = m_categoryMap.find(it->second->m_type);
      if (m_categoryMap.end() != categoryIt)
      {
        // iterate over url subfolders
        CResourceTreeItem* pItem = categoryIt->second;
        CResourceTreeItem* pChild = nullptr;
        if (1 < sPathParts.size())
        {
          for (qint32 iCurrentPart = 0; sPathParts.size() > iCurrentPart; ++iCurrentPart)
          {
            const QString& sPathPart = sPathParts[iCurrentPart];
            QUrl sFolderName = QUrl(sPathPart);
            QString sDisplayFolderName = sFolderName.toString(QUrl::RemoveScheme);

            // iterate over children of current folder
            bool bFoundFolder = false;
            for (qint32 i = 0; i < pItem->ChildCount(); ++i)
            {
              pChild = pItem->Child(i);
              if (pChild->Type()._to_integral() == EResourceTreeItemType::eFolder &&
                  pChild->Data(c_iColumnName).toString() == sDisplayFolderName)
              {
                bFoundFolder = true;
                pItem = pChild;
                break;
              }
            }
            // folder not found -> create
            if (!bFoundFolder && iCurrentPart != sPathParts.size() - 1)
            {
              CResourceTreeItem* pNewItem =
                new CResourceTreeItem(EResourceTreeItemType::eFolder,
                                      std::nullopt,
                                      sDisplayFolderName, nullptr, pItem);
              pItem->AppendChild(pNewItem);
              pItem = pNewItem;
            }
          }
        }

        pItem->AppendChild(
              new CResourceTreeItem(EResourceTreeItemType::eResource,
                                    std::nullopt,
                                    QString(), it->second, pItem));
      }
    }
    endInsertRows();

    if (nullptr != m_spProject)
    {
      m_resourcecheckTimer.start();
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CResourceTreeItemModel::DeInitializeModel()
{
  if (nullptr != m_pRootItem)
  {
    beginRemoveRows(QModelIndex(), 0, static_cast<qint32>(EResourceType::_size() - 1));
    m_spProject = nullptr;
    m_categoryMap.clear();
    delete m_pRootItem;
    m_pRootItem = nullptr;

    m_sOldProjectLayoutResource = QString();
    m_sOldProjectTitleResource = QString();
    endRemoveRows();

    if (m_resourcecheckTimer.isActive())
    {
      m_resourcecheckTimer.stop();
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CResourceTreeItemModel::SetCardIcon(const QImage& img)
{
  m_cardIcon = img;
}

//----------------------------------------------------------------------------------------
//
void CResourceTreeItemModel::SetLayoutIcon(const QImage& img)
{
  m_layoutIcon = img;
}

//----------------------------------------------------------------------------------------
//
void CResourceTreeItemModel::SetIconSize(qint32 iValue)
{
  m_iIconSize = iValue;
}

//----------------------------------------------------------------------------------------
//
QVariant CResourceTreeItemModel::data(const QModelIndex& index, int iRole, int iColumnOverride)
{
  if (!index.isValid()) { return QVariant(); }
  if (Qt::DisplayRole == iRole)
  {
    CResourceTreeItem* item = static_cast<CResourceTreeItem*>(index.internalPointer());
    return item->Data(iColumnOverride);
  }
  else if (Qt::DecorationRole == iRole)
  {
    QReadLocker locker(&m_spProject->m_rwLock);
    CResourceTreeItem* item = static_cast<CResourceTreeItem*>(index.internalPointer());
    QString sName = item->Data(c_iColumnName).toString();
    if (sName == m_spProject->m_sTitleCard)
    {
      return QPixmap::fromImage(m_cardIcon.scaled(m_iIconSize, m_iIconSize));
    }
    if (sName == m_spProject->m_sPlayerLayout)
    {
      return QPixmap::fromImage(m_layoutIcon.scaled(m_iIconSize, m_iIconSize));
    }
    return QVariant();
  }
  // used for id to search
  else if (CResourceTreeItemModel::eSearchRole == iRole)
  {
    CResourceTreeItem* item = static_cast<CResourceTreeItem*>(index.internalPointer());
    return QString(item->Type()._to_string()) + ";" + item->Data(c_iColumnName).toString();
  }
  else if (CResourceTreeItemModel::eLoadedIDRole == iRole)
  {
    CResourceTreeItem* item = static_cast<CResourceTreeItem*>(index.internalPointer());
    return item->Data(c_iColumnLoadedID);
  }
  else if (CResourceTreeItemModel::eItemTypeRole == iRole)
  {
    CResourceTreeItem* item = static_cast<CResourceTreeItem*>(index.internalPointer());
    return item->Type()._to_integral();
  }
  else
  {
    return QVariant();
  }
}

//----------------------------------------------------------------------------------------
//
QVariant CResourceTreeItemModel::data(const QModelIndex& index, int iRole) const
{
  if (!index.isValid()) { return QVariant(); }
  if (Qt::DisplayRole == iRole || Qt::EditRole == iRole)
  {
    CResourceTreeItem* item = static_cast<CResourceTreeItem*>(index.internalPointer());
    return item->Data(index.column());
  }
  else if (Qt::DecorationRole == iRole)
  {
    QReadLocker locker(&m_spProject->m_rwLock);
    CResourceTreeItem* item = static_cast<CResourceTreeItem*>(index.internalPointer());
    QString sName = item->Data(c_iColumnName).toString();
    bool bIsSceneCard = false;
    for (const tspScene& spScene : m_spProject->m_baseData.m_vspScenes)
    {
      QReadLocker l(&spScene->m_rwLock);
      bIsSceneCard |= spScene->m_sTitleCard == sName;
    }
    if (sName == m_spProject->m_sTitleCard || bIsSceneCard)
    {
      return QPixmap::fromImage(m_cardIcon.scaled(m_iIconSize, m_iIconSize));
    }
    if (sName == m_spProject->m_sPlayerLayout)
    {
      return QPixmap::fromImage(m_layoutIcon.scaled(m_iIconSize, m_iIconSize));
    }
    return QVariant();
  }
  // used for id to search
  else if (CResourceTreeItemModel::eSearchRole == iRole)
  {
    CResourceTreeItem* item = static_cast<CResourceTreeItem*>(index.internalPointer());
    return QString(item->Type()._to_string()) + ";" + item->Data(c_iColumnName).toString();
  }
  else if (Qt::ToolTipRole == iRole)
  {
    CResourceTreeItem* item = static_cast<CResourceTreeItem*>(index.internalPointer());
    return item->Data(c_iColumnToolTip);
  }
  else if (CResourceTreeItemModel::eLoadedIDRole == iRole)
  {
    CResourceTreeItem* item = static_cast<CResourceTreeItem*>(index.internalPointer());
    return item->Data(c_iColumnLoadedID);
  }
  else if (CResourceTreeItemModel::eItemTypeRole == iRole)
  {
    CResourceTreeItem* item = static_cast<CResourceTreeItem*>(index.internalPointer());
    return item->Type()._to_integral();
  }
  else if(CResourceTreeItemModel::eItemWarningRole == iRole)
  {
    CResourceTreeItem* item = static_cast<CResourceTreeItem*>(index.internalPointer());
    return item->Data(c_iColumnWarning);
  }
  else
  {
    return QVariant();
  }
}

//----------------------------------------------------------------------------------------
//
Qt::ItemFlags CResourceTreeItemModel::flags(const QModelIndex& index) const
{
  if (!index.isValid()) { return Qt::NoItemFlags; }

  CResourceTreeItem* pItem = GetItem(index);
  if (nullptr != pItem)
  {
    if (pItem->Parent() != m_pRootItem && pItem->Parent() != nullptr)
    {
      switch (index.column())
      {
        case c_iColumnName:
        {
          Qt::ItemFlags iFlags = QAbstractItemModel::flags(index);
          if (pItem->Type()._to_integral() == EResourceTreeItemType::eResource)
          {
            iFlags |= Qt::ItemIsEditable;
          }
          return iFlags;
        }
        default: return QAbstractItemModel::flags(index);
      }
    }
    else
    {
      return QAbstractItemModel::flags(index);
    }
  }
  else
  {
    return QAbstractItemModel::flags(index);
  }
}

//----------------------------------------------------------------------------------------
//
QVariant CResourceTreeItemModel::headerData(
    int iSection, Qt::Orientation orientation, int iRole) const
{
  if (Qt::Horizontal == orientation && Qt::DisplayRole == iRole)
  {
    return m_pRootItem->HeaderData(iSection);
  }
  return QVariant();
}

//----------------------------------------------------------------------------------------
//
QModelIndex CResourceTreeItemModel::index(
    int iRow, int iColumn, const QModelIndex& parent) const
{
  // hasIndex checks if the values are in the valid ranges by using
  // rowCount and columnCount
  if (!hasIndex(iRow, iColumn, parent)) { return QModelIndex(); }

  CResourceTreeItem* pParentItem = GetItem(parent);
  CResourceTreeItem* pChildItem = pParentItem->Child(iRow);
  if (nullptr != pChildItem)
  {
      return createIndex(iRow, iColumn, pChildItem);
  }
  return QModelIndex();
}

//----------------------------------------------------------------------------------------
//
QModelIndex CResourceTreeItemModel::parent(const QModelIndex& index) const
{
  if (!index.isValid()) { return QModelIndex(); }

  CResourceTreeItem* pChildItem = GetItem(index);
  CResourceTreeItem* pParentItem = nullptr != pChildItem ? pChildItem->Parent() : nullptr;

  if (pParentItem == m_pRootItem || nullptr == pParentItem) { return QModelIndex(); }
  return createIndex(pParentItem->Row(), 0, pParentItem);
}

//----------------------------------------------------------------------------------------
//
int CResourceTreeItemModel::rowCount(const QModelIndex& parent) const
{
  const CResourceTreeItem* pParentItem = GetItem(parent);

  return nullptr != pParentItem ? pParentItem->ChildCount() : 0;
}

//----------------------------------------------------------------------------------------
//
int CResourceTreeItemModel::columnCount(const QModelIndex& parent) const
{
  if (parent.isValid())
  {
    return static_cast<CResourceTreeItem*>(parent.internalPointer())->ColumnCount();
  }
  return m_pRootItem->ColumnCount();
}

//----------------------------------------------------------------------------------------
//
bool CResourceTreeItemModel::setData(const QModelIndex& index, const QVariant& value,
                                     qint32 iRole)
{
  if (Qt::EditRole != iRole) { return false; }

  CResourceTreeItem* pItem = GetItem(index);

  if (nullptr != m_pUndoStack)
  {
    if (index.column() == resource_item::c_iColumnName &&
        CResourceTreeItemModel::eItemWarningRole != iRole)
    {
      m_pUndoStack->push(
          new CCommandChangeResourceData(this, m_spProject,
                                         pItem->Data(resource_item::c_iColumnName).toString(),
                                         index.column(),
                                         value));
    }

    return true;
  }
  else
  {
    bool bResult = pItem->SetData(index.column(), value);

    if (bResult)
    {
      emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
      emit SignalProjectEdited();
    }

    return bResult;
  }
}

//----------------------------------------------------------------------------------------
//
bool CResourceTreeItemModel::setHeaderData(qint32 iSection, Qt::Orientation orientation,
                   const QVariant& value, qint32 iRole)
{
  if (iRole != Qt::EditRole || orientation != Qt::Horizontal) { return false; }

  const bool bResult = m_pRootItem->SetData(iSection, value);

  if (bResult) { emit headerDataChanged(orientation, iSection, iSection); }
  return bResult;
}

//----------------------------------------------------------------------------------------
//
bool CResourceTreeItemModel::insertColumns(qint32 iPosition, qint32 iColumns,
                   const QModelIndex& parent)
{
  beginInsertColumns(parent, iPosition, iPosition + iColumns - 1);
  const bool success = m_pRootItem->InsertColumns(iPosition, iColumns);
  endInsertColumns();

  return success;
}

//----------------------------------------------------------------------------------------
//
bool CResourceTreeItemModel::removeColumns(qint32 iPosition, qint32 iColumns,
                   const QModelIndex& parent)
{
  beginRemoveColumns(parent, iPosition, iPosition + iColumns - 1);
  const bool success = m_pRootItem->RemoveColumns(iPosition, iColumns);
  endRemoveColumns();

  if (m_pRootItem->ColumnCount() == 0) { removeRows(0, rowCount()); }
  return success;
}

//----------------------------------------------------------------------------------------
//
bool CResourceTreeItemModel::insertRows(qint32 iPosition, qint32 iRows,
                const QModelIndex& parent)
{
  CResourceTreeItem* pParentItem = GetItem(parent);
  if (!pParentItem) { return false; }

  beginInsertRows(parent, iPosition, iPosition + iRows - 1);
  const bool success = pParentItem->InsertChildren(
        iPosition, iRows, m_pRootItem->ColumnCount());
  endInsertRows();

  return success;
}

//----------------------------------------------------------------------------------------
//
bool CResourceTreeItemModel::removeRows(qint32 iPosition, qint32 iRows,
                const QModelIndex& parent)
{
  CResourceTreeItem* pParentItem = GetItem(parent);
  if (nullptr == pParentItem) { return false; }

  beginRemoveRows(parent, iPosition, iPosition + iRows - 1);
  const bool success = pParentItem->RemoveChildren(iPosition, iRows);
  endRemoveRows();

  return success;
}

//----------------------------------------------------------------------------------------
//
bool CResourceTreeItemModel::IsResourceType(const QModelIndex& index)
{
  if (!index.isValid()) { return false; }

  CResourceTreeItem* pItem = GetItem(index);
  return pItem->Type()._to_integral() == EResourceTreeItemType::eResource;
}

//----------------------------------------------------------------------------------------
//
bool CResourceTreeItemModel::IsFolderType(const QModelIndex& index)
{
  if (!index.isValid()) { return false; }

  CResourceTreeItem* pItem = GetItem(index);
  return pItem->Type()._to_integral() == EResourceTreeItemType::eFolder;
}

//----------------------------------------------------------------------------------------
//
QModelIndex CResourceTreeItemModel::IndexForResource(const tspResource& spResource)
{
  if (nullptr == m_spProject || nullptr == spResource) { return QModelIndex(); }

  m_spProject->m_rwLock.lockForRead();
  qint32 iThisId = m_spProject->m_iId;
  m_spProject->m_rwLock.unlock();

  spResource->m_rwLock.lockForRead();
  const QString sName = spResource->m_sName;
  spResource->m_spParent->m_rwLock.lockForRead();
  qint32 iResourceId = spResource->m_spParent->m_iId;
  spResource->m_spParent->m_rwLock.unlock();
  spResource->m_rwLock.unlock();

  if (iResourceId != iThisId) { return QModelIndex(); }

  QModelIndexList indices =
    QAbstractItemModel::match(createIndex(0, 0, m_pRootItem), CResourceTreeItemModel::eSearchRole,
                              QString(EResourceTreeItemType((EResourceTreeItemType::eResource))._to_string()) +
                                ";" + sName, 1,
                              Qt::MatchStartsWith | Qt::MatchWrap | Qt::MatchRecursive);
  if (0 < indices.size())
  {
    return indices.first();
  }
  return QModelIndex();
}

//----------------------------------------------------------------------------------------
//
tspResource CResourceTreeItemModel::ResourceForIndex(const QModelIndex& idx)
{
  if (nullptr == m_spProject || !idx.isValid()) { return nullptr; }
  CResourceTreeItem* pItem = GetItem(idx);
  if (pItem == nullptr) { return nullptr; }

  if (EResourceTreeItemType::eResource == pItem->Type()._to_integral())
  {
    return pItem->Resource();
  }
  return nullptr;
}

//----------------------------------------------------------------------------------------
//
void CResourceTreeItemModel::SlotProjectPropertiesEdited()
{
  if (nullptr!= m_spProject)
  {
    std::vector<QString> vsChanged;
    m_spProject->m_rwLock.lockForRead();
    vsChanged.push_back(m_sOldProjectLayoutResource);
    vsChanged.push_back(m_sOldProjectTitleResource);
    vsChanged.push_back(m_spProject->m_sPlayerLayout);
    vsChanged.push_back(m_spProject->m_sTitleCard);
    m_sOldProjectLayoutResource = m_spProject->m_sPlayerLayout;
    m_sOldProjectTitleResource = m_spProject->m_sTitleCard;
    m_spProject->m_rwLock.unlock();

    for (const QString& sResource : vsChanged)
    {
      QModelIndexList indices =
          QAbstractItemModel::match(createIndex(0, 0, m_pRootItem), CResourceTreeItemModel::eSearchRole,
                                    QString(EResourceTreeItemType((EResourceTreeItemType::eResource))._to_string()) +
                                        ";" + sResource, 1,
                                    Qt::MatchStartsWith | Qt::MatchWrap | Qt::MatchRecursive);
      if (0 < indices.size())
      {
        emit dataChanged(indices[0], indices[0], QVector<int>() << Qt::DecorationRole);
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CResourceTreeItemModel::CheckChildResources(CResourceTreeItem* pParent)
{
  for (qint32 i = 0; pParent->ChildCount() > i; ++i)
  {
    auto pChild = pParent->Child(i);
    if (pChild->Type()._to_integral() == EResourceTreeItemType::eResource)
    {
      auto spResource = pChild->Resource();
      QReadLocker l(&spResource->m_rwLock);
      bool bExists = QFileInfo::exists(spResource->ResourceToAbsolutePath());
      if (!bExists)
      {
        pChild->AddWarning(EWarningType::eMissingResource,
                           QCoreApplication::tr("Media for Resource %1 missing.", "CResourceTreeItemModel")
                               .arg(spResource->m_sName));
        QModelIndex idx = createIndex(i, resource_item::c_iColumnName, pChild);
        emit dataChanged(idx, idx, {Qt::DisplayRole, CResourceTreeItemModel::eItemWarningRole});
      }
      else
      {
        bool bChanged = pChild->ClearWarning(EWarningType::eMissingResource);
        if (bChanged)
        {
          QModelIndex idx = createIndex(i, resource_item::c_iColumnName, pChild);
          emit dataChanged(idx, idx, {Qt::DisplayRole, CResourceTreeItemModel::eItemWarningRole});
        }
      }
    }
    CheckChildResources(pChild);
  }
}

//----------------------------------------------------------------------------------------
//
CResourceTreeItem* CResourceTreeItemModel::GetItem(const QModelIndex& index) const
{
  if (index.isValid())
  {
    CResourceTreeItem* pItem = static_cast<CResourceTreeItem*>(index.internalPointer());
    if (nullptr != pItem) { return pItem; }
  }
  return m_pRootItem;
}

//----------------------------------------------------------------------------------------
//
void CResourceTreeItemModel::SlotResourceCheckerTimeout()
{
  if (nullptr != m_pRootItem)
  {
    CheckChildResources(m_pRootItem);
  }
}

//----------------------------------------------------------------------------------------
//
void CResourceTreeItemModel::SlotResourceAdded(qint32 iProjId, const QString& sName)
{
  if (nullptr!= m_spProject)
  {
    m_spProject->m_rwLock.lockForRead();
    qint32 iThisId = m_spProject->m_iId;
    m_spProject->m_rwLock.unlock();

    if (iProjId == iThisId)
    {
      auto spDbManager = m_wpDbManager.lock();
      tspResource spResource = spDbManager->FindResourceInProject(m_spProject, sName);
      spResource->m_rwLock.lockForRead();
      EResourceType type = spResource->m_type;
      SResourcePath path = spResource->m_sPath;
      spResource->m_rwLock.unlock();

      QStringList sPathParts;
      if (path.IsLocalFile())
      {
        sPathParts = spResource->ResourceToAbsolutePath().split("/");
        sPathParts.removeAt(0); // first element is always the scheme
      }
      else
      {
        QUrl url = static_cast<QUrl>(path);
        sPathParts.push_back(url.host());
        sPathParts << url.path().remove(0, 1).split("/");
      }

      auto itCategoryItem = m_categoryMap.find(type);
      if (m_categoryMap.end() != itCategoryItem)
      {
        // iterate over url subfolders
        qint32 indexOfItem = m_pRootItem->ChildIndex(itCategoryItem->second);
        CResourceTreeItem* pItem = itCategoryItem->second;
        CResourceTreeItem* pChild = nullptr;

        if (1 < sPathParts.size())
        {
          for (qint32 iCurrentPart = 0; sPathParts.size() > iCurrentPart; ++iCurrentPart)
          {
            const QString& sPathPart = sPathParts[iCurrentPart];
            QUrl sFolderName = QUrl(sPathPart);
            QString sDisplayFolderName = sFolderName.toString(QUrl::RemoveScheme);

            // iterate over children of current folder
            bool bFoundFolder = false;
            for (qint32 i = 0; i < pItem->ChildCount(); ++i)
            {
              pChild = pItem->Child(i);
              if (pChild->Type()._to_integral() == EResourceTreeItemType::eFolder &&
                  pChild->Data(c_iColumnName).toString() == sDisplayFolderName)
              {
                bFoundFolder = true;
                indexOfItem = i;
                pItem = pChild;
                break;
              }
            }
            // folder not found -> create
            if (!bFoundFolder && iCurrentPart != sPathParts.size() - 1)
            {
              QModelIndex parent =
                createIndex(indexOfItem, 0, pItem);
              qint32 iPosition = pItem->ChildCount();

              beginInsertRows(parent, iPosition, iPosition);
              const bool bSuccess = pItem->InsertChildren(
                    iPosition, 1, m_pRootItem->ColumnCount());
              if (bSuccess)
              {
                CResourceTreeItem* pNewItem = pItem->Child(pItem->ChildCount() - 1);
                pNewItem->SetType(EResourceTreeItemType::eFolder);
                pNewItem->SetLabel(sDisplayFolderName);
                indexOfItem = iPosition;
                pItem = pNewItem;
              }
              endInsertRows();
            }
          }
        }

        QModelIndex parent =
          createIndex(indexOfItem, 0, pItem);
        qint32 iPosition = pItem->ChildCount();

        beginInsertRows(parent, iPosition, iPosition);
        const bool bSuccess = pItem->InsertChildren(
              iPosition, 1, m_pRootItem->ColumnCount());
        if (bSuccess)
        {
          CResourceTreeItem* pNewItem = pItem->Child(pItem->ChildCount() - 1);
          pNewItem->SetType(EResourceTreeItemType::eResource);
          pNewItem->SetResource(spResource);
        }
        endInsertRows();
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CResourceTreeItemModel::SlotResourceRemoved(qint32 iProjId, const QString& sName)
{
  if (nullptr!= m_spProject)
  {
    m_spProject->m_rwLock.lockForRead();
    qint32 iThisId = m_spProject->m_iId;
    m_spProject->m_rwLock.unlock();

    if (iProjId == iThisId)
    {

      QModelIndexList indices =
        QAbstractItemModel::match(createIndex(0, 0, m_pRootItem), CResourceTreeItemModel::eSearchRole,
                                  QString(EResourceTreeItemType((EResourceTreeItemType::eResource))._to_string()) +
                                    ";" + sName, 1,
                                  Qt::MatchStartsWith | Qt::MatchWrap | Qt::MatchRecursive);
      if (0 < indices.size())
      {
        CResourceTreeItem* pItem = GetItem(indices[0]);
        CResourceTreeItem* pParent = pItem->Parent();
        qint32 iPosition = pParent->ChildIndex(pItem);
        qint32 iParentIndex = pParent->Parent()->ChildIndex(pParent);

        // remove item
        QModelIndex parentIndex =
          createIndex(iParentIndex, 0, pParent);
        beginRemoveRows(parentIndex, iPosition, iPosition);
        const bool bSuccess = pParent->RemoveChildren(iPosition, 1);
        Q_UNUSED(bSuccess);
        endRemoveRows();

        pItem = pParent;
        pParent = pParent->Parent();
        iPosition = pParent->ChildIndex(pItem);
        if (nullptr != pParent->Parent())
        {
          iParentIndex = pParent->Parent()->ChildIndex(pParent);
        }
        while (pParent != m_pRootItem)
        {

          if (pItem->ChildCount() == 0)
          {
            parentIndex = createIndex(iParentIndex, 0, pParent);
            beginRemoveRows(parentIndex, iPosition, iPosition);
            const bool bSuccess = pParent->RemoveChildren(iPosition, 1);
            Q_UNUSED(bSuccess);
            endRemoveRows();
          }

          pItem = pParent;
          pParent = pParent->Parent();
          iPosition = pParent->ChildIndex(pItem);
          if (nullptr != pParent->Parent())
          {
            iParentIndex = pParent->Parent()->ChildIndex(pParent);
          }
        }
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CResourceTreeItemModel::SlotSceneDataChanged(qint32 iProjId, qint32 iId)
{
  if (nullptr!= m_spProject)
  {
    m_spProject->m_rwLock.lockForRead();
    qint32 iThisId = m_spProject->m_iId;
    QString sResourceName;
    for (const tspScene& spScene : m_spProject->m_baseData.m_vspScenes)
    {
      QReadLocker l(&spScene->m_rwLock);
      if (spScene->m_iId == iId)
      {
        sResourceName = spScene->m_sTitleCard;
      }
    }
    m_spProject->m_rwLock.unlock();

    if (iProjId == iThisId)
    {
      QModelIndexList indices =
          QAbstractItemModel::match(createIndex(0, 0, m_pRootItem), CResourceTreeItemModel::eSearchRole,
                                    QString(EResourceTreeItemType((EResourceTreeItemType::eResource))._to_string()) +
                                        ";" + sResourceName, 1,
                                    Qt::MatchStartsWith | Qt::MatchWrap | Qt::MatchRecursive);
      for (const QModelIndex& idx : indices)
      {
        emit dataChanged(idx, idx, {Qt::DecorationRole});
      }
    }
  }
}
