#include "DialogueEditorTreeModel.h"
#include "DialogueEditorTreeItem.h"

#include "Application.h"

#include "Systems/DialogueTree.h"
#include "Systems/Resource.h"

#include <QFileInfo>
#include <QFont>
#include <QPointer>
#include <QUndoStack>

#include <functional>

CDialogueEditorTreeModel::CDialogueEditorTreeModel(QObject* pParent) :
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>()),
  m_spProject(),
  m_pRootItem(nullptr)
{
  auto spDbManager = m_wpDbManager.lock();
  connect(spDbManager.get(), &CDatabaseManager::SignalResourceAdded,
          this, &CDialogueEditorTreeModel::SlotResourceAdded, Qt::QueuedConnection);
  connect(spDbManager.get(), &CDatabaseManager::SignalResourceRemoved,
          this, &CDialogueEditorTreeModel::SlotResourceRemoved, Qt::QueuedConnection);
}

CDialogueEditorTreeModel::~CDialogueEditorTreeModel()
{
  DeInitializeModel();
}

//----------------------------------------------------------------------------------------
//
void CDialogueEditorTreeModel::InitializeModel(tspProject spProject)
{
  beginResetModel();

  m_spProject = spProject;

  std::vector<tspResource> vsResFiles;
  {
    QReadLocker lock(&spProject->m_rwLock);
    for (const auto& [_, spResource] : spProject->m_spResourcesMap)
    {
      QReadLocker rLock(&spResource->m_rwLock);
      if (EResourceType::eDatabase == spResource->m_type._to_integral() &&
          QFileInfo(PhysicalResourcePath(spResource)).suffix() == joip_resource::c_sDialogueFileType)
      {
        vsResFiles.push_back(spResource);
      }
    }
  }

  if (nullptr != m_pRootItem)
  {
    delete m_pRootItem;
    m_pRootItem = nullptr;
  }

  m_spDataRootNode = dialogue_tree::LoadDialogues(vsResFiles);
  if (nullptr == m_spDataRootNode)
  {
    m_spDataRootNode = std::make_shared<CDialogueNode>();
  }

  m_pRootItem = new CDialogueEditorTreeItem(m_spDataRootNode, nullptr);
  BuildTreeItems(m_pRootItem, m_spDataRootNode);

  endResetModel();
}

//----------------------------------------------------------------------------------------
//
void CDialogueEditorTreeModel::DeInitializeModel()
{
  beginResetModel();

  if (nullptr != m_pRootItem)
  {
    delete m_pRootItem;
    m_pRootItem = nullptr;
  }

  m_spDataRootNode = nullptr;

  m_spProject = nullptr;

  endResetModel();
}

//----------------------------------------------------------------------------------------
//
QVariant CDialogueEditorTreeModel::data(const QModelIndex& index, int iRole, int iColumnOverride) const
{
  if (!index.isValid()) { return QVariant(); }
  return CDialogueEditorTreeModel::data(
      this->index(index.row(), iColumnOverride, index.parent()), iRole);
}

