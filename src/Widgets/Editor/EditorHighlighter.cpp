#include "EditorHighlighter.h"
#include "EditorCustomBlockUserData.h"

#include <set>
#include <stack>

namespace
{
  constexpr char c_sBracketStart = '{';
  constexpr char c_sBracketEnd = '}';
  constexpr char c_sParenthStart = '(';
  constexpr char c_sParenthEnd = ')';
  constexpr char c_sBraceStart = '[';
  constexpr char c_sBraceEnd = ']';

  ERegionType RegionTypeFromChar(QChar c)
  {
    if (c_sBracketStart == c || c_sBracketEnd == c) { return ERegionType::eBracket; }
    if (c_sParenthStart == c || c_sParenthEnd == c) { return ERegionType::eParenthesis; }
    if (c_sBraceStart == c   || c_sBraceEnd == c)   { return ERegionType::eBrace; }
    return ERegionType::eNone;
  }

  const SBlockDelimiter& ParseCharAndCreateBrackets(
      QChar c, qint32 iPos, ERegionType type,
      std::vector<SBlockDelimiter>& vLastBlockDelimiters,
      std::vector<SBlockDelimiter>& unmatchedBlockDelimiterStarts,
      CCustomBlockUserData* pUserData)
  {
    static const std::set<QChar> startChars = {
        c_sBracketStart, c_sParenthStart, c_sBraceStart
    };
    static const std::set<QChar> endChars = {
        c_sBracketEnd, c_sParenthEnd, c_sBraceEnd
    };

    auto itStart = startChars.find(c);
    auto itEnd = endChars.find(c);
    if (startChars.end() != itStart)
    {
      qint32 iRelativeDepth = 1;
      if (!vLastBlockDelimiters.empty())
      {
        if (KSyntaxHighlighting::FoldingRegion::Type::Begin == vLastBlockDelimiters.back().m_delimiterType)
        {
          iRelativeDepth = vLastBlockDelimiters.back().m_iRelativeDepth + 1;
        }
        else if (KSyntaxHighlighting::FoldingRegion::Type::End == vLastBlockDelimiters.back().m_delimiterType)
        {
          iRelativeDepth =  vLastBlockDelimiters.back().m_iRelativeDepth;
        }
      }
      const SBlockDelimiter& delim =
          pUserData->AddRegionDelimiterElement(iRelativeDepth, type,
                                               KSyntaxHighlighting::FoldingRegion::Type::Begin,
                                               iPos);
      vLastBlockDelimiters.push_back(delim);
      unmatchedBlockDelimiterStarts.push_back(delim);
      return delim;
    }
    else if (endChars.end() != itEnd)
    {
      qint32 iRelativeDepth = 0;
      for (qint32 i = static_cast<qint32>(unmatchedBlockDelimiterStarts.size())-1; 0 <= i; --i)
      {
        iRelativeDepth = vLastBlockDelimiters[size_t(i)].m_iRelativeDepth;
        unmatchedBlockDelimiterStarts.erase(unmatchedBlockDelimiterStarts.begin() + i);
        break;
      }
      const SBlockDelimiter& delim =
        pUserData->AddRegionDelimiterElement(iRelativeDepth, type,
                                             KSyntaxHighlighting::FoldingRegion::Type::End,
                                             iPos);
      vLastBlockDelimiters.push_back(delim);
      return delim;
    }

    static SBlockDelimiter def;
    return def;
  }
}

//----------------------------------------------------------------------------------------
//
CEditorHighlighter::CEditorHighlighter(QTextDocument* pParent) :
  KSyntaxHighlighting::SyntaxHighlighter(pParent),
  m_vBracketColors({QColor(237,41,57),
                    QColor(0, 150, 255),
                    QColor(255, 191, 0),
                    QColor(191, 64, 191)})
{
}

CEditorHighlighter::~CEditorHighlighter()
{}

//----------------------------------------------------------------------------------------
//
void CEditorHighlighter::SetBracketColors(std::vector<QColor> vColors)
{
  m_vBracketColors = vColors;
}

//----------------------------------------------------------------------------------------
//
void CEditorHighlighter::SetSearchColors(const QColor& background,
                                         const QColor& foreground)
{
  m_searchFormat.setBackground(background);
  m_searchFormat.setForeground(foreground);
}

