#include "DialogEditorTreeItem.h"

#include <QApplication>

#include <optional>

using namespace dialogue_item;

CDialogueEditorTreeItem::CDialogueEditorTreeItem(std::shared_ptr<CDialogueNode> spNode,
                                             CDialogueEditorTreeItem* pParentItem) :
    m_pParentItem(pParentItem),
    m_spNode(spNode)
{}

CDialogueEditorTreeItem::~CDialogueEditorTreeItem()
{
  qDeleteAll(m_vpChildItems);
}

//----------------------------------------------------------------------------------------
//
void CDialogueEditorTreeItem::AppendChild(CDialogueEditorTreeItem* pItem)
{
  m_vpChildItems.append(pItem);
}

//----------------------------------------------------------------------------------------
//
CDialogueEditorTreeItem* CDialogueEditorTreeItem::Child(qint32 iRow)
{
  if (iRow < 0 || iRow >= m_vpChildItems.size())
  {
    return nullptr;
  }
  return m_vpChildItems.at(iRow);
}

//----------------------------------------------------------------------------------------
//
qint32 CDialogueEditorTreeItem::ChildIndex(CDialogueEditorTreeItem* pCompare)
{
  for (qint32 i = 0; ChildCount() > i; ++i)
  {
    if (nullptr != pCompare && m_spNode == pCompare->m_spNode)
    {
      return i;
    }
  }
  return -1;
}

//----------------------------------------------------------------------------------------
//
qint32 CDialogueEditorTreeItem::ChildCount() const
{
  return m_vpChildItems.count();
}

//----------------------------------------------------------------------------------------
//
qint32 CDialogueEditorTreeItem::ColumnCount() const
{
  return c_iNumColumns;
}

//----------------------------------------------------------------------------------------
//
QVariant CDialogueEditorTreeItem::Data(qint32 iColumn) const
{
  if (iColumn < 0 || iColumn >= c_iNumOptItems || nullptr == m_spNode)
  {
    return QVariant();
  }
  else
  {
    switch (m_spNode->m_type)
    {
      case EDialogueTreeNodeType::eRoot:    // fallthrough
      case EDialogueTreeNodeType::eCategory:// fallthrough
      {
        switch(iColumn)
        {
          case c_iColumnId:
            return m_spNode->m_sName;
          case c_iColumnString: [[fallthrough]];
          case c_iColumnDisplayString:
            return QVariant();
          case c_iColumnType:
            return m_spNode->m_type._to_integral();
          case c_iColumnWarning:
            return m_sWarning;
          case c_iColumnResource: [[fallthrough]];
          default:
            return QVariant();
        }
      } break;
      case EDialogueTreeNodeType::eDialogue:
      {
        auto spNode = std::static_pointer_cast<CDialogueNodeDialogue>(m_spNode);
        switch(iColumn)
        {
          case c_iColumnId:
            return spNode->m_sName;
          case c_iColumnString:
            if (m_vpChildItems.empty())
            {
              return QString();
            }
            else
            {
              return m_vpChildItems[0]->Data(iColumn);
            }
          case c_iColumnDisplayString:
            if (m_vpChildItems.empty())
            {
              return QT_TR_NOOP("&lt;Empty&gt;");
            }
            else
            {
              return m_vpChildItems[0]->Data(iColumn);
            }
          case c_iColumnWaitMS:
          {
            qint64 iWaitMS = -2;
            for (auto pChild : qAsConst(m_vpChildItems))
            {
              qint64 iVal = pChild->Data(iColumn).toLongLong();
              if (-1 != iVal)
              {
                if (-2 == iWaitMS)
                {
                  iWaitMS = iVal;
                }
                else if (iVal != iWaitMS)
                {
                  iWaitMS = -1;
                }
              }
              else
              {
                iWaitMS = -1;
              }
            }
            return -2 != iWaitMS ? iWaitMS : -1;
          }
          case c_iColumnSkippable:
          {
            std::optional<Qt::CheckState> state = std::nullopt;
            for (auto pChild : qAsConst(m_vpChildItems))
            {
              Qt::CheckState iVal = pChild->Data(iColumn).value<Qt::CheckState>();
              if (!state.has_value())
              {
                state = iVal;
              }
              else if (iVal != state)
              {
                state = Qt::CheckState::PartiallyChecked;
                break;
              }
            }
            return state.has_value() ? state.value() : QVariant();
          }
          case c_iColumnMedia:
            return spNode->m_bHasCondition ? QVariant() :
                     (m_vpChildItems.empty() ? QVariant() : m_vpChildItems[0]->Data(iColumn));
          case c_iColumnCondition:
            return QVariant();
          case c_iColumnType:
            return spNode->m_type._to_integral();
          case c_iColumnResource:
            return spNode->m_sFileId;
          case c_iColumnWarning:
            return m_sWarning;
          default:
            return QVariant();
        }
      } break;
      case EDialogueTreeNodeType::eDialogueFragment:
      {
        auto spNode = std::static_pointer_cast<CDialogueData>(m_spNode);
        switch(iColumn)
        {
          case c_iColumnId:
            return spNode->m_sName;
          case c_iColumnString:
            return spNode->m_sString;
          case c_iColumnDisplayString:
          {
            if (spNode->m_sString.isEmpty()) { return QT_TR_NOOP("&lt;Empty&gt;"); }
            const QString sText =
                spNode->m_sString.replace("<p>", " ").replace("</p>", " ")
                                 .replace("<br>", " ").replace("</br>", " ")
                                 .replace(QRegExp("\\s"), " ");
            return sText;
          }
          case c_iColumnWaitMS:
            return spNode->m_iWaitTimeMs;
          case c_iColumnSkippable:
            return spNode->m_bSkipable ? Qt::Checked : Qt::Unchecked;
          case c_iColumnMedia:
            return spNode->m_sSoundResource;
          case c_iColumnCondition:
            return spNode->m_sCondition;
          case c_iColumnType:
            return m_spNode->m_type._to_integral();
          case c_iColumnResource:
            return spNode->m_sFileId;
          case c_iColumnWarning:
            return m_sWarning;
          default:
            return QVariant();
        }
      } break;
      default: return QVariant();
    }
  }
}

