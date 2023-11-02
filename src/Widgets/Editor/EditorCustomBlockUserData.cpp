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

//----------------------------------------------------------------------------------------
//
const SBlockDelimiter& CCustomBlockUserData::AddRegionDelimiterElement(
    qint32 iRelativeDepth,
    ERegionType regionType,
    KSyntaxHighlighting::FoldingRegion::Type delimiterType,
    qint32 iAtPosition)
{
  SBlockDelimiter delim{iRelativeDepth, regionType, delimiterType, iAtPosition};
  m_allRegionDelimiters.push_back(delim);
  return m_allRegionDelimiters.back();
}

void CCustomBlockUserData::ClearRegionDelimiters()
{
  m_allRegionDelimiters.clear();
}

qint32 CCustomBlockUserData::EndingRelatvieDepth()
{
  if (m_allRegionDelimiters.size() > 0)
  {
    auto& lastElem = m_allRegionDelimiters.back();
    if (KSyntaxHighlighting::FoldingRegion::Type::Begin == lastElem.m_delimiterType)
    {
      return lastElem.m_iRelativeDepth;
    }
    else if (KSyntaxHighlighting::FoldingRegion::Type::End == lastElem.m_delimiterType)
    {
      return lastElem.m_iRelativeDepth -1;
    }
  }
  return 0;
}