//----------------------------------------------------------------------------------------
//
QVariant CDialogueEditorTreeModel::data(const QModelIndex& index, int iRole) const
{
  if (!index.isValid()) { return QVariant(); }
  CDialogueEditorTreeItem* item = static_cast<CDialogueEditorTreeItem*>(index.internalPointer());
  if (Qt::ToolTipRole == iRole)
  {
    return GetToolTip(index);
  }
  else if (CDialogueEditorTreeModel::eSearchRole == iRole)
  {
    using namespace dialogue_item;
    return item->Data(c_iColumnMedia).toString() + ";" +
           item->Data(c_iColumnCondition).toString()  + ";" +
           item->Data(c_iColumnString).toString()  + ";" +
           QString(item->Type()._to_string())  + ";" +
           item->Data(c_iColumnResource).toString();
  }
  else if (CDialogueEditorTreeModel::eSearchRole < iRole &&
           CDialogueEditorTreeModel::eItemWarningRole >= iRole)
  {
    return item->Data(iRole - (CDialogueEditorTreeModel::eSearchRole+1) + dialogue_item::c_iNumColumns);
  }
  else if (Qt::DisplayRole == iRole || Qt::EditRole == iRole || Qt::CheckStateRole == iRole)
  {
    if (dialogue_item::c_iColumnString == index.column())
    {
      if (Qt::DisplayRole == iRole)
      {
        return item->Data(dialogue_item::c_iColumnDisplayString);
      }
      else if (Qt::EditRole == iRole)
      {
        return item->Data(dialogue_item::c_iColumnString);
      }
      else
      {
        return QVariant();
      }
    }

    bool bIsCheckable = item->Flags(index.column()).testFlag(Qt::ItemIsUserCheckable);
    if (Qt::CheckStateRole == iRole)
    {
      if (bIsCheckable)
      {
        return item->Data(index.column());
      }
      else
      {
        return QVariant();
      }
    }
    else
    {
      if (!bIsCheckable)
      {
        return item->Data(index.column());
      }
      else
      {
        return QVariant();
      }
    }
  }
  return QVariant();
}

//----------------------------------------------------------------------------------------
//
Qt::ItemFlags CDialogueEditorTreeModel::flags(const QModelIndex& index) const
{
  if (!index.isValid()) { return Qt::NoItemFlags; }
  CDialogueEditorTreeItem* pItem = GetItem(index);
  if (nullptr != pItem)
  {
    return pItem->Flags(index.column());
  }
  else
  {
    return QAbstractItemModel::flags(index);
  }
}

//----------------------------------------------------------------------------------------
//
QVariant CDialogueEditorTreeModel::headerData(int iSection, Qt::Orientation orientation, int iRole) const
{
  if (Qt::Horizontal == orientation && Qt::DisplayRole == iRole)
  {
    return m_pRootItem->HeaderData(iSection);
  }
  return QVariant();
}

//----------------------------------------------------------------------------------------
//
QModelIndex CDialogueEditorTreeModel::index(int iRow, int iColumn, const QModelIndex& parent) const
{
  // hasIndex checks if the values are in the valid ranges by using
  // rowCount and columnCount
  if (!hasIndex(iRow, iColumn, parent)) { return QModelIndex(); }

  CDialogueEditorTreeItem* pParentItem = GetItem(parent);
  CDialogueEditorTreeItem* pChildItem = pParentItem->Child(iRow);
  if (nullptr != pChildItem)
  {
    return createIndex(iRow, iColumn, pChildItem);
  }
  return QModelIndex();
}

//----------------------------------------------------------------------------------------
//
QModelIndex CDialogueEditorTreeModel::parent(const QModelIndex& index) const
{
  if (!index.isValid()) { return QModelIndex(); }

  CDialogueEditorTreeItem* pChildItem = GetItem(index);
  CDialogueEditorTreeItem* pParentItem = nullptr != pChildItem ? pChildItem->Parent() : nullptr;

  if (pParentItem == m_pRootItem || nullptr == pParentItem) { return QModelIndex(); }
  return createIndex(pParentItem->Row(), 0, pParentItem);
}

//----------------------------------------------------------------------------------------
//
int CDialogueEditorTreeModel::rowCount(const QModelIndex& parent) const
{
  const CDialogueEditorTreeItem* pParentItem = GetItem(parent);
  if (nullptr == pParentItem) { return 0; }
  if (IsDialogueType(parent))
  {
    return HasCondition(parent) ? pParentItem->ChildCount() : 0;
  }
  return pParentItem->ChildCount();
}

//----------------------------------------------------------------------------------------
//
int CDialogueEditorTreeModel::columnCount(const QModelIndex& parent) const
{
  if (parent.isValid())
  {
    return static_cast<CDialogueEditorTreeItem*>(parent.internalPointer())->ColumnCount();
  }
  return m_pRootItem->ColumnCount();
}

