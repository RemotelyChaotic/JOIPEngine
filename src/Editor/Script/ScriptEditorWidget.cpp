#include "ScriptEditorWidget.h"
#include "Application.h"
#include "ScriptEditorAddonWidgets.h"
#include "ScriptEditorCodeToolTip.h"
#include "ScriptEditorCompleter.h"
#include "ScriptEditorKeyHandler.h"
#include "Settings.h"
#include "Themes.h"

#include "Widgets/Editor/EditorCustomBlockUserData.h"
#include "Widgets/Editor/EditorSearchBar.h"
#include "Widgets/Editor/EditorHighlighter.h"
#include "Widgets/Editor/HighlightedSearchableTextEdit.h"
#include "Widgets/Editor/TextEditZoomEnabler.h"

#include <syntaxhighlighter.h>
#include <definition.h>
#include <theme.h>
#include <QtWidgets>
#include <QFontMetrics>

namespace
{
  const qint32 c_iTabStop = 2;  // 2 characters
}

//----------------------------------------------------------------------------------------
//
CScriptEditorWidget::CScriptEditorWidget(QWidget* pParent) :
  QPlainTextEdit(pParent),
  m_spRepository(std::make_unique<KSyntaxHighlighting::Repository>()),
  m_spSettings(CApplication::Instance()->Settings()),
  m_pCompleter(new CScriptEditorCompleter(this)),
  m_pHighlightedSearchableEdit(nullptr),
  m_foldedIcon(":/resources/style/img/ButtonPlay.png"),
  m_unfoldedIcon(":/resources/style/img/ButtonArrowDown.png"),
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
  m_sFontFamily("Courier New")
{
  setAttribute(Qt::WA_NoMousePropagation, false);
  installEventFilter(this);

  //for (auto def : m_spRepository->definitions()) qDebug() << def.name();

  m_spRepository->addCustomSearchPath(joip_style::ThemeFolder());
  m_pHighlightedSearchableEdit = new CHighlightedSearchableTextEdit(this);
  SlotSettingThemeChanged();

  m_pZoomEnabler = new CTextEditZoomEnabler(this);

  // register all addons
  auto pWidgetArea = new CWidgetArea(this);
  m_vpEditorAddonsMap[EScriptEditorAddonPosition::eLeft] = {
    new CLineNumberArea(this), pWidgetArea, new CFoldBlockArea(this)
  };
  m_fnWidget = std::bind(&CWidgetArea::Widget, pWidgetArea, std::placeholders::_1);
  m_vpEditorAddonsMap[EScriptEditorAddonPosition::eBottom] = {
    new CFooterArea(this, pWidgetArea)
  };

  // register key handlers
  IScriptEditorKeyHandler::RegisterHandlers(m_vpEditorKeyHandlerMap, this, &m_previouslyClickedKey);

  connect(this, &CScriptEditorWidget::blockCountChanged,
          this, &CScriptEditorWidget::UpdateLeftAreaWidth);
  connect(this, &CScriptEditorWidget::updateRequest,
          this, &CScriptEditorWidget::SlotUpdateAllAddons);
  connect(this, &CScriptEditorWidget::cursorPositionChanged,
          this, &CScriptEditorWidget::HighlightCurrentLine);
  connect(this, &CScriptEditorWidget::cursorPositionChanged,
          this, &CScriptEditorWidget::HighlightCurrentWord);

  UpdateLeftAreaWidth(0);
  UpdateBottomAreaHeight(0);
  HighlightCurrentLine();

  SlotSettingFontChanged();
  SetTabStopWidth(c_iTabStop);

  SlotSettingShowWhitespaceChanged();
  SlotSettingCaseInsensitiveSearchChanged();

  connect(m_spSettings.get(), &CSettings::editorCaseInsensitiveSearchChanged,
          this, &CScriptEditorWidget::SlotSettingCaseInsensitiveSearchChanged);
  connect(m_spSettings.get(), &CSettings::editorFontChanged,
          this, &CScriptEditorWidget::SlotSettingFontChanged);
  connect(m_spSettings.get(), &CSettings::editorShowWhitespaceChanged,
          this, &CScriptEditorWidget::SlotSettingShowWhitespaceChanged);
  connect(m_spSettings.get(), &CSettings::editorThemeChanged,
          this, &CScriptEditorWidget::SlotSettingThemeChanged);
}

