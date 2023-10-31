#include "ScriptEditorWidget.h"
#include "ScriptEditorAddonWidgets.h"
#include "ScriptEditorKeyHandler.h"

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
CCustomBlockUserData::CCustomBlockUserData(KSyntaxHighlighting::TextBlockUserData* pOldData) :
  KSyntaxHighlighting::TextBlockUserData()
{
  if (nullptr != pOldData)
  {
    state = pOldData->state;
    foldingRegions = pOldData->foldingRegions;
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
CScriptEditorWidget::CScriptEditorWidget(QWidget* pParent) :
  QPlainTextEdit(pParent),
  m_spRepository(std::make_unique<KSyntaxHighlighting::Repository>()),
  m_pHighlightedSearchableEdit(nullptr),
  m_foldedIcon(":/resources/style/img/ButtonPlay.png"),
  m_unfoldedIcon(":/resources/style/img/ButtonArrowDown.png"),
  m_foldAreaBackgroundColor(24, 24, 24),
  m_lineNumberBackgroundColor(24, 24, 24),
  m_lineNumberTextColor(Qt::white),
  m_highlightLineColor(68, 71, 90),
  m_widgetsBackgroundColor(24, 24, 24),
  m_previouslyClickedKey(Qt::Key(0))
{
  setAttribute(Qt::WA_NoMousePropagation, false);
  installEventFilter(this);

  m_pHighlightedSearchableEdit = new CHighlightedSearchableTextEdit(this);
  m_pHighlightedSearchableEdit->Highlighter()->setTheme(m_spRepository->theme(m_sTheme));

  m_pZoomEnabler = new CTextEditZoomEnabler(this);

  // register all addons
  auto pWidgetArea = new CWidgetArea(this);
  m_vpEditorAddonsMap[EScriptEditorAddonPosition::eLeft] = {
    new CLineNumberArea(this), pWidgetArea, new CFoldBlockArea(this)
  };
  m_fnWidget = std::bind(&CWidgetArea::Widget, pWidgetArea, std::placeholders::_1);

  // register key handlers
  IScriptEditorKeyHandler::RegisterHandlers(m_vpEditorKeyHandlerMap, this, &m_previouslyClickedKey);

  connect(this, &CScriptEditorWidget::blockCountChanged,
          this, &CScriptEditorWidget::UpdateLeftAreaWidth);
  connect(this, &CScriptEditorWidget::updateRequest,
          this, &CScriptEditorWidget::SlotUpdateAllAddons);
  connect(this, &CScriptEditorWidget::cursorPositionChanged,
          this, &CScriptEditorWidget::HighlightCurrentLine);

  UpdateLeftAreaWidth(0);
  HighlightCurrentLine();

  QFont font;
  font.setFamily("Courier");
  font.setStyleHint(QFont::Monospace);
  font.setFixedPitch(true);
  font.setPointSize(10);
  document()->setDefaultFont(font);

  QFontMetrics metrics(font);
  setTabStopDistance(c_iTabStop * metrics.boundingRect(' ').width());

  QTextOption option = document()->defaultTextOption();
  option.setFlags(QTextOption::ShowTabsAndSpaces);
  document()->setDefaultTextOption(option);
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
void CScriptEditorWidget::SetHighlightSearchBackgroundColor(const QColor& color)
{
  if (m_highlightSearchBackgroundColor != color)
  {
    m_highlightSearchBackgroundColor = color;
    m_pHighlightedSearchableEdit->Highlighter()->SetSearchColors(m_highlightSearchBackgroundColor,
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
    m_pHighlightedSearchableEdit->Highlighter()->SetSearchColors(m_highlightSearchBackgroundColor,
                                                                 m_highlightSearchColor);
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorWidget::SetTheme(const QString& sTheme)
{
  m_sTheme = sTheme;
  m_pHighlightedSearchableEdit->Highlighter()->setTheme(m_spRepository->theme(m_sTheme));
}

//----------------------------------------------------------------------------------------
//
QPointer<CEditorHighlighter> CScriptEditorWidget::Highlighter() const
{ return m_pHighlightedSearchableEdit->Highlighter(); }
QPointer<CEditorSearchBar>   CScriptEditorWidget::SearchBar() const
{ return m_pHighlightedSearchableEdit->SearchBar(); }

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
      UpdateLeftAreaWidth(iNewBlockCount);
    case EScriptEditorAddonPosition::eRight:
      UpdateRightAreaWidth(iNewBlockCount);
    case EScriptEditorAddonPosition::eTop:
      UpdateTopAreaHeight(iNewBlockCount);
    case EScriptEditorAddonPosition::eBottom:
      UpdateBottomAreaHeight(iNewBlockCount);
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorWidget::ResetAddons()
{
  for (const auto& [pos, vpAddons] : m_vpEditorAddonsMap)
  {
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
  for (IScriptEditorAddon* pAddon : m_vpEditorAddonsMap[EScriptEditorAddonPosition::eTop])
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
          QToolTip::showText(
              mapToGlobal(QPoint(viewportMargins().left(), blockRectToCheck.topLeft().y())),
              sFoldedContent);
        }
        else
        {
          QToolTip::hideText();
        }
      }
      else
      {
        QToolTip::hideText();
      }
      return true;
    }
  }

  return QPlainTextEdit::eventFilter(pTarget, pEvent);
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

  QTextBlock block = firstVisibleBlock();
  qint32 iBlockNumber = block.blockNumber();
  qint32 iTop = static_cast<qint32>(blockBoundingGeometry(block).translated(contentOffset()).top());
  const QRectF blockRect = blockBoundingRect(block);
  qint32 iBlockHeight = static_cast<qint32>(blockRect.height());
  qint32 iBottom = iTop + iBlockHeight;
  const qint32 iFontHeight = fontMetrics().boundingRect(QLatin1Char('9')).height();

  while (block.isValid() && iTop <= pEvent->rect().bottom())
  {
    const bool bTopEventOk = iBottom >= pEvent->rect().top();
    if (block.isVisible() && bTopEventOk)
    {
      if (Highlighter()->startsFoldingRegion(block))
      {
        if (!Highlighter()->findFoldingRegionEnd(block).previous().isVisible())
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
    }

    block = block.next();
    iTop = iBottom;
    iBottom = iTop + static_cast<qint32>(blockBoundingRect(block).height());
    ++iBlockNumber;
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorWidget::resizeEvent(QResizeEvent* pEvent)
{
  QPlainTextEdit::resizeEvent(pEvent);

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
            pWidget->setGeometry(QRect(cr.left() + iTotalWidth, cr.top(), iWidth, cr.height()));
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
            pWidget->setGeometry(QRect(cr.right() - iTotalWidth, cr.top(), iWidth, cr.height()));
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
            qint32 iHeight = pAddon->AreaHeight();
            pWidget->setGeometry(QRect(cr.left(), cr.top() + iTotalHeight, cr.width(), iHeight));
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
            qint32 iHeight = pAddon->AreaHeight();
            pWidget->setGeometry(QRect(cr.left(), cr.bottom() - iTotalHeight, cr.width(), iHeight));
            iTotalHeight += iHeight;
          }
        }
      } break;
    }
  }
}

//----------------------------------------------------------------------------------------
//
QPointer<QLabel> CScriptEditorWidget::WidgetAreaWidget(qint32 iLine)
{
  return m_fnWidget(iLine);
}