//----------------------------------------------------------------------------------------
//
bool CDialogueEditorTreeModel::setData(const QModelIndex& index, const QVariant& value, qint32 iRole)
{
  if (!(Qt::EditRole == iRole || Qt::CheckStateRole == iRole ||
        (CDialogueEditorTreeModel::eItemWarningRole >= iRole &&
         CDialogueEditorTreeModel::eSearchRole < iRole)) ||
      CDialogueEditorTreeModel::eTypeRole == iRole) { return false; }

  CDialogueEditorTreeItem* pItem = GetItem(index);
  bool bResult = false;
  if (Qt::EditRole == iRole || Qt::CheckStateRole == iRole)
  {
    bResult = pItem->SetData(index.column(), value);
  }
  else
  {
    bResult = pItem->SetData(iRole - CDialogueEditorTreeModel::eSearchRole+1 + dialogue_item::c_iNumColumns, value);
  }

  if (bResult)
  {
    emit dataChanged(index, index, {Qt::DisplayRole, iRole});

    if (IsDialogueType(index))
    {
      QModelIndex tl = this->index(0, index.column(), index);
      QModelIndex br = this->index(rowCount(index)-1, index.column(), index);
      emit dataChanged(tl, br, {Qt::DisplayRole, iRole});
    }
    else if (IsDialogueFragmentType(index))
    {
      QModelIndex par = parent(index);
      emit dataChanged(par, par, {Qt::DisplayRole, iRole});
    }

    emit SignalProjectEdited();
  }

  return bResult;
}

//----------------------------------------------------------------------------------------
//
bool CDialogueEditorTreeModel::setHeaderData(qint32 iSection, Qt::Orientation orientation, const QVariant& value, qint32 iRole)
{
  if (iRole != Qt::EditRole || orientation != Qt::Horizontal) { return false; }

  const bool bResult = m_pRootItem->SetData(iSection, value);

  if (bResult) { emit headerDataChanged(orientation, iSection, iSection); }
  return bResult;
}

//----------------------------------------------------------------------------------------
//
bool CDialogueEditorTreeModel::insertRow(qint32 iPosition, EDialogueTreeNodeType type,
                                       const QString& sName,
                                       const QModelIndex& parent)
{
  CDialogueEditorTreeItem* pParentItem = GetItem(parent);
  if (!pParentItem) { return false; }

  beginInsertRows(parent, iPosition, iPosition);
  const bool success = pParentItem->InsertChildren(
      iPosition, 1, m_pRootItem->ColumnCount());

  if (success)
  {
    auto spNode = pParentItem->Node();
    std::shared_ptr<CDialogueNode> spNewNode = nullptr;
    switch (type)
    {
      case EDialogueTreeNodeType::eRoot:
        spNewNode = std::make_shared<CDialogueNode>();
        break;
      case EDialogueTreeNodeType::eCategory:
        spNewNode = std::make_shared<CDialogueNodeCategory>();
        break;
      case EDialogueTreeNodeType::eDialogue:
        spNewNode = std::make_shared<CDialogueNodeDialogue>();
        break;
      case EDialogueTreeNodeType::eDialogueFragment:
        spNewNode = std::make_shared<CDialogueData>();
        break;
    }
    spNewNode->m_wpParent = spNode;
    spNewNode->m_sName = dialogue_tree::EnsureUniqueName(sName, spNode, spNewNode);

    if (iPosition >= spNode->m_vspChildren.size())
    {
      spNode->m_vspChildren.push_back(spNewNode);
    }
    else if (0 <= iPosition)
    {
      spNode->m_vspChildren.insert(spNode->m_vspChildren.begin()+iPosition,
                                   spNewNode);
    }

    CDialogueEditorTreeItem* pChildItem = pParentItem->Child(iPosition);
    pChildItem->SetNode(spNewNode);
  }

  endInsertRows();

  return success;
}

