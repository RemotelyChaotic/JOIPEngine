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
class CCustomBlockUserData : public KSyntaxHighlighting::TextBlockUserData
{
public:
  CCustomBlockUserData(QTextBlockUserData* pOldData = nullptr);
  ~CCustomBlockUserData() override;

  void SetFoldedContent(const QString& sContent);
  const QString& FoldedContent() const;

  std::vector<SMatchedWordData> m_vMatchedWordData;

private:
  QString m_sFoldedContent;
};

#endif // CCUSTOMBLOCKUSERDATA_H