//----------------------------------------------------------------------------------------
//
Qt::ItemFlags CDialogueEditorTreeItem::Flags(qint32 iColumn) const
{
  Qt::ItemFlags flags;
  if (iColumn < 0 || iColumn >= c_iNumOptItems || nullptr == m_spNode)
  {
    return flags;
  }
  flags |= Qt::ItemIsSelectable | Qt::ItemIsEnabled;

  switch (m_spNode->m_type)
  {
    case EDialogueTreeNodeType::eRoot: [[fallthrough]];
    case EDialogueTreeNodeType::eCategory:
    {
      switch(iColumn)
      {
        case c_iColumnId:
          flags |= Qt::ItemIsEditable;
          break;
        default: break;
      }
    } break;
    case EDialogueTreeNodeType::eDialogue:
    {
      auto spNode = std::static_pointer_cast<CDialogueNodeDialogue>(m_spNode);
      switch(iColumn)
      {
        case c_iColumnId:
          flags |= Qt::ItemIsEditable;
          break;
        case c_iColumnString: [[fallthrough]];
        case c_iColumnMedia:
          if (nullptr != spNode && !spNode->m_bHasCondition)
          {
            flags |= Qt::ItemIsEditable;
          }
          break;
        case c_iColumnWaitMS: [[fallthrough]];
        case c_iColumnResource:
          flags |= Qt::ItemIsEditable;
          break;
        case c_iColumnSkippable:
          if (nullptr != spNode && !spNode->m_bHasCondition)
          {
            flags |= Qt::ItemIsUserCheckable;
          }
          else
          {
            flags |= Qt::ItemIsAutoTristate | Qt::ItemIsUserCheckable;
          }
          break;
        default: break;
      }
    } break;
    case EDialogueTreeNodeType::eDialogueFragment:
    {
      switch(iColumn)
      {
        case c_iColumnSkippable:
          flags |= Qt::ItemIsUserCheckable;
          break;
        case c_iColumnId: [[fallthrough]];
        case c_iColumnWaitMS: [[fallthrough]];
        case c_iColumnMedia: [[fallthrough]];
        case c_iColumnCondition: [[fallthrough]];
        case c_iColumnString:
          flags |= Qt::ItemIsEditable;
          break;
        default: break;
      }
    } break;
  }

  return flags;
}

//----------------------------------------------------------------------------------------
//
QVariant CDialogueEditorTreeItem::HeaderData(qint32 iColumn) const
{
  if (iColumn < 0 || iColumn >= c_iNumColumns)
  {
    return QVariant();
  }
  else
  {
    switch(iColumn)
    {
      case c_iColumnId:
        return QObject::tr("Id");
      case c_iColumnString:
      return QObject::tr("Text");
      case c_iColumnWaitMS:
        return QObject::tr("Wait");
      case c_iColumnSkippable:
        return QObject::tr("Skippable");
      case c_iColumnMedia:
        return QObject::tr("Resource");
      default: return QVariant();
    }
  }
}

