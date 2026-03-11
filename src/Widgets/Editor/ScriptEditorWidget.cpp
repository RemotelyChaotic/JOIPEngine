#include "ScriptEditorWidget.h"
#include "ScriptEditorWidgetPrivate.h"

#include "EditorCustomBlockUserData.h"
#include "EditorSearchBar.h"
#include "EditorHighlighter.h"
#include "ScriptEditorAddonWidgets.h"
#include "ScriptEditorCodeToolTip.h"
#include "ScriptEditorCompleter.h"
#include "ScriptEditorKeyHandler.h"
#include "HighlightedSearchableTextEdit.h"
#include "TextEditZoomEnabler.h"

#include <syntaxhighlighter.h>
#include <definition.h>
#include <theme.h>

#include <QtWidgets>
#include <QFontMetrics>
#include <QHBoxLayout>

namespace
{
  [[maybe_unused]] const qint32 c_iTabStop = 2;  // 2 characters
}

//----------------------------------------------------------------------------------------
//
CScriptEditorWidgetPrivate::CScriptEditorWidgetPrivate(CScriptEditorWidget* pParent) :
  QPlainTextEdit(pParent),
  m_pParent(pParent)
{
}
CScriptEditorWidgetPrivate::~CScriptEditorWidgetPrivate() = default;

//----------------------------------------------------------------------------------------
//
CScriptEditorWidget::CScriptEditorWidget(QWidget* pParent) :
  QWidget(pParent),
  m_spRepository(std::make_unique<KSyntaxHighlighting::Repository>()),
  m_pTextEditor(new CScriptEditorWidgetPrivate(this)),
  m_pCompleter(new CScriptEditorCompleter(m_pTextEditor.data())),
  m_pHighlightedSearchableEdit(nullptr),
  m_foldedIcon(""),
  m_unfoldedIcon(""),
  m_bracketColor0(237,41,57),
  m_bracketColor1(0, 150, 255),
  m_bracketColor2(255, 191, 0),
  m_bracketColor3(191, 64, 191),
  m_foldAreaBackgroundColor(24, 24, 24),
  m_lineNumberBackgroundColor(24, 24, 24),
  m_lineNumberTextColor(Qt::white),
  m_highlightLineColor(68, 71, 90),
  m_wordhighlightColor(200, 200, 200),
  m_widgetsBackgroundColor(24, 24, 24),
  m_previouslyClickedKey(Qt::Key(0)),
  m_iTabStopWidth(c_iTabStop),
  m_iFontSize(10),
  m_sFontFamily("")
{
  QLayout* pLayout = new QHBoxLayout(this);
  pLayout->setContentsMargins({0, 0, 0, 0});
  pLayout->addWidget(m_pTextEditor);

  m_pTextEditor->setAttribute(Qt::WA_NoMousePropagation, false);
  m_pTextEditor->installEventFilter(this);
  m_pTextEditor->setLineWrapMode(QPlainTextEdit::LineWrapMode::NoWrap);

  //for (auto def : m_spRepository->definitions()) qDebug() << def.name();

  m_pHighlightedSearchableEdit = new CHighlightedSearchableTextEdit(m_pTextEditor.data());

  auto vThemes = m_spRepository->themes();
  if (!vThemes.empty())
  {
    SlotSettingThemeChanged(vThemes.first().name());
  }

  m_pZoomEnabler = new CTextEditZoomEnabler(m_pTextEditor.data());

  // register all addons
  auto pWidgetArea = new CWidgetArea(this);
  m_vpEditorAddonsMap[EScriptEditorAddonPosition::eLeft] = {
    new CLineNumberArea(this), pWidgetArea, new CFoldBlockArea(this)
  };
  m_fnWidget = std::bind(&CWidgetArea::Widget, pWidgetArea, std::placeholders::_1);

  auto pFooter = new CFooterArea(this, m_pTextEditor->horizontalScrollBar(), pWidgetArea);
  m_vpEditorAddonsMap[EScriptEditorAddonPosition::eBottom] = { pFooter };

  connect(pFooter, &CFooterArea::SignalShowWhiteSpaceChanged, this,
          &CScriptEditorWidget::SlotSettingShowWhitespaceChanged);

  // register key handlers
  IScriptEditorKeyHandler::RegisterHandlers(m_vpEditorKeyHandlerMap, m_pTextEditor, &m_previouslyClickedKey);

  connect(m_pTextEditor, &CScriptEditorWidgetPrivate::blockCountChanged,
          this, &CScriptEditorWidget::UpdateLeftAreaWidth);
  connect(m_pTextEditor, &CScriptEditorWidgetPrivate::updateRequest,
          this, &CScriptEditorWidget::SlotUpdateAllAddons);
  connect(m_pTextEditor, &CScriptEditorWidgetPrivate::cursorPositionChanged,
          this, &CScriptEditorWidget::HighlightCurrentLine);
  connect(m_pTextEditor, &CScriptEditorWidgetPrivate::cursorPositionChanged,
          this, &CScriptEditorWidget::HighlightCurrentWord);

  connect(m_pTextEditor, &CScriptEditorWidgetPrivate::textChanged,
          this, &CScriptEditorWidget::SignalTextChanged);

  UpdateLeftAreaWidth(0);
  UpdateBottomAreaHeight(0);
  HighlightCurrentLine();

  SlotSettingFontChanged("Courier New");
  SetTabStopWidth(c_iTabStop);

  SlotSettingShowWhitespaceChanged(true);
  SlotSettingCaseInsensitiveSearchChanged(true);
}

