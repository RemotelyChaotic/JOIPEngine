#include "CommandChangeFilter.h"
#include "ResourceTreeItemSortFilterProxyModel.h"
#include "Editor/EditorCommandIds.h"
#include "Editor/EditorWidgetTypes.h"
#include "Widgets/SearchWidget.h"

CCommandChangeFilter::CCommandChangeFilter(QPointer<CResourceTreeItemSortFilterProxyModel> pProxyModel,
                                           QPointer<CSearchWidget> pSearchWidget,
                                           QUndoCommand* pParent) :
  QUndoCommand("Resource tree filter changed", pParent),
  m_pProxyModel(pProxyModel),
  m_pSearchWidget(pSearchWidget),
  m_sOldFilter(m_pSearchWidget->property(editor::c_sPropertyOldValue).toString()),
  m_sNewFilter(m_pSearchWidget->Filter())
{
}
CCommandChangeFilter::~CCommandChangeFilter()
{

}

//----------------------------------------------------------------------------------------
//
void CCommandChangeFilter::undo()
{
  m_pSearchWidget->blockSignals(true);
  m_pSearchWidget->setProperty(editor::c_sPropertyOldValue, m_sOldFilter);
  m_pSearchWidget->SetFilter(m_sOldFilter);
  m_pSearchWidget->blockSignals(false);

  if (m_sOldFilter.isNull() || m_sOldFilter.isEmpty())
  {
    m_pProxyModel->setFilterRegExp(QRegExp(".*", Qt::CaseInsensitive, QRegExp::RegExp));
  }
  else
  {
    m_pProxyModel->setFilterRegExp(QRegExp(m_sOldFilter, Qt::CaseInsensitive, QRegExp::RegExp));
  }
}

//----------------------------------------------------------------------------------------
//
void CCommandChangeFilter::redo()
{
  m_pSearchWidget->blockSignals(true);
  m_pSearchWidget->setProperty(editor::c_sPropertyOldValue, m_sNewFilter);
  m_pSearchWidget->SetFilter(m_sNewFilter);
  m_pSearchWidget->blockSignals(false);

  if (m_sNewFilter.isNull() || m_sNewFilter.isEmpty())
  {
    m_pProxyModel->setFilterRegExp(QRegExp(".*", Qt::CaseInsensitive, QRegExp::RegExp));
  }
  else
  {
    m_pProxyModel->setFilterRegExp(QRegExp(m_sNewFilter, Qt::CaseInsensitive, QRegExp::RegExp));
  }
}

//----------------------------------------------------------------------------------------
//
int CCommandChangeFilter::id() const
{
  return EEditorCommandId::eChangeFilter;
}

//----------------------------------------------------------------------------------------
//
bool CCommandChangeFilter::mergeWith(const QUndoCommand* pOther)
{
  if (nullptr == pOther || id() != pOther->id())
  {
    return false;
  }

  const CCommandChangeFilter* pOtherCasted = dynamic_cast<const CCommandChangeFilter*>(pOther);
  if (nullptr == pOtherCasted) { return false; }
  m_sNewFilter = pOtherCasted->m_sNewFilter;
  return true;
}