//----------------------------------------------------------------------------------------
//
bool CDialogueEditorTreeItem::InsertChildren(qint32 iPosition, qint32 iCount, qint32 iColumns)
{
  Q_UNUSED(iColumns)
  if (iPosition < 0 || iPosition > m_vpChildItems.size())
  {
    return false;
  }

  for (qint32 iRow = 0; iRow < iCount; ++iRow)
  {
    CDialogueEditorTreeItem* pItem =
        new CDialogueEditorTreeItem(nullptr, this);
    m_vpChildItems.insert(iPosition, pItem);
  }

  return true;
}

//----------------------------------------------------------------------------------------
//
bool CDialogueEditorTreeItem::InsertColumns(qint32 iPosition, qint32 iColumns)
{
  Q_UNUSED(iPosition)
  Q_UNUSED(iColumns)
  return true;
}

//----------------------------------------------------------------------------------------
//
bool CDialogueEditorTreeItem::RemoveChildren(qint32 iPosition, qint32 iCount)
{
  if (iPosition < 0 || iPosition + iCount > m_vpChildItems.size())
  {
    return false;
  }

  for (qint32 iRow = 0; iRow < iCount; ++iRow)
  {
    delete m_vpChildItems.takeAt(iPosition);
  }

  return true;
}

//----------------------------------------------------------------------------------------
//
bool CDialogueEditorTreeItem::RemoveColumns(qint32 iPosition, qint32 iColumns)
{
  Q_UNUSED(iPosition)
  Q_UNUSED(iColumns)
  return true;
}

//----------------------------------------------------------------------------------------
//
CDialogueEditorTreeItem* CDialogueEditorTreeItem::Parent()
{
  return m_pParentItem;
}

//----------------------------------------------------------------------------------------
//
int CDialogueEditorTreeItem::Row() const
{
  if (m_pParentItem)
  {
    return m_pParentItem->m_vpChildItems.indexOf(const_cast<CDialogueEditorTreeItem*>(this));
  }
  return 0;
}

//----------------------------------------------------------------------------------------
//
bool CDialogueEditorTreeItem::SetData(qint32 iColumn, const QVariant &value)
{
  if (iColumn < 0 || iColumn >= c_iNumColumns)
  {
    return false;
  }

  switch (m_spNode->m_type)
  {
    case EDialogueTreeNodeType::eRoot:    // fallthrough
    case EDialogueTreeNodeType::eCategory:// fallthrough
    {
      if (iColumn == c_iColumnId)
      {
        m_spNode->m_sName = dialogue_tree::EnsureUniqueName(value.toString(), m_spNode->m_wpParent.lock(), m_spNode);
      }
      else
      {
        return false;
      }
    } break;
    case EDialogueTreeNodeType::eDialogue:
    {
      auto spNode = std::static_pointer_cast<CDialogueNodeDialogue>(m_spNode);
      switch(iColumn)
      {
        case c_iColumnId:
        {
          spNode->m_sName = dialogue_tree::EnsureUniqueName(value.toString(), spNode->m_wpParent.lock(), spNode);
        } break;
        case c_iColumnString: [[fallthrough]];
        case c_iColumnMedia:
        {
          if (nullptr != spNode && !spNode->m_bHasCondition && !m_vpChildItems.empty())
          {
            m_vpChildItems[0]->SetData(iColumn, value);
          }
        } break;
        case c_iColumnWaitMS: [[fallthrough]];
        case c_iColumnSkippable:
        {
          for (auto pChild : qAsConst(m_vpChildItems))
          {
            pChild->SetData(iColumn, value);
          }
        } break;
        case c_iColumnResource:
          {
            const QString sNewResource = value.toString();
            spNode->m_sFileId = sNewResource;
          } break;
        default: return false;
      }
    } break;
    case EDialogueTreeNodeType::eDialogueFragment:
    {
      auto spNode = std::static_pointer_cast<CDialogueData>(m_spNode);
      switch(iColumn)
      {
        case c_iColumnId:
          spNode->m_sName = dialogue_tree::EnsureUniqueName(value.toString(), spNode->m_wpParent.lock(), spNode);
          break;
        case c_iColumnWaitMS:
          spNode->m_iWaitTimeMs = value.toLongLong();
          break;
        case c_iColumnSkippable:
          spNode->m_bSkipable = value.value<Qt::CheckState>() == Qt::Checked;
          break;
        case c_iColumnMedia:
          spNode->m_sSoundResource = value.toString();
          break;
        case c_iColumnCondition:
          spNode->m_sCondition = value.toString();
          break;
        case c_iColumnString:
          spNode->m_sString = value.toString();
          break;
        case c_iColumnResource:
          spNode->m_sFileId = value.toString();
          break;
        default: return false;
      }
    } break;
  }
  return true;
}

