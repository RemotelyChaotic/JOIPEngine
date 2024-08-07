#include "CommandChangeCurrentResource.h"
#include "Application.h"
#include "ResourceTreeItemModel.h"
#include "Editor/EditorCommandIds.h"
#include <QItemSelectionModel>
#include <QSortFilterProxyModel>

CCommandChangeCurrentResource::CCommandChangeCurrentResource(
    std::shared_ptr<SProject> spCurrentProject,
    std::vector<QPointer<QItemSelectionModel>> vpSelectionModel,
    QPointer<QSortFilterProxyModel> pProxyModel,
    const QString& sOld,
    const QString& sNew,
    std::function<void(const QString&)> fnOnSelectionChanged,
    QUndoCommand* pParent) :
  QUndoCommand("Selected resource: " + sNew, pParent),
  m_spCurrentProject(spCurrentProject),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>()),
  m_vpSelectionModel(vpSelectionModel),
  m_pProxyModel(pProxyModel),
  m_sOldResource(sOld),
  m_sNewResource(sNew),
  m_fnOnSelectionChanged(fnOnSelectionChanged)
{
}

CCommandChangeCurrentResource::~CCommandChangeCurrentResource()
{}

//----------------------------------------------------------------------------------------
//
void CCommandChangeCurrentResource::undo()
{
  RunDoOrUndo(m_sOldResource);
}

//----------------------------------------------------------------------------------------
//
void CCommandChangeCurrentResource::redo()
{
  RunDoOrUndo(m_sNewResource);
}

//----------------------------------------------------------------------------------------
//
int CCommandChangeCurrentResource::id() const
{
  return EEditorCommandId::eChangeCurrentResource;
}

//----------------------------------------------------------------------------------------
//
bool CCommandChangeCurrentResource::mergeWith(const QUndoCommand* pOther)
{
  if (nullptr == pOther || id() != pOther->id())
  {
    return false;
  }

  const CCommandChangeCurrentResource* pOtherCasted = dynamic_cast<const CCommandChangeCurrentResource*>(pOther);
  if (nullptr == pOtherCasted) { return false; }

  m_sNewResource = pOtherCasted->m_sNewResource;

  setText("Selected resource: " + m_sNewResource);
  return true;
}

//----------------------------------------------------------------------------------------
//
void CCommandChangeCurrentResource::RunDoOrUndo(const QString& sResource)
{
  if (!sResource.isEmpty())
  {
    auto spDbManager = m_wpDbManager.lock();
    if (nullptr != spDbManager)
    {
      auto spResource = spDbManager->FindResourceInProject(m_spCurrentProject, sResource);
      if (nullptr != spResource)
      {
        CResourceTreeItemModel* pModel =
          dynamic_cast<CResourceTreeItemModel*>(m_pProxyModel->sourceModel());

        if (nullptr != pModel)
        {
          for (QPointer<QItemSelectionModel> pSelectionModel : m_vpSelectionModel)
          {
            if (nullptr != pSelectionModel)
            {
              pSelectionModel->blockSignals(true);
              pSelectionModel->select(m_pProxyModel->mapFromSource(pModel->IndexForResource(spResource)),
                                      QItemSelectionModel::Clear | QItemSelectionModel::SelectCurrent |
                                      QItemSelectionModel::Rows);
              pSelectionModel->blockSignals(false);
            }
          }

          if (nullptr != m_fnOnSelectionChanged)
          {
            m_fnOnSelectionChanged(sResource);
          }
        }
      }
    }
  }
  else
  {
    CResourceTreeItemModel* pModel =
      dynamic_cast<CResourceTreeItemModel*>(m_pProxyModel->sourceModel());
    if (nullptr != pModel)
    {
      for (QPointer<QItemSelectionModel> pSelectionModel : m_vpSelectionModel)
      {
        if (nullptr != pSelectionModel)
        {
          pSelectionModel->blockSignals(true);
          pSelectionModel->clearSelection();
          pSelectionModel->blockSignals(false);
        }
      }
      if (nullptr != m_fnOnSelectionChanged)
      {
        m_fnOnSelectionChanged(sResource);
      }
    }
  }
}
