#include "EditorCustomBlockUserData.h"

//----------------------------------------------------------------------------------------
//
CCustomBlockUserData::CCustomBlockUserData(QTextBlockUserData* pOldData) :
    KSyntaxHighlighting::TextBlockUserData()
{
  if (auto pOldDataKS = dynamic_cast<KSyntaxHighlighting::TextBlockUserData*>(pOldData))
  {
    state = pOldDataKS->state;
    foldingRegions = pOldDataKS->foldingRegions;
  }
  if (auto pOldDataCustom = dynamic_cast<CCustomBlockUserData*>(pOldData))
  {
    m_sFoldedContent = pOldDataCustom->m_sFoldedContent;
    m_vMatchedWordData = pOldDataCustom->m_vMatchedWordData;
  }
}
CCustomBlockUserData::~CCustomBlockUserData() = default;

//----------------------------------------------------------------------------------------
//
void CCustomBlockUserData::SetFoldedContent(const QString& sContent)
{
  m_sFoldedContent = sContent;
}

const QString& CCustomBlockUserData::FoldedContent() const
{
  return m_sFoldedContent;
}