//----------------------------------------------------------------------------------------
//
void CEditorHighlighter::SetActiveWordExpression(const QString& sWord)
{
  if (m_sWord != sWord)
  {
    m_sWord = sWord;

    QRegularExpression::PatternOptions options;
    if (m_bCaseInsensitiveSearch)
    {
      options |= QRegularExpression::CaseInsensitiveOption;
    }

    if (!sWord.isEmpty())
    {
      m_activeWord = QRegularExpression("(^|\\W)(" + sWord + ")(\\W|$)", options);
    }
    else
    {
      m_activeWord = QRegularExpression();
    }
    rehighlight(); // Restart the backlight
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorHighlighter::SetCaseInsensitiveSearchEnabled(bool bEnabled)
{
  if (m_bCaseInsensitiveSearch != bEnabled)
  {
    m_bCaseInsensitiveSearch = bEnabled;
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorHighlighter::SetSearchExpression(const QString& sExpresion)
{
  if (m_sSearch != sExpresion)
  {
    m_sSearch = sExpresion;
    QRegularExpression::PatternOptions options;
    if (m_bCaseInsensitiveSearch)
    {
      options |= QRegularExpression::CaseInsensitiveOption;
    }
    m_searchExpression = QRegularExpression(sExpresion, options);
    rehighlight(); // Restart the backlight
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorHighlighter::SetSyntaxHighlightingEnabled(bool bEnabled)
{
  if (m_bSyntaxHighlightingEnabled != bEnabled)
  {
    m_bSyntaxHighlightingEnabled = bEnabled;
    rehighlight();
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorHighlighter::highlightBlock(const QString& sText)
{
  // default highlighting
  if (m_bSyntaxHighlightingEnabled)
  {
    KSyntaxHighlighting::SyntaxHighlighter::highlightBlock(sText);
  }

  CCustomBlockUserData* pUserData =
      dynamic_cast<CCustomBlockUserData*>(currentBlockUserData());
  if (nullptr == pUserData)
  {
    // first time we highlight this
    pUserData = new CCustomBlockUserData(currentBlockUserData());
    setCurrentBlockUserData(pUserData);
  }
  pUserData->m_vMatchedWordData.clear();

  if (m_bSyntaxHighlightingEnabled)
  {
    pUserData->ClearRegionDelimiters();
    std::vector<QTextBlock> blocks;
    QTextBlock block = currentBlock();

    qint32 iTotalDepthOfPreviousBlocks = 0;
    while (block.previous().isValid())
    {
      block = block.previous();
      blocks.push_back(block);
    }
    for (const QTextBlock& b : blocks)
    {
      CCustomBlockUserData* pUserData =
          dynamic_cast<CCustomBlockUserData*>(b.userData());
      if (nullptr != pUserData)
      {
        iTotalDepthOfPreviousBlocks += pUserData->EndingRelatvieDepth();
      }
    }

    std::vector<SBlockDelimiter> lastBlockDelimiters;
    std::vector<SBlockDelimiter> unmatchedBlockDelimiterStarts;
    for (qint32 i = 0; sText.size() > i; ++i)
    {
      QChar c = sText[i];
      ERegionType type = RegionTypeFromChar(c);
      const SBlockDelimiter& delim =
          ParseCharAndCreateBrackets(c, i, type, lastBlockDelimiters,
                                     unmatchedBlockDelimiterStarts, pUserData);
      if (ERegionType::eNone != type)
      {
        FormatBasedOnDepth(iTotalDepthOfPreviousBlocks + delim.m_iRelativeDepth, i);
      }
    }
  }

  if (m_activeWord.isValid() && !m_sWord.isEmpty())
  {
    // search text highlighting
    // Using a regular expression, we find all matches.
    QRegularExpressionMatchIterator matchIterator = m_activeWord.globalMatch(sText);
    while (matchIterator.hasNext())
    {
      // Highlight all matches
      QRegularExpressionMatch match = matchIterator.next();
      //const QString sMatch = match.captured(2);

      pUserData->m_vMatchedWordData.push_back(
          {match.capturedStart(2), match.capturedLength(2)});
    }
  }
  if (m_searchExpression.isValid() && !m_sSearch.isEmpty())
  {
    // search text highlighting
    // Using a regular expression, we find all matches.
    QRegularExpressionMatchIterator matchIterator = m_searchExpression.globalMatch(sText);
    while (matchIterator.hasNext())
    {
      // Highlight all matches
      QRegularExpressionMatch match = matchIterator.next();
      setFormat(match.capturedStart(), match.capturedLength(), m_searchFormat);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorHighlighter::applyFormat(int offset, int length, const KSyntaxHighlighting::Format &format)
{
  if (m_bSyntaxHighlightingEnabled)
  {
    KSyntaxHighlighting::SyntaxHighlighter::applyFormat(offset, length, format);
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorHighlighter::FormatBasedOnDepth(qint32 iDepth, qint32 iAt)
{
  qint32 iNumColors = static_cast<qint32>(m_vBracketColors.size());
  setFormat(iAt, 1, m_vBracketColors[std::abs(iDepth%iNumColors)]);
}