//----------------------------------------------------------------------------------------
//
bool CDialogueEditorTreeModel::insertNode(qint32 iPosition,
                                        const std::shared_ptr<CDialogueNode>& spNode,
                                        const QModelIndex& parent)
{
  CDialogueEditorTreeItem* pParentItem = GetItem(parent);
  if (!pParentItem) { return false; }

  auto spParentNode = pParentItem->Node();

  if (0 > iPosition)
  {
    iPosition = static_cast<qint32>(spParentNode->m_vspChildren.size());
  }

  spNode->m_sName = dialogue_tree::EnsureUniqueName(spNode->m_sName, spParentNode, spNode);

  beginResetModel();
  if (spParentNode->m_vspChildren.size() <= static_cast<size_t>(iPosition))
  {
    spParentNode->m_vspChildren.push_back(spNode);
  }
  else
  {
    spParentNode->m_vspChildren.insert(spParentNode->m_vspChildren.begin()+iPosition, spNode);
  }
  spNode->m_wpParent = spParentNode;

  if (nullptr != m_pRootItem)
  {
    delete m_pRootItem;
    m_pRootItem = nullptr;
  }

  m_pRootItem = new CDialogueEditorTreeItem(m_spDataRootNode, nullptr);
  BuildTreeItems(m_pRootItem, m_spDataRootNode);

  endResetModel();
  return true;
}

//----------------------------------------------------------------------------------------
//
bool CDialogueEditorTreeModel::removeRow(qint32 iPosition, const QModelIndex& parent)
{
  CDialogueEditorTreeItem* pParentItem = GetItem(parent);
  if (nullptr == pParentItem) { return false; }

  beginRemoveRows(parent, iPosition, iPosition);

  auto spNode = pParentItem->Node();

  const bool success = pParentItem->RemoveChildren(iPosition, 1);

  if (success)
  {
    spNode->m_vspChildren.erase(spNode->m_vspChildren.begin()+iPosition);
  }

  endRemoveRows();

  return success;
}

//----------------------------------------------------------------------------------------
//
bool CDialogueEditorTreeModel::HasCondition(const QModelIndex& index) const
{
  CDialogueEditorTreeItem* pParentItem = GetItem(index);
  if (nullptr == pParentItem) { return false; }

  auto spNode = std::dynamic_pointer_cast<CDialogueNodeDialogue>(pParentItem->Node());
  if (nullptr == spNode) { return false; }

  return spNode->m_bHasCondition;
}

//----------------------------------------------------------------------------------------
//
QModelIndex CDialogueEditorTreeModel::Index(const QStringList& vsPath) const
{
  QStringList vsCopy = vsPath;
  QModelIndex idx;
  CDialogueEditorTreeItem* pItem = m_pRootItem;
  while (!vsCopy.isEmpty() && nullptr != m_pRootItem)
  {
    const QString sElem = vsCopy[0];
    vsCopy.erase(vsCopy.begin());
    for (qint32 i = 0; pItem->ChildCount() > i; ++i)
    {
      auto pChild = pItem->Child(i);
      if (pChild->Name() == sElem)
      {
        pItem = pChild;
        idx = createIndex(i, 0, pItem);
        break;
      }
    }
  }
  return idx;
}

//----------------------------------------------------------------------------------------
//
bool CDialogueEditorTreeModel::IsCategoryType(const QModelIndex& index) const
{
  if (!index.isValid()) { return false; }
  CDialogueEditorTreeItem* pItem = GetItem(index);
  return pItem->Type()._to_integral() == EDialogueTreeNodeType::eCategory;
}

//----------------------------------------------------------------------------------------
//
bool CDialogueEditorTreeModel::IsDialogueType(const QModelIndex& index) const
{
  if (!index.isValid()) { return false; }
  CDialogueEditorTreeItem* pItem = GetItem(index);
  return pItem->Type()._to_integral() == EDialogueTreeNodeType::eDialogue;
}