CScriptEditorWidget::~CScriptEditorWidget()
{
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorWidget::AddCustomThemeSearchPath(const QString& sCustomSearchPath)
{
  m_spRepository->addCustomSearchPath(sCustomSearchPath);

  // Force reload of Theme
  QString sTheme = m_sTheme;
  m_sTheme = QString();
  SlotSettingThemeChanged(sTheme);
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorWidget::Clear()
{
  m_pTextEditor->clear();
}

//----------------------------------------------------------------------------------------
//
QTextDocument* CScriptEditorWidget::Document() const
{
  return m_pTextEditor->document();
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorWidget::InsertText(const QString& sText)
{
  m_pTextEditor->insertPlainText(sText);
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorWidget::SetHighlightDefinition(const QString& sType)
{
  const auto def = m_spRepository->definitionForName(sType);
  m_pHighlightedSearchableEdit->Highlighter()->setDefinition(def);
  m_pHighlightedSearchableEdit->Highlighter()->rehighlight();
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorWidget::SetReadOnly(bool bReadOnly)
{
  m_pTextEditor->setReadOnly(bReadOnly);
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorWidget::SetTabStopWidth(qint32 iNumSpaces)
{
  m_iTabStopWidth = iNumSpaces;
#if QT_VERSION >= QT_VERSION_CHECK(5,10,0)
  QFontMetrics metrics(font());
  m_pTextEditor->setTabStopDistance(m_iTabStopWidth * metrics.boundingRect(' ').width());
#endif
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorWidget::SetText(const QString& sText)
{
  m_pTextEditor->setPlainText(sText);
  UpdateFont();
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorWidget::SetTextWhilePreservingCursor(const QString& sText)
{
  const INT32 iCurrentCursorPosition = m_pTextEditor->textCursor().position();
  m_pTextEditor->setPlainText(sText);

  QTextCursor cursor = m_pTextEditor->textCursor();
  cursor.setPosition(iCurrentCursorPosition);
  m_pTextEditor->setTextCursor(cursor);

  UpdateFont();
}

//----------------------------------------------------------------------------------------
//
QString CScriptEditorWidget::Text() const
{
  return m_pTextEditor->toPlainText();
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorWidget::SetBracketColor0(QColor c)
{
  if (m_bracketColor0 != c)
  {
    m_bracketColor0 = c;
    m_pHighlightedSearchableEdit->Highlighter()->SetBracketColors(
        {m_bracketColor0, m_bracketColor1, m_bracketColor2, m_bracketColor3});
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorWidget::SetBracketColor1(QColor c)
{
  if (m_bracketColor1 != c)
  {
    m_bracketColor1 = c;
    m_pHighlightedSearchableEdit->Highlighter()->SetBracketColors(
        {m_bracketColor0, m_bracketColor1, m_bracketColor2, m_bracketColor3});
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorWidget::SetBracketColor2(QColor c)
{
  if (m_bracketColor2 != c)
  {
    m_bracketColor2 = c;
    m_pHighlightedSearchableEdit->Highlighter()->SetBracketColors(
        {m_bracketColor0, m_bracketColor1, m_bracketColor2, m_bracketColor3});
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorWidget::SetBracketColor3(QColor c)
{
  if (m_bracketColor3 != c)
  {
    m_bracketColor3 = c;
    m_pHighlightedSearchableEdit->Highlighter()->SetBracketColors(
        {m_bracketColor0, m_bracketColor1, m_bracketColor2, m_bracketColor3});
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorWidget::SetCaseInsensitiveSearch(bool bValue)
{
  SlotSettingCaseInsensitiveSearchChanged(bValue);
}

//----------------------------------------------------------------------------------------
//
bool CScriptEditorWidget::IsCaseInsensitiveSearchenabled() const
{
  return m_pHighlightedSearchableEdit->IsCaseInsensitiveFindEnabled();
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorWidget::SetFontSize(qint32 iFontSize)
{
  if (m_iFontSize != iFontSize)
  {
    m_iFontSize = iFontSize;
    bool bOk = QMetaObject::invokeMethod(this, "UpdateFont", Qt::QueuedConnection);
    assert(bOk); Q_UNUSED(bOk)
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorWidget::SetFontFamily(const QString& sFont)
{
  SlotSettingFontChanged(sFont);
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorWidget::SetLineNumberTextColor(const QColor& color)
{
  if (m_lineNumberTextColor != color)
  {
    m_lineNumberTextColor = color;
    for (auto pWidget : m_vpEditorAddonsMap[EScriptEditorAddonPosition::eBottom])
    {
      auto pFooter =
          dynamic_cast<CFooterArea*>(pWidget);
      if (nullptr != pFooter)
      {
        pFooter->StyleChanged();
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorWidget::SetHighlightSearchBackgroundColor(const QColor& color)
{
  if (m_highlightSearchBackgroundColor != color)
  {
    m_highlightSearchBackgroundColor = color;
    Highlighter()->SetSearchColors(m_highlightSearchBackgroundColor,
                                   m_highlightSearchColor);
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorWidget::SetHighlightSearchColor(const QColor& color)
{
  if (m_highlightSearchColor != color)
  {
    m_highlightSearchColor = color;
    Highlighter()->SetSearchColors(m_highlightSearchBackgroundColor,
                                   m_highlightSearchColor);
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorWidget::SetWordHighlightColor(const QColor& color)
{
  if (m_wordhighlightColor != color)
  {
    m_wordhighlightColor = color;
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorWidget::SetShowWhitespace(bool bValue)
{
  SlotSettingShowWhitespaceChanged(bValue);
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorWidget::SetTheme(const QString& sTheme)
{
  SlotSettingThemeChanged(sTheme);
}

//----------------------------------------------------------------------------------------
//
QPointer<CScriptEditorCompleter> CScriptEditorWidget::Completer() const
{ return m_pCompleter; }
QPointer<CEditorHighlighter> CScriptEditorWidget::Highlighter() const
{ return m_pHighlightedSearchableEdit->Highlighter(); }
QPointer<CEditorSearchBar>   CScriptEditorWidget::SearchBar() const
{ return m_pHighlightedSearchableEdit->SearchBar(); }
QPointer<CTextEditZoomEnabler> CScriptEditorWidget::ZoomEnabler() const
{ return m_pZoomEnabler; }

//----------------------------------------------------------------------------------------
//
QMenu* CScriptEditorWidget::CreateContextMenu()
{
  return m_pHighlightedSearchableEdit->CreateContextMenu();
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorWidget::UpdateArea(EScriptEditorAddonPosition pos,
                                     qint32 iNewBlockCount)
{
  switch (pos)
  {
    case EScriptEditorAddonPosition::eLeft:
      UpdateLeftAreaWidth(iNewBlockCount); break;
    case EScriptEditorAddonPosition::eRight:
      UpdateRightAreaWidth(iNewBlockCount); break;
    case EScriptEditorAddonPosition::eTop:
      UpdateTopAreaHeight(iNewBlockCount); break;
    case EScriptEditorAddonPosition::eBottom:
      UpdateBottomAreaHeight(iNewBlockCount); break;
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorWidget::ResetAddons()
{
  for (const auto& [pos, vpAddons] : m_vpEditorAddonsMap)
  {
    Q_UNUSED(pos)
    for (IScriptEditorAddon* pAddon : vpAddons)
    {
      pAddon->Reset();
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorWidget::HighlightCurrentLine()
{
  QList<QTextEdit::ExtraSelection> vExtraSelections;

  if (!m_pTextEditor->isReadOnly())
  {
    QTextEdit::ExtraSelection selection;
    selection.format.setBackground(m_highlightLineColor);
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.cursor = m_pTextEditor->textCursor();
    selection.cursor.clearSelection();
    vExtraSelections.append(selection);
  }

  m_pTextEditor->setExtraSelections(vExtraSelections);
  m_pTextEditor->repaint();
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorWidget::HighlightCurrentWord()
{
  QTextCursor cursor = m_pTextEditor->textCursor();
  if (!cursor.hasSelection())
  {
    cursor.select(QTextCursor::WordUnderCursor);
  }
  QString sText = cursor.selectedText();
  static const QRegExp exp("^\\w+$");
  if (!exp.exactMatch(sText))
  {
    cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, 1);
    cursor.select(QTextCursor::WordUnderCursor);

    sText = cursor.selectedText();
    if (!exp.exactMatch(sText))
    {
      sText = QString();
    }
  }

  Highlighter()->SetActiveWordExpression(sText);
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorWidget::UpdateFont()
{
  qint32 iZoom = m_pZoomEnabler->Zoom();

  QFont font;
  font.setFamily(m_sFontFamily);
  font.setStyleHint(QFont::Monospace);
  font.setFixedPitch(true);
  font.setPointSizeF(m_iFontSize);

  setFont(font);
  m_pTextEditor->document()->setDefaultFont(font);
  m_pTextEditor->viewport()->setFont(font);

  m_pZoomEnabler->UpdateZoom(iZoom);
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorWidget::UpdateLeftAreaWidth(qint32 iNewBlockCount)
{
  Q_UNUSED(iNewBlockCount)
  QMargins margins = m_pTextEditor->viewportMargins();
  qint32 iTotalWidth = 0;
  for (IScriptEditorAddon* pAddon : m_vpEditorAddonsMap[EScriptEditorAddonPosition::eLeft])
  {
    iTotalWidth += pAddon->AreaWidth();
  }
  m_pTextEditor->setViewportMargins(iTotalWidth, margins.top(), margins.right(), margins.bottom());
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorWidget::UpdateRightAreaWidth(qint32 iNewBlockCount)
{
  Q_UNUSED(iNewBlockCount)
  QMargins margins = m_pTextEditor->viewportMargins();
  qint32 iTotalWidth = 0;
  for (IScriptEditorAddon* pAddon : m_vpEditorAddonsMap[EScriptEditorAddonPosition::eRight])
  {
    iTotalWidth += pAddon->AreaWidth();
  }
  m_pTextEditor->setViewportMargins(margins.left(), margins.top(), iTotalWidth, margins.bottom());
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorWidget::UpdateTopAreaHeight(qint32 iNewBlockCount)
{
  Q_UNUSED(iNewBlockCount)
  QMargins margins = m_pTextEditor->viewportMargins();
  qint32 iTotalHeight = 0;
  for (IScriptEditorAddon* pAddon : m_vpEditorAddonsMap[EScriptEditorAddonPosition::eTop])
  {
    iTotalHeight += pAddon->AreaHeight();
  }
  m_pTextEditor->setViewportMargins(margins.left(), iTotalHeight, margins.right(), margins.bottom());
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorWidget::UpdateBottomAreaHeight(qint32 iNewBlockCount)
{
  Q_UNUSED(iNewBlockCount)
  QMargins margins = m_pTextEditor->viewportMargins();
  qint32 iTotalHeight = 0;
  for (IScriptEditorAddon* pAddon : m_vpEditorAddonsMap[EScriptEditorAddonPosition::eBottom])
  {
    iTotalHeight += pAddon->AreaHeight();
  }
  m_pTextEditor->setViewportMargins(margins.left(), margins.top(), margins.right(), iTotalHeight);
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorWidget::SlotExecutionError(QString sException, qint32 iLine, QString sStack)
{
  for (auto [pos, vpAddons] : m_vpEditorAddonsMap)
  {
    Q_UNUSED(pos)
    for (IScriptEditorAddon* pAddon : vpAddons)
    {
      pAddon->Reset();
      if (CWidgetArea* pWidgetArea = dynamic_cast<CWidgetArea*>(pAddon); nullptr != pWidgetArea)
      {
        pWidgetArea->AddError(sException, iLine, sStack);
      }
    }
  }

  QTextCursor cursor(m_pTextEditor->document()->findBlockByLineNumber(iLine));
  m_pTextEditor->moveCursor(QTextCursor::End);
  m_pTextEditor->setTextCursor(cursor);
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorWidget::SlotSettingCaseInsensitiveSearchChanged(bool bCaseInsensitive)
{
  m_pHighlightedSearchableEdit->SetCaseInsensitiveFindEnabled(bCaseInsensitive);
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorWidget::SlotSettingFontChanged(const QString& sFontFamily)
{
  if (m_sFontFamily != sFontFamily)
  {
    m_sFontFamily = sFontFamily;
    bool bOk = QMetaObject::invokeMethod(this, "UpdateFont", Qt::QueuedConnection);
    assert(bOk); Q_UNUSED(bOk)
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorWidget::SlotSettingShowWhitespaceChanged(bool bShowWhiteSpace)
{
  if (m_bShowWhitespaceEnabled != bShowWhiteSpace)
  {
    m_bShowWhitespaceEnabled = bShowWhiteSpace;
    QTextOption option = m_pTextEditor->document()->defaultTextOption();
    if (bShowWhiteSpace)
    {
      option.setFlags(QTextOption::ShowTabsAndSpaces);
    }
    else
    {
      option.setFlags(QTextOption::Flag(0x0));
    }

    for (auto pWidget : m_vpEditorAddonsMap[EScriptEditorAddonPosition::eBottom])
    {
      auto pFooter =
          dynamic_cast<CFooterArea*>(pWidget);
      if (nullptr != pFooter)
      {
        QSignalBlocker b(pFooter);
        pFooter->UpdateWhitespaceText(bShowWhiteSpace);
      }
    }

    m_pTextEditor->document()->setDefaultTextOption(option);
    m_pTextEditor->repaint();
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorWidget::SlotSettingThemeChanged(const QString& sTheme)
{
  if (m_sTheme != sTheme)
  {
    if (sTheme.isEmpty())
    {
      Highlighter()->setTheme(m_spRepository->theme(m_sTheme));
    }
    else
    {
      m_sTheme = sTheme;
      Highlighter()->setTheme(m_spRepository->theme(sTheme));
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorWidget::SlotUpdateAllAddons(const QRect& rect, qint32 iDy)
{
  for (auto [pos, vpAddons] : m_vpEditorAddonsMap)
  {
    Q_UNUSED(pos)
    for (IScriptEditorAddon* pAddon : vpAddons)
    {
      pAddon->Update(rect, iDy);
    }
  }
}

//----------------------------------------------------------------------------------------
//
const CScriptEditorWidget::tAdditionsMap& CScriptEditorWidget::AdditionsMap() const
{
  return m_vpEditorAddonsMap;
}

//----------------------------------------------------------------------------------------
//
QPointer<CScriptEditorWidgetPrivate> CScriptEditorWidget::TextEdit() const
{
  return m_pTextEditor.data();
}

//----------------------------------------------------------------------------------------
//
QPointer<QLabel> CScriptEditorWidget::WidgetAreaWidget(qint32 iLine)
{
  return m_fnWidget(iLine);
}

//----------------------------------------------------------------------------------------
//
bool CScriptEditorWidget::eventFilter(QObject* pTarget, QEvent* pEvent)
{
  if (nullptr != pEvent && this == pTarget)
  {
    if (QEvent::KeyPress == pEvent->type())
    {
      QKeyEvent* pKeyEvent = static_cast<QKeyEvent*>(pEvent);

      bool bRet = false;
      auto itKeyHandler = m_vpEditorKeyHandlerMap.find(Qt::Key(pKeyEvent->key()));
      if (m_vpEditorKeyHandlerMap.end() != itKeyHandler)
      {
        bRet = itKeyHandler->second->KeyEvent(pKeyEvent);
      }

      m_previouslyClickedKey = Qt::Key(pKeyEvent->key());
      return bRet;
    }

    else if (QEvent::ToolTip == pEvent->type())
    {
      QHelpEvent* helpEvent = static_cast<QHelpEvent*>(pEvent);
      QPoint pos = helpEvent->pos();

      QTextCursor cursor = m_pTextEditor->cursorForPosition(pos);
      if (CCustomBlockUserData* pUserData =
          dynamic_cast<CCustomBlockUserData*>(cursor.block().userData());
          nullptr != pUserData)
      {
        const QRectF blockRect = m_pTextEditor->blockBoundingRect(cursor.block());
        qint32 iBlockHeight = static_cast<qint32>(blockRect.height());
        cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        QPoint topLeft = m_pTextEditor->cursorRect(cursor).topLeft();
        QRect blockRectToCheck(topLeft.x(), topLeft.y(),
                              static_cast<qint32>(blockRect.width()) - topLeft.x(),
                              iBlockHeight);

        const qint32 iDotWidth = fontMetrics().boundingRect("...").width();
        blockRectToCheck.setWidth(iDotWidth + 10);
        blockRectToCheck.translate(m_pTextEditor->viewportMargins().left(), m_pTextEditor->viewportMargins().top());

        //qDebug() << pos << cursor.blockNumber() << cursor.block().text() << blockRectToCheck;

        const QString& sFoldedContent = pUserData->FoldedContent();
        if (!sFoldedContent.isEmpty() &&
            blockRectToCheck.contains(pos))
        {
          CScriptEditorCodeToolTip::showToolTip(
              mapToGlobal(QPoint(m_pTextEditor->viewportMargins().left(), blockRectToCheck.topLeft().y())),
              sFoldedContent, Highlighter());
        }
        else
        {
          CScriptEditorCodeToolTip::hideToolTip();
        }
      }
      else
      {
        CScriptEditorCodeToolTip::hideToolTip();
      }
      return true;
    }
  }

  return QWidget::eventFilter(pTarget, pEvent);
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorWidgetPrivate::keyPressEvent(QKeyEvent* pEvt)
{
  // needed otherwise completer doesn't work correctly.
  bool bRet = m_pParent->Completer()->keyPressEvent(pEvt);
  if (bRet)
  {
    return;
  }
  QPlainTextEdit::keyPressEvent(pEvt);
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorWidgetPrivate::paintEvent(QPaintEvent* pEvent)
{
  QPlainTextEdit::paintEvent(pEvent);

  QPainter painter(viewport());
  if (!m_pParent->m_foldSelection.isNull())
  {
    painter.save();
    QRect rectToDarkenTop = pEvent->rect();
    QRect rectToDarkenBottom = pEvent->rect();

    rectToDarkenTop.setBottom(m_pParent->m_foldSelection.y());
    rectToDarkenBottom.setTop(m_pParent->m_foldSelection.y() + m_pParent->m_foldSelection.height());

    painter.fillRect(rectToDarkenTop, QColor(100, 100, 100, 100));
    painter.fillRect(rectToDarkenBottom, QColor(100, 100, 100, 100));
    painter.restore();
  }

  QFont font = document()->defaultFont();
  QFontMetrics metrics(font);
  const qint32 iFontHeight = metrics.boundingRect(QLatin1Char('9')).height();

  // paint vertical lines
  painter.save();
  painter.setFont(font);
  QColor col = palette().color(QPalette::Text);
  col.setAlpha(100);
  painter.setPen(col);
  for (QTextBlock block = document()->begin(); block != document()->end(); block = block.next())
  {
    if (block.isVisible())
    {
      if (m_pParent->Highlighter()->startsFoldingRegion(block))
      {
        QTextBlock blockEnd = m_pParent->Highlighter()->findFoldingRegionEnd(block);
        if (blockEnd.previous().isVisible() && blockEnd.isValid())
        {
          static const QRegExp rx("\\S");
          // find first char
          QTextCursor cursorStart(block);
          cursorStart.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
          qint32 iLinePos = block.text().indexOf(rx);
          cursorStart.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, iLinePos);
          QPoint topLeft = cursorRect(cursorStart).bottomLeft();

          QTextCursor cursorEnd(blockEnd);
          cursorEnd.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
          QPoint bottomLeft(topLeft.x(), cursorRect(cursorEnd).top());

          painter.drawLine(topLeft, bottomLeft);
        }
      }
    }
  }
  painter.restore();


  QTextBlock block = firstVisibleBlock();
  qint32 iBlockNumber = block.blockNumber();
  qint32 iTop = static_cast<qint32>(blockBoundingGeometry(block).translated(contentOffset()).top());
  const QRectF blockRect = blockBoundingRect(block);
  qint32 iBlockHeight = static_cast<qint32>(blockRect.height());
  qint32 iBottom = iTop + iBlockHeight;

  while (block.isValid() && iTop <= pEvent->rect().bottom())
  {
    painter.save();
    painter.setFont(font);

    const bool bTopEventOk = iBottom >= pEvent->rect().top();
    if (block.isVisible() && bTopEventOk)
    {
      if (m_pParent->Highlighter()->startsFoldingRegion(block))
      {
        QTextBlock blockEnd = m_pParent->Highlighter()->findFoldingRegionEnd(block);
        if (!blockEnd.previous().isVisible())
        {
          QTextCursor cursor(block);
          cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
          QPoint topLeft = cursorRect(cursor).topLeft();
          QRect blockRectToDraw(topLeft.x(), topLeft.y(),
                                static_cast<qint32>(blockRect.width()) - topLeft.x(),
                                iBlockHeight);

          const qint32 iDotWidth = fontMetrics().boundingRect("...").width();
          blockRectToDraw.setWidth(iDotWidth + 10);

          painter.save();
          painter.setBrush(QColor(200, 200, 200, 200));
          painter.setPen(QColor(50, 50, 50, 100));
          painter.drawRoundedRect(blockRectToDraw, 4, 4);
          painter.restore();
          painter.save();
          painter.setPen(QColor(50, 50, 50, 255));
          painter.drawText(blockRectToDraw.adjusted(5, 0, 5, 0), "...");
          painter.restore();
        }
      }

      QPointer<QLabel> pWidget = m_pParent->WidgetAreaWidget(iBlockNumber);
      if (nullptr != pWidget)
      {
        QTextCursor cursor(block);
        cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        QPoint topLeft = cursorRect(cursor).topLeft();

        QRect bgRectToDraw(topLeft.x(), topLeft.y(),
                           static_cast<qint32>(blockRect.width()) - topLeft.x(),
                           static_cast<qint32>(blockRect.height()));
        QLinearGradient linGrad(topLeft.x(), 0, bgRectToDraw.width(), 0);
        linGrad.setColorAt(0.0, QColor(255, 0, 0, 0));
        linGrad.setColorAt(30.0 / bgRectToDraw.width(), QColor(255, 0, 0, 100));
        linGrad.setColorAt(1.0, QColor(255, 0, 0, 100));

        painter.setPen(Qt::transparent);
        painter.setBrush(linGrad);
        painter.drawRect(bgRectToDraw);

        painter.setPen(QColor(200, 100, 100));
        painter.setBrush(Qt::transparent);
        painter.drawText(QPointF(topLeft.x() + 30,
                                 topLeft.y() + iFontHeight + iBlockHeight / 2 - iFontHeight / 2),
                         pWidget->toolTip().replace("\n", " "));
      }

      CCustomBlockUserData* pUserData = dynamic_cast<CCustomBlockUserData*>(block.userData());
      if (nullptr != pUserData)
      {
        for (const SMatchedWordData& wordData : pUserData->m_vMatchedWordData)
        {
          QColor colSelect(m_pParent->WordHighlightColor());
          colSelect.setAlpha(50);

          QTextCursor cursor(block);
          cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
          cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, wordData.m_iStart);
          QPoint tl = cursorRect(cursor).topLeft();
          cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, wordData.m_iLength);
          QPoint br = cursorRect(cursor).bottomRight();
          QRect blockRectToDraw = QRect(tl, br);

          painter.save();
          painter.setBrush(colSelect);
          colSelect.setAlpha(200);
          painter.setPen(colSelect);
          painter.drawRect(blockRectToDraw);
          painter.restore();
        }
      }
    }

    block = block.next();
    iTop = iBottom;
    iBottom = iTop + static_cast<qint32>(blockBoundingRect(block).height());
    ++iBlockNumber;

    painter.restore();
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorWidgetPrivate::resizeEvent(QResizeEvent* pEvent)
{
  QPlainTextEdit::resizeEvent(pEvent);

  QMargins margins = viewportMargins();
  QRect cr = contentsRect();
  for (const auto& [pos, vpAddons] : m_pParent->AdditionsMap())
  {
    switch (pos)
    {
      case CScriptEditorWidget::EScriptEditorAddonPosition::eLeft:
      {
        qint32 iTotalWidth = 0;
        for (IScriptEditorAddon* pAddon : vpAddons)
        {
          if (auto pWidget = dynamic_cast<QWidget*>(pAddon); nullptr != pWidget)
          {
            qint32 iWidth = pAddon->AreaWidth();
            qint32 iHeight = pAddon->AreaHeight() - margins.top() - margins.bottom();
            pWidget->setGeometry(QRect(cr.left() + iTotalWidth, cr.top() + margins.top(),
                                       iWidth, iHeight));
            iTotalWidth += iWidth;
          }
        }
      } break;
      case CScriptEditorWidget::EScriptEditorAddonPosition::eRight:
      {
        qint32 iTotalWidth = 0;
        for (IScriptEditorAddon* pAddon : vpAddons)
        {
          if (auto pWidget = dynamic_cast<QWidget*>(pAddon); nullptr != pWidget)
          {
            qint32 iWidth = pAddon->AreaWidth();
            qint32 iHeight = pAddon->AreaHeight() - margins.top() - margins.bottom();
            pWidget->setGeometry(QRect(cr.right() - iTotalWidth, cr.top() + margins.top(),
                                       iWidth, iHeight));
            iTotalWidth += iWidth;
          }
        }
      } break;
      case CScriptEditorWidget::EScriptEditorAddonPosition::eTop:
      {
        qint32 iTotalHeight = 0;
        for (IScriptEditorAddon* pAddon : vpAddons)
        {
          if (auto pWidget = dynamic_cast<QWidget*>(pAddon); nullptr != pWidget)
          {
            qint32 iWidth = pAddon->AreaWidth();
            qint32 iHeight = pAddon->AreaHeight();
            pWidget->setGeometry(QRect(cr.left(), cr.top() + iTotalHeight,
                                       iWidth, iHeight));
            iTotalHeight += iHeight;
          }
        }
      } break;
      case CScriptEditorWidget::EScriptEditorAddonPosition::eBottom:
      {
        qint32 iTotalHeight = 0;
        for (IScriptEditorAddon* pAddon : vpAddons)
        {
          if (auto pWidget = dynamic_cast<QWidget*>(pAddon); nullptr != pWidget)
          {
            qint32 iWidth = pAddon->AreaWidth();
            qint32 iHeight = pAddon->AreaHeight();
            pWidget->setGeometry(QRect(cr.left(), cr.bottom() - iTotalHeight - iHeight,
                                       iWidth, iHeight));
            iTotalHeight += iHeight;
          }
        }
      } break;
    }
  }

  verticalScrollBar()->setGeometry(QRect(verticalScrollBar()->x(),
                                         cr.top() + margins.top(),
                                         verticalScrollBar()->width(),
                                         cr.height() - margins.top() - margins.bottom()));
}

