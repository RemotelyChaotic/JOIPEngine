#include "CommandChangeResourceData.h"
#include "Application.h"
#include "ResourceTreeItemModel.h"
#include "ResourceTreeItem.h"
#include "Editor/EditorCommandIds.h"
#include "Systems/DatabaseManager.h"

CCommandChangeResourceData::CCommandChangeResourceData(QPointer<CResourceTreeItemModel> pModel,
                                                       tspProject spProject,
                                                       const QString& sResource,
                                                       qint32 iColumn,
                                                       const QVariant& data,
                                                       QUndoCommand* pParent) :
  QUndoCommand("Changed resource data -> " + data.toString(), pParent),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>()),
  m_spProject(spProject),
  m_spResurce(nullptr),
  m_pModel(pModel),
  m_sResource(sResource),
  m_iColumn(iColumn),
  m_oldData(),
  m_data(data),
  m_bFirstRun(true)
{

}
CCommandChangeResourceData::~CCommandChangeResourceData()
{

}

//----------------------------------------------------------------------------------------
//
void CCommandChangeResourceData::undo()
{
  RunDoOrUndo(m_oldData);
}

//----------------------------------------------------------------------------------------
//
void CCommandChangeResourceData::redo()
{
  if (nullptr == m_spResurce && m_bFirstRun)
  {
    if (auto spDbManager = m_wpDbManager.lock())
    {
      auto spResource = spDbManager->FindResourceInProject(m_spProject, m_sResource);
      if (nullptr != spResource)
      {
        m_spResurce = std::make_shared<SResource>(*spResource);
      }
    }
  }

  RunDoOrUndo(m_data);

  m_bFirstRun = false;
}

//----------------------------------------------------------------------------------------
//
int CCommandChangeResourceData::id() const
{
  return EEditorCommandId::eChangeResourceData;
}

//----------------------------------------------------------------------------------------
//
bool CCommandChangeResourceData::mergeWith(const QUndoCommand* pOther)
{
  if (nullptr == pOther || id() != pOther->id())
  {
    return false;
  }

  const CCommandChangeResourceData* pOtherCasted = dynamic_cast<const CCommandChangeResourceData*>(pOther);
  if (nullptr == pOtherCasted ||
      m_sResource != pOtherCasted->m_sResource ||
      m_iColumn != pOtherCasted->m_iColumn)
  { return false; }

  m_data = pOtherCasted->m_data;

  if (nullptr != m_pModel)
  {
    QModelIndex index = m_pModel->IndexForResource(m_spResurce);
    CResourceTreeItem* pItem = m_pModel->GetItem(index);
    SetText(pItem);
  }
  return true;
}

//----------------------------------------------------------------------------------------
//
void CCommandChangeResourceData::RunDoOrUndo(const QVariant& data)
{
  if (nullptr != m_pModel)
  {
    QModelIndex index = m_pModel->IndexForResource(m_spResurce);
    CResourceTreeItem* pItem = m_pModel->GetItem(index);
    if (nullptr != pItem)
    {
      if (!m_oldData.isValid() && m_bFirstRun)
      {
        m_oldData = pItem->Data(m_iColumn);
        SetText(pItem);
      }

      bool bResult = pItem->SetData(m_iColumn, data);

      if (bResult)
      {
        emit m_pModel->dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
        emit m_pModel->SignalProjectEdited();
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CCommandChangeResourceData::SetText(CResourceTreeItem* pItem)
{
  if (nullptr != pItem)
  {
    setText(QString("Changed resource %1 -> %2")
            .arg(pItem->HeaderData(m_iColumn).toString()).arg(m_data.toString()));
  }
}
