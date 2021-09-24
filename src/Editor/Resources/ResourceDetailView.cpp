#include "ResourceDetailView.h"

CResourceDetailView::CResourceDetailView(QWidget* pParent) :
  QListView(pParent),
  m_bReadOnly(false)
{
  setWrapping(true);
  setBatchSize(10);
  setLayoutMode(QListView::Batched);
  setUniformItemSizes(true);
  setViewMode(QListView::IconMode);
}

//----------------------------------------------------------------------------------------
//
void CResourceDetailView::Initialize(tspProject spProject)
{

}

//----------------------------------------------------------------------------------------
//
void CResourceDetailView::DeInitilaze()
{

}

//----------------------------------------------------------------------------------------
//
void CResourceDetailView::Expand(const QModelIndex& index)
{

}

//----------------------------------------------------------------------------------------
//
void CResourceDetailView::Collapse(const QModelIndex& index)
{

}

//----------------------------------------------------------------------------------------
//
void CResourceDetailView::SetReadOnly(bool bReadOnly)
{
  if (m_bReadOnly != bReadOnly)
  {
    m_bReadOnly = bReadOnly;
  }
}

//----------------------------------------------------------------------------------------
//
bool CResourceDetailView::ReadOnly()
{
  return m_bReadOnly;
}