CScriptEditorWidget::~CScriptEditorWidget()
{
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
void CScriptEditorWidget::SetTabStopWidth(qint32 iNumSpaces)
{
  m_iTabStopWidth = iNumSpaces;
#if QT_VERSION >= QT_VERSION_CHECK(5,10,0)
  QFontMetrics metrics(font());
  setTabStopDistance(m_iTabStopWidth * metrics.boundingRect(' ').width());
#endif
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorWidget::SetText(const QString& sText)
{
  setPlainText(sText);
  UpdateFont();
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
void CScriptEditorWidget::SetTheme(const QString& sTheme)
{
  m_sTheme = sTheme;
  SlotSettingThemeChanged();
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

  if (!isReadOnly())
  {
    QTextEdit::ExtraSelection selection;
    selection.format.setBackground(m_highlightLineColor);
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.cursor = textCursor();
    selection.cursor.clearSelection();
    vExtraSelections.append(selection);
  }

  setExtraSelections(vExtraSelections);
  repaint();
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorWidget::HighlightCurrentWord()
{
  QTextCursor cursor = textCursor();
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
void CScriptEditorWidget::SlotSettingCaseInsensitiveSearchChanged()
{
  m_pHighlightedSearchableEdit->SetCaseInsensitiveFindEnabled(
      m_spSettings->EditorCaseInsensitiveSearch());
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorWidget::SlotSettingFontChanged()
{
  m_sFontFamily = m_spSettings->EditorFont();
  UpdateFont();
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorWidget::SlotSettingShowWhitespaceChanged()
{
  QTextOption option = document()->defaultTextOption();
  if (m_spSettings->EditorShowWhitespace())
  {
    option.setFlags(QTextOption::ShowTabsAndSpaces);
  }
  else
  {
    option.setFlags(QTextOption::Flag(0x0));
  }
  document()->setDefaultTextOption(option);
  repaint();
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorWidget::SlotSettingThemeChanged()
{
  QString sThemeFromSettings = m_spSettings->EditorTheme();
  if (sThemeFromSettings.isEmpty())
  {
    Highlighter()->setTheme(m_spRepository->theme(m_sTheme));
  }
  else
  {
    Highlighter()->setTheme(m_spRepository->theme(sThemeFromSettings));
  }
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
  document()->setDefaultFont(font);
  viewport()->setFont(font);

  m_pZoomEnabler->UpdateZoom(iZoom);
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorWidget::UpdateLeftAreaWidth(qint32 iNewBlockCount)
{
  Q_UNUSED(iNewBlockCount)
  QMargins margins = viewportMargins();
  qint32 iTotalWidth = 0;
  for (IScriptEditorAddon* pAddon : m_vpEditorAddonsMap[EScriptEditorAddonPosition::eLeft])
  {
    iTotalWidth += pAddon->AreaWidth();
  }
  setViewportMargins(iTotalWidth, margins.top(), margins.right(), margins.bottom());
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorWidget::UpdateRightAreaWidth(qint32 iNewBlockCount)
{
  Q_UNUSED(iNewBlockCount)
  QMargins margins = viewportMargins();
  qint32 iTotalWidth = 0;
  for (IScriptEditorAddon* pAddon : m_vpEditorAddonsMap[EScriptEditorAddonPosition::eRight])
  {
    iTotalWidth += pAddon->AreaWidth();
  }
  setViewportMargins(margins.left(), margins.top(), iTotalWidth, margins.bottom());
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorWidget::UpdateTopAreaHeight(qint32 iNewBlockCount)
{
  Q_UNUSED(iNewBlockCount)
  QMargins margins = viewportMargins();
  qint32 iTotalHeight = 0;
  for (IScriptEditorAddon* pAddon : m_vpEditorAddonsMap[EScriptEditorAddonPosition::eTop])
  {
    iTotalHeight += pAddon->AreaHeight();
  }
  setViewportMargins(margins.left(), iTotalHeight, margins.right(), margins.bottom());
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorWidget::UpdateBottomAreaHeight(qint32 iNewBlockCount)
{
  Q_UNUSED(iNewBlockCount)
  QMargins margins = viewportMargins();
  qint32 iTotalHeight = 0;
  for (IScriptEditorAddon* pAddon : m_vpEditorAddonsMap[EScriptEditorAddonPosition::eBottom])
  {
    iTotalHeight += pAddon->AreaHeight();
  }
  setViewportMargins(margins.left(), margins.top(), margins.right(), iTotalHeight);
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

  QTextCursor cursor(document()->findBlockByLineNumber(iLine));
  moveCursor(QTextCursor::End);
  setTextCursor(cursor);
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

      QTextCursor cursor = cursorForPosition(pos);
      if (CCustomBlockUserData* pUserData =
          dynamic_cast<CCustomBlockUserData*>(cursor.block().userData());
          nullptr != pUserData)
      {
        const QRectF blockRect = blockBoundingRect(cursor.block());
        qint32 iBlockHeight = static_cast<qint32>(blockRect.height());
        cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        QPoint topLeft = cursorRect(cursor).topLeft();
        QRect blockRectToCheck(topLeft.x(), topLeft.y(),
                              static_cast<qint32>(blockRect.width()) - topLeft.x(),
                              iBlockHeight);

        const qint32 iDotWidth = fontMetrics().boundingRect("...").width();
        blockRectToCheck.setWidth(iDotWidth + 10);
        blockRectToCheck.translate(viewportMargins().left(), viewportMargins().top());

        //qDebug() << pos << cursor.blockNumber() << cursor.block().text() << blockRectToCheck;

        const QString& sFoldedContent = pUserData->FoldedContent();
        if (!sFoldedContent.isEmpty() &&
            blockRectToCheck.contains(pos))
        {
          CScriptEditorCodeToolTip::showToolTip(
              mapToGlobal(QPoint(viewportMargins().left(), blockRectToCheck.topLeft().y())),
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

  return QPlainTextEdit::eventFilter(pTarget, pEvent);
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorWidget::keyPressEvent(QKeyEvent* pEvt)
{
  // needed otherwise completer doesn't work correctly.
  bool bRet = m_pCompleter->keyPressEvent(pEvt);
  if (bRet)
  {
    return;
  }
  QPlainTextEdit::keyPressEvent(pEvt);
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorWidget::paintEvent(QPaintEvent* pEvent)
{
  QPlainTextEdit::paintEvent(pEvent);

  QPainter painter(viewport());
  if (!m_foldSelection.isNull())
  {
    painter.save();
    QRect rectToDarkenTop = pEvent->rect();
    QRect rectToDarkenBottom = pEvent->rect();

    rectToDarkenTop.setBottom(m_foldSelection.y());
    rectToDarkenBottom.setTop(m_foldSelection.y() + m_foldSelection.height());

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
      if (Highlighter()->startsFoldingRegion(block))
      {
        QTextBlock blockEnd = Highlighter()->findFoldingRegionEnd(block);
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
      if (Highlighter()->startsFoldingRegion(block))
      {
        QTextBlock blockEnd = Highlighter()->findFoldingRegionEnd(block);
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

      QPointer<QLabel> pWidget = WidgetAreaWidget(iBlockNumber);
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
          QColor colSelect(m_wordhighlightColor);
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
void CScriptEditorWidget::resizeEvent(QResizeEvent* pEvent)
{
  QPlainTextEdit::resizeEvent(pEvent);

  QMargins margins = viewportMargins();
  QRect cr = contentsRect();
  for (const auto& [pos, vpAddons] : m_vpEditorAddonsMap)
  {
    switch (pos)
    {
      case EScriptEditorAddonPosition::eLeft:
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
      case EScriptEditorAddonPosition::eRight:
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
      case EScriptEditorAddonPosition::eTop:
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
      case EScriptEditorAddonPosition::eBottom:
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

//----------------------------------------------------------------------------------------
//
QPointer<QLabel> CScriptEditorWidget::WidgetAreaWidget(qint32 iLine)
{
  return m_fnWidget(iLine);
}