//----------------------------------------------------------------------------------------
//
bool CDialogueEditorTreeModel::IsDialogueFragmentType(const QModelIndex& index) const
{
  if (!index.isValid()) { return false; }
  CDialogueEditorTreeItem* pItem = GetItem(index);
  return pItem->Type()._to_integral() == EDialogueTreeNodeType::eDialogueFragment;
}

//----------------------------------------------------------------------------------------
//
QStringList CDialogueEditorTreeModel::Path(QModelIndex idx) const
{
  auto spNode = Node(idx);
  QStringList vsPath;
  while (nullptr != spNode && spNode != m_spDataRootNode)
  {
    vsPath.push_front(spNode->m_sName);
    spNode = spNode->m_wpParent.lock();
  }
  return vsPath;
}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<CDialogueNode> CDialogueEditorTreeModel::Node(QModelIndex idx) const
{
  auto pItem = GetItem(idx);
  if (nullptr == pItem || !idx.isValid()) { return nullptr; }
  return pItem->Node();
}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<CDialogueNode> CDialogueEditorTreeModel::Root() const
{
  return m_spDataRootNode;
}

//----------------------------------------------------------------------------------------
//
namespace
{
  void UpdateChild(std::shared_ptr<CDialogueNode>& spCopyTo,
                   const std::shared_ptr<CDialogueNode>& spCopyFrom)
  {
    spCopyTo->CopyFrom(spCopyFrom);
    for (qint32 i = 0; spCopyFrom->m_vspChildren.size() > static_cast<size_t>(i); ++i)
    {
      if (spCopyTo->m_vspChildren.size() > static_cast<size_t>(i))
      {
        UpdateChild(spCopyTo->m_vspChildren[i], spCopyFrom->m_vspChildren[i]);
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CDialogueEditorTreeModel::UpdateFrom(const QModelIndex& idx, const std::shared_ptr<CDialogueNode>& spNode)
{
  auto pItem = GetItem(idx);
  if (nullptr == pItem || !idx.isValid()) { return; }

  auto spNodeOld = pItem->Node();
  spNodeOld->CopyFrom(spNode);

  for (qint32 i = 0; spNode->m_vspChildren.size() > static_cast<size_t>(i); ++i)
  {
    if (rowCount(idx) > i)
    {
      UpdateFrom(index(i, idx.column(), idx), spNode->m_vspChildren[static_cast<size_t>(i)]);
    }
    else if (spNodeOld->m_vspChildren.size() > static_cast<size_t>(i))
    {
      UpdateChild(spNodeOld->m_vspChildren[i], spNode->m_vspChildren[i]);
    }
  }

  QModelIndex parentIndex = idx.parent();

  for (qint32 i = 0; columnCount(parentIndex) > i; ++i)
  {
    QModelIndex idx2 = index(idx.row(), i, parentIndex);
    emit dataChanged(idx2, idx2, {Qt::DisplayRole, Qt::EditRole, Qt::CheckStateRole, Qt::ToolTipRole});
  }
  emit SignalProjectEdited();
}

//----------------------------------------------------------------------------------------
//
void CDialogueEditorTreeModel::BuildTreeItems(CDialogueEditorTreeItem* pLocalRoot,
                                            const std::shared_ptr<CDialogueNode>& spNode)
{
  for (const auto& spChildNode : spNode->m_vspChildren)
  {
    CDialogueEditorTreeItem* pChild = new CDialogueEditorTreeItem(spChildNode,
                                                              pLocalRoot);
    pLocalRoot->AppendChild(pChild);
    BuildTreeItems(pChild, spChildNode);
  }
}

//----------------------------------------------------------------------------------------
//
CDialogueEditorTreeItem* CDialogueEditorTreeModel::GetItem(const QModelIndex& index) const
{
  if (index.isValid())
  {
    CDialogueEditorTreeItem* pItem = static_cast<CDialogueEditorTreeItem*>(index.internalPointer());
    if (nullptr != pItem) { return pItem; }
  }
  return m_pRootItem;
}

//----------------------------------------------------------------------------------------
//
QString CDialogueEditorTreeModel::GetToolTip(const QModelIndex& index) const
{
  QFont font = CApplication::Instance()->font();
  QString sFontFace = font.family();
  qint32 iFontsize = font.pointSize();

  CDialogueEditorTreeItem* item = static_cast<CDialogueEditorTreeItem*>(index.internalPointer());
  auto spNode = item->Node();

  auto fnDialogueData = [](const std::shared_ptr<CDialogueData>& spDial, bool bNoCond)
  {
    QString sCond = spDial->m_sCondition;
    if (sCond.isEmpty()) { sCond = "<nobr>&lt;No Condition&gt;</nobr>"; }
    else if (sCond.contains("\n"))
    {
      auto vsLines = sCond.split("\n");
      sCond = vsLines.size() > 0 ? vsLines[0] : "<nobr>&lt;No Condition&gt;</nobr>";
    }
    if (sCond.length() > 70)
    {
      sCond = sCond.left(70) + "...";
    }
    QString sDetail = "<nobr>Sleep Ms: %1, Skippable: %2</nobr><br>"
                      "<nobr>Sound Resource: %3</nobr><br>";
    sDetail = sDetail.arg(spDial->m_iWaitTimeMs)
                     .arg(spDial->m_bSkipable ? "true" : "false")
                     .arg(spDial->m_sSoundResource.isEmpty() ? "&lt;no resource&gt;" : spDial->m_sSoundResource);
    if (!bNoCond)
    {
      sDetail +=
          QString("Condition: %1<br>").arg(sCond);
    }
    return sDetail;
  };

  QString sRet;
  switch (spNode->m_type)
  {
    case EDialogueTreeNodeType::eRoot:
      return QString();
    case EDialogueTreeNodeType::eCategory:
      sRet = "<p style=\"font-family:'%1';font-size:%2px\">"
             "<nobr>Category: %3</nobr>"
             "</p>";
      sRet = sRet.arg(sFontFace).arg(iFontsize).arg(spNode->m_sName);
      break;
    case EDialogueTreeNodeType::eDialogue: [[fallthrough]];
    case EDialogueTreeNodeType::eDialogueFragment:
    {
      sRet =
          "<p style=\"font-family:'%1';font-size:%2px\">"
          "<nobr>Resource: %3</nobr><br>"
          "%4"
          "<table><tr><td>Tags:</td><td>%6</td></tr>"
          "</p>";
      sRet = sRet.arg(sFontFace).arg(iFontsize)
                 .arg(spNode->m_sFileId);
      if (EDialogueTreeNodeType::eDialogue == spNode->m_type._to_integral())
      {
        auto spDial = std::static_pointer_cast<CDialogueNodeDialogue>(spNode);
        if (!spDial->m_bHasCondition && spDial->m_vspChildren.size() > 0)
        {
          auto spDialFrag = std::static_pointer_cast<CDialogueData>(spDial->m_vspChildren[0]);
          sRet = sRet.arg(fnDialogueData(spDialFrag, true));
        }
        else
        {
          qint32 iCheckState = item->Data(dialogue_item::c_iColumnSkippable).toInt();
          QString sDetail = "<nobr>Sleep Ms: %1, Skippable: %2</nobr><br>";
          sDetail = sDetail.arg(item->Data(dialogue_item::c_iColumnWaitMS).toLongLong())
              .arg(iCheckState == Qt::Unchecked ?
                   "false" : (iCheckState == 1 ? "varying" : "true"));
          sRet = sRet.arg(sDetail);
        }
      }
      else if (EDialogueTreeNodeType::eDialogueFragment == spNode->m_type._to_integral())
      {
        auto spDial = std::static_pointer_cast<CDialogueData>(spNode);
        sRet = sRet.arg(fnDialogueData(spDial, false));
      }
    } break;
  }

  qint32 iCounter = 0;
  if (EDialogueTreeNodeType::eDialogue == spNode->m_type._to_integral() ||
      EDialogueTreeNodeType::eDialogueFragment == spNode->m_type._to_integral())
  {
    std::shared_ptr<CDialogueNodeDialogue> spDial = nullptr;
    if (EDialogueTreeNodeType::eDialogue == spNode->m_type._to_integral())
    {
      spDial = std::static_pointer_cast<CDialogueNodeDialogue>(spNode);
    }
    else if (EDialogueTreeNodeType::eDialogueFragment == spNode->m_type._to_integral())
    {
      spDial = std::static_pointer_cast<CDialogueNodeDialogue>(spNode->m_wpParent.lock());
    }

    QString sTags = nullptr == spDial || spDial->m_tags.empty() ? "&lt;no tags&gt;" : "";
    if (nullptr != spDial)
    {
      for (const auto& [sTag, spTag] : spDial->m_tags)
      {
        QColor colBg = Qt::transparent;
        QColor colText;
        {
          QReadLocker tagLocker(&spTag->m_rwLock);
          colBg = CalculateTagColor(*spTag);

          // calculate foreground / text color
          double dLuminance = (0.299 * colBg.red() +
                               0.587 * colBg.green() +
                               0.114 * colBg.blue()) / 255;
          colText = Qt::white;
          if (dLuminance > 0.5)
          {
            colText = Qt::black;
          }
        }

        QString sTagFormated = sTag;
        if (colText.isValid())
        {
          sTagFormated =
              QString("<span style=\"border-radius:3px;background:%1;color:%2\">%3</span>")
                  .arg(colBg.name(QColor::HexRgb)).arg(colText.name(QColor::HexRgb))
                  .arg(sTag);
        }

        if (iCounter % 5 == 0 && 0 != iCounter)
        {
          sTags += sTagFormated + "<br>";
        }
        else if (spDial->m_tags.size()-1 == iCounter)
        {
          sTags += sTagFormated;
        }
        else
        {
          sTags += sTagFormated + ", ";
        }
        ++iCounter;
      }
    }

    sRet = sRet.arg(sTags);
  }

  return sRet;
}

//----------------------------------------------------------------------------------------
//
void CDialogueEditorTreeModel::SlotResourceAdded(qint32 iProjId, const QString& sName)
{
  if (nullptr != m_spProject)
  {
    m_spProject->m_rwLock.lockForRead();
    qint32 iThisId = m_spProject->m_iId;
    m_spProject->m_rwLock.unlock();

    auto spDbManager = m_wpDbManager.lock();
    if (iProjId == iThisId && nullptr != spDbManager)
    {
      tspResource spResource = spDbManager->FindResourceInProject(m_spProject, sName);
      QReadLocker l(&spResource->m_rwLock);
      if (EResourceType::eDatabase == spResource->m_type._to_integral())
      {
        InitializeModel(m_spProject);
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CDialogueEditorTreeModel::SlotResourceRemoved(qint32 iProjId, const QString& sName)
{
  if (nullptr!= m_spProject)
  {
    m_spProject->m_rwLock.lockForRead();
    qint32 iThisId = m_spProject->m_iId;
    m_spProject->m_rwLock.unlock();

    if (iProjId == iThisId && !sName.isEmpty() && nullptr != m_spDataRootNode)
    {
      bool bWasInTree = false;
      if (m_spDataRootNode->m_sFileId == sName)
      {
        bWasInTree = true;
      }
      std::function<bool(const std::vector<std::shared_ptr<CDialogueNode>>&)> fnCheckChildren =
          [&](const std::vector<std::shared_ptr<CDialogueNode>>& vspChildren) -> bool {
        for (const auto& spChild : qAsConst(vspChildren))
        {
          if (spChild->m_sFileId == sName) { return true; }
          if (fnCheckChildren(spChild->m_vspChildren)) { return true; }
        }
        return false;
      };
      bWasInTree |= fnCheckChildren(m_spDataRootNode->m_vspChildren);

      if (bWasInTree)
      {
        InitializeModel(m_spProject);
      }
    }
  }
}
