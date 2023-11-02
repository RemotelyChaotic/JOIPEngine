#ifndef CCUSTOMBLOCKUSERDATA_H
#define CCUSTOMBLOCKUSERDATA_H

#include <syntaxhighlighter.h>
#include <vector>

//----------------------------------------------------------------------------------------
//
struct SMatchedWordData
{
  qint32 m_iStart;
  qint32 m_iLength;
};

//----------------------------------------------------------------------------------------
//
enum class ERegionType : qint32
{
  eNone = 0,

  eBrace,
  eParenthesis,
  eBracket,

  eLast
};

//----------------------------------------------------------------------------------------
//
struct SBlockDelimiter
{
  qint32                             m_iRelativeDepth = 0;
  ERegionType                        m_regionType = ERegionType::eNone;
  KSyntaxHighlighting::FoldingRegion::Type m_delimiterType = KSyntaxHighlighting::FoldingRegion::Type::None;
  qint32                             m_iPosition = 0;
};

//----------------------------------------------------------------------------------------
//
class CCustomBlockUserData : public KSyntaxHighlighting::TextBlockUserData
{
public:
  CCustomBlockUserData(QTextBlockUserData* pOldData = nullptr);
  ~CCustomBlockUserData() override;

  void SetFoldedContent(const QString& sContent);
  const QString& FoldedContent() const;

  const SBlockDelimiter& AddRegionDelimiterElement(
      qint32 iRelativeDepth,
      ERegionType regionType,
      KSyntaxHighlighting::FoldingRegion::Type delimiterType,
      qint32 iAtPosition);
  void ClearRegionDelimiters();
  qint32 EndingRelatvieDepth();

  std::vector<SMatchedWordData> m_vMatchedWordData;

private:
  QVector<SBlockDelimiter> m_allRegionDelimiters;
  QString                  m_sFoldedContent;
};

#endif // CCUSTOMBLOCKUSERDATA_H
