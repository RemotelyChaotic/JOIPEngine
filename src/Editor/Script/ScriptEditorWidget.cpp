#include "ScriptEditorWidget.h"
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
  m_previouslyClickedKey(Qt::Key(0)),
  m_foldSelection()
{
  setAttribute(Qt::WA_NoMousePropagation, false);
  installEventFilter(this);

  m_pHighlightedSearchableEdit = new CHighlightedSearchableTextEdit(this);
  m_pHighlightedSearchableEdit->Highlighter()->setTheme(m_spRepository->theme(m_sTheme));

  m_pZoomEnabler = new CTextEditZoomEnabler(this);

  m_pLineNumberArea = new CLineNumberArea(this);
  m_pWidgetArea = new CWidgetArea(this);
  m_pFoldBlockArea = new CFoldBlockArea(this);

  connect(this, &CScriptEditorWidget::blockCountChanged,
          this, &CScriptEditorWidget::UpdateLeftAreaWidth);
  connect(this, &CScriptEditorWidget::updateRequest,
          this, &CScriptEditorWidget::UpdateLineNumberArea);
  connect(this, &CScriptEditorWidget::updateRequest,
          this, &CScriptEditorWidget::UpdateWidgetArea);
  connect(this, &CScriptEditorWidget::updateRequest,
          this, &CScriptEditorWidget::UpdateFoldBlockArea);
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

//----------------------------------------------------------------------------------------
//
void CScriptEditorWidget::SetHighlightDefinition(const QString& sType)
{
  const auto def = m_spRepository->definitionForName(sType);
  m_pHighlightedSearchableEdit->Highlighter()->setDefinition(def);
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
void CScriptEditorWidget::FoldBlockAreaMouseEvent(QMouseEvent* pEvt)
{
  QTextBlock block = firstVisibleBlock();
  qint32 iBlockNumber = block.blockNumber();
  qint32 iTop = static_cast<qint32>(blockBoundingGeometry(block).translated(contentOffset()).top());
  qint32 iBottom = iTop + static_cast<qint32>(blockBoundingRect(block).height());

  while (block.isValid() && iTop <= rect().bottom())
  {
    if (iBottom >= rect().top() &&
        m_pHighlightedSearchableEdit->Highlighter()->startsFoldingRegion(block))
    {
      QTextBlock blockEnd = m_pHighlightedSearchableEdit->Highlighter()->findFoldingRegionEnd(block);
      bool bToggleFolding = false;
      if (block.isVisible())
      {
        QRectF iconBox = QRectF(0, iTop, m_pFoldBlockArea->width(), fontMetrics().height());
        if (iconBox.contains(pEvt->localPos()))
        {
          bToggleFolding = true;
        }
      }
      else
      {
        qint32 iEndTop = static_cast<qint32>(blockBoundingGeometry(blockEnd).translated(contentOffset()).top());
        QRectF iconBox = QRectF(0, iEndTop, m_pFoldBlockArea->width(), fontMetrics().height());

        if (iconBox.contains(pEvt->localPos()))
        {
          bToggleFolding = true;
        }
      }

      if (bToggleFolding)
      {
        QTextBlock current = block;
        while (blockEnd != current)
        {
          current = current.next();
          current.setVisible(!current.isVisible());
        }

        LineNumberArea()->update(0, viewport()->rect().y(),
                                 LineNumberAreaWidth(),
                                 viewport()->rect().height());
        WidgetArea()->update(0, viewport()->rect().y(),
                             WidgetAreaWidth(),
                             viewport()->rect().height());
        update();
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
void CScriptEditorWidget::FoldBlockAreaPaintEvent(QPaintEvent* pEvent)
{
  QPainter painter(m_pFoldBlockArea);
  painter.fillRect(pEvent->rect(), m_foldAreaBackgroundColor);

  bool bNeedsRepaintOfContent = false;
  if (!m_foldSelection.isNull())
  {
    m_foldSelection = QRect();
    bNeedsRepaintOfContent = true;
  }

  double dLuminance = (0.299 * m_foldAreaBackgroundColor.red() +
                       0.587 * m_foldAreaBackgroundColor.green() +
                       0.114 * m_foldAreaBackgroundColor.blue()) / 255;
  QColor selection = dLuminance > 0.5 ? m_lineNumberBackgroundColor.darker() :
                                        m_lineNumberBackgroundColor.lighter();

  QTextBlock block = firstVisibleBlock();
  qint32 iBlockNumber = block.blockNumber();
  qint32 iTop = static_cast<qint32>(blockBoundingGeometry(block).translated(contentOffset()).top());
  qint32 iBottom = iTop + static_cast<qint32>(blockBoundingRect(block).height());

  while (block.isValid() && iTop <= pEvent->rect().bottom())
  {
    if (iBottom >= pEvent->rect().top() &&
        m_pHighlightedSearchableEdit->Highlighter()->startsFoldingRegion(block))
    {
      if (block.isVisible())
      {
        QPoint globalCursorPos = QCursor::pos();
        QRect iconBox = QRect(0, iTop, m_pFoldBlockArea->width(), fontMetrics().height());
        QPoint localPos = m_pFoldBlockArea->mapFromGlobal(globalCursorPos);
        if (iconBox.contains(localPos))
        {
          QTextBlock blockEnd = m_pHighlightedSearchableEdit->Highlighter()->findFoldingRegionEnd(block);
          qint32 iFoldingBlockEnd = 0;
          iFoldingBlockEnd =
              static_cast<qint32>(blockBoundingGeometry(blockEnd).translated(contentOffset()).top())+
                  fontMetrics().height();
          QRect foldingBlockRect = QRect(iconBox); foldingBlockRect.setHeight(iFoldingBlockEnd-iTop);
          painter.fillRect(foldingBlockRect, selection);
          m_foldSelection = foldingBlockRect;
          bNeedsRepaintOfContent = true;
        }
        m_unfoldedIcon.paint(&painter, iconBox);
      }
      else
      {
        m_foldedIcon.paint(&painter,
                           QRect(0, iTop, m_pFoldBlockArea->width(), m_pFoldBlockArea->width()));
      }
    }

    block = block.next();
    iTop = iBottom;
    iBottom = iTop + static_cast<qint32>(blockBoundingRect(block).height());
    ++iBlockNumber;
  }

  if (bNeedsRepaintOfContent)
  {
    update();
  }
}

//----------------------------------------------------------------------------------------
//
qint32 CScriptEditorWidget::FoldBlockAreaWidth() const
{
  return 16;
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorWidget::LineNumberAreaPaintEvent(QPaintEvent* pEvent)
{
  QPainter painter(m_pLineNumberArea);
  painter.fillRect(pEvent->rect(), m_lineNumberBackgroundColor);

  QTextBlock block = firstVisibleBlock();
  qint32 iBlockNumber = block.blockNumber();
  qint32 iTop = static_cast<qint32>(blockBoundingGeometry(block).translated(contentOffset()).top());
  qint32 iBottom = iTop + static_cast<qint32>(blockBoundingRect(block).height());

  while (block.isValid() && iTop <= pEvent->rect().bottom())
  {
    if (block.isVisible() && iBottom >= pEvent->rect().top())
    {
      QString number = QString::number(iBlockNumber + 1);
      painter.setPen(m_lineNumberTextColor);
      painter.drawText(0, iTop, m_pLineNumberArea->width(), fontMetrics().height(),
                       Qt::AlignRight, number);
    }

    block = block.next();
    iTop = iBottom;
    iBottom = iTop + static_cast<qint32>(blockBoundingRect(block).height());
    ++iBlockNumber;
  }
}

//----------------------------------------------------------------------------------------
//
qint32 CScriptEditorWidget::LineNumberAreaWidth() const
{
  qint32 iDigits = 1;
  qint32 iMax = qMax(1, blockCount());
  while (iMax >= 10)
  {
    iMax /= 10;
    ++iDigits;
  }

  qint32 iSpace = 3 + fontMetrics().boundingRect(QLatin1Char('9')).width() * iDigits;
  return iSpace;
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorWidget::ResetWidget()
{
  m_pWidgetArea->HideAllErrors();
  m_pWidgetArea->ClearAllErrors();
  m_pWidgetArea->repaint();
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorWidget::WidgetAreaPaintEvent(QPaintEvent* pEvent)
{
  QPainter painter(m_pWidgetArea);
  painter.fillRect(pEvent->rect(), m_widgetsBackgroundColor);

  QTextBlock block = firstVisibleBlock();
  qint32 iBlockNumber = block.blockNumber();
  qint32 iTop = static_cast<qint32>(blockBoundingGeometry(block).translated(contentOffset()).top());
  const QRectF blockRect = blockBoundingRect(block);
  qint32 iBlockHeight = static_cast<qint32>(blockRect.height());
  qint32 iBottom = iTop + iBlockHeight;

  while (block.isValid() && iTop <= pEvent->rect().bottom())
  {
    if (block.isVisible() && iBottom >= pEvent->rect().top())
    {
      QPointer<QLabel> pWidget = m_pWidgetArea->Widget(iBlockNumber);
      if (nullptr != pWidget)
      {
        pWidget->setGeometry(0, iTop, pWidget->width(), pWidget->height());
        QPixmap pixmap = pWidget->grab();
        painter.drawPixmap(pWidget->geometry(), pixmap, pixmap.rect());
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
qint32 CScriptEditorWidget::WidgetAreaWidth() const
{
  return 16;
}

//----------------------------------------------------------------------------------------
//
QMenu* CScriptEditorWidget::CreateContextMenu()
{
  return m_pHighlightedSearchableEdit->CreateContextMenu();
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
  setViewportMargins(WidgetAreaWidth() + LineNumberAreaWidth() + FoldBlockAreaWidth(), 0, 0, 0);
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorWidget::UpdateFoldBlockArea(const QRect& rect, qint32 iDy)
{
  if (0 != iDy)
  {
    m_pFoldBlockArea->scroll(0, iDy);
  }
  else
  {
    m_pFoldBlockArea->update(0, 0, m_pFoldBlockArea->width(), m_pFoldBlockArea->height());
  }

  if (rect.contains(viewport()->rect()))
  {
    UpdateLeftAreaWidth(0);
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorWidget::UpdateLineNumberArea(const QRect& rect, qint32 iDy)
{
  if (0 != iDy)
  {
    m_pLineNumberArea->scroll(0, iDy);
  }
  else
  {
    m_pLineNumberArea->update(0, rect.y(), m_pLineNumberArea->width(), rect.height());
  }

  if (rect.contains(viewport()->rect()))
  {
    UpdateLeftAreaWidth(0);
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorWidget::UpdateWidgetArea(const QRect& rect, qint32 iDy)
{
  if (0 != iDy)
  {
    m_pWidgetArea->scroll(0, iDy);
  }
  else
  {
    m_pWidgetArea->update(0, rect.y(), m_pWidgetArea->width(), rect.height());
  }

  if (rect.contains(viewport()->rect()))
  {
    UpdateLeftAreaWidth(0);
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorWidget::SlotExecutionError(QString sException, qint32 iLine, QString sStack)
{
  m_pWidgetArea->ClearAllErrors();
  m_pWidgetArea->AddError(sException, iLine, sStack);
  m_pWidgetArea->update();

  QTextCursor cursor(document()->findBlockByLineNumber(iLine));
  moveCursor(QTextCursor::End);
  setTextCursor(cursor);
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
      // Enter
      if (pKeyEvent->key() == Qt::Key_Enter || pKeyEvent->key() == Qt::Key_Return)
      {
        // get indentation of current line, and mimic for new line
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::StartOfLine);
        cursor.select(QTextCursor::SelectionType::LineUnderCursor);
        QString sSelectedText = cursor.selectedText();
        QString sIndentation;
        for (qint32 i = 0; i < sSelectedText.length(); ++i)
        {
          if (sSelectedText[i].isSpace())
          {
            sIndentation += sSelectedText[i];
          }
          else
          {
            break;
          }
        }
        insertPlainText(QString("\n") + sIndentation);
        m_previouslyClickedKey = Qt::Key(pKeyEvent->key());
        pEvent->ignore();
        return true;
      }
      // Tab
      else if (pKeyEvent->key() == Qt::Key_Tab)
      {
        QTextCursor cursor = textCursor();
        qint32 iSelectedLines = 0;
        QString sSelection;
        if(cursor.hasSelection())
        {
            sSelection = cursor.selection().toPlainText();
            iSelectedLines = sSelection.count("\n")+1;
        }

        if (2 > iSelectedLines)
        {
          insertPlainText(QString("").leftJustified(4, ' ', false));
          pEvent->ignore();
        }
        else
        {
          QStringList vsLines = sSelection.split("\n");
          for (QString& sLine : vsLines)
          {
            sLine.prepend(QString("").leftJustified(4, ' ', false));
          }
          m_previouslyClickedKey = Qt::Key(pKeyEvent->key());
          insertPlainText(vsLines.join("\n"));
          pEvent->ignore();
        }
        // workaround for losing focus after blocking tab
        QMetaObject::invokeMethod(this, "setFocus", Qt::QueuedConnection);
        return true;
      }
      // brackets
      else if (pKeyEvent->key() == Qt::Key_BraceLeft)
      {
        insertPlainText(QString("{  }"));
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, 2);
        setTextCursor(cursor);
        m_previouslyClickedKey = Qt::Key(pKeyEvent->key());
        pEvent->ignore();
        return true;
      }
      else if (pKeyEvent->key() == Qt::Key_BraceRight &&
              Qt::Key_BraceLeft == m_previouslyClickedKey)
      {
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, 2);
        setTextCursor(cursor);
        m_previouslyClickedKey = Qt::Key(0);
        pEvent->ignore();
        return true;
      }
      else if (pKeyEvent->key() == Qt::Key_ParenLeft)
      {
        insertPlainText(QString("()"));
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, 1);
        m_previouslyClickedKey = Qt::Key(pKeyEvent->key());
        setTextCursor(cursor);
        pEvent->ignore();
        return true;
      }
      else if (pKeyEvent->key() == Qt::Key_ParenRight &&
              Qt::Key_ParenLeft == m_previouslyClickedKey)
      {
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, 1);
        setTextCursor(cursor);
        m_previouslyClickedKey = Qt::Key(0);
        pEvent->ignore();
        return true;
      }
      else if (pKeyEvent->key() == Qt::Key_BracketLeft)
      {
        insertPlainText(QString("[]"));
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, 1);
        setTextCursor(cursor);
        m_previouslyClickedKey = Qt::Key(pKeyEvent->key());
        pEvent->ignore();
        return true;
      }
      else if (pKeyEvent->key() == Qt::Key_BracketRight &&
              Qt::Key_BracketLeft == m_previouslyClickedKey)
      {
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, 1);
        setTextCursor(cursor);
        m_previouslyClickedKey = Qt::Key(0);
        pEvent->ignore();
        return true;
      }
      // Quotes
      else if (pKeyEvent->key() == Qt::Key_QuoteDbl)
      {
        if (Qt::Key_QuoteDbl != m_previouslyClickedKey)
        {
          insertPlainText(QString("\"\""));
          QTextCursor cursor = textCursor();
          cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, 1);
          setTextCursor(cursor);
          m_previouslyClickedKey = Qt::Key(pKeyEvent->key());
        }
        else
        {
          QTextCursor cursor = textCursor();
          cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, 1);
          setTextCursor(cursor);
          m_previouslyClickedKey = Qt::Key(0);
        }
        pEvent->ignore();
        return true;
      }

      m_previouslyClickedKey = Qt::Key(pKeyEvent->key());
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
        if (!Highlighter()->findFoldingRegionEnd(block).isVisible())
        {
          QRectF blockRect = blockBoundingRect(block);
          blockRect.setHeight(fontMetrics().height()-4);
          blockRect.translate(0, iTop+2);

          QString sText = block.text();
          const qint32 iTextWidth = fontMetrics().boundingRect(sText).width();
          blockRect.setLeft(blockRect.left() + iTextWidth + 10);
          const qint32 iDotWidth = fontMetrics().boundingRect("...").width();
          blockRect.setWidth(iDotWidth + 10);

          painter.save();
          painter.setBrush(QColor(200, 200, 200, 200));
          painter.setPen(QColor(50, 50, 50, 100));
          painter.drawRoundedRect(blockRect, 4, 4);
          painter.restore();
          painter.save();
          painter.setPen(QColor(50, 50, 50, 255));
          painter.drawText(blockRect.adjusted(5, 0, 5, 0), "...");
          painter.restore();
        }
      }

      QPointer<QLabel> pWidget = m_pWidgetArea->Widget(iBlockNumber);
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
  m_pWidgetArea->setGeometry(QRect(cr.left(), cr.top(), WidgetAreaWidth(), cr.height()));
  m_pLineNumberArea->setGeometry(QRect(cr.left() + WidgetAreaWidth(), cr.top(), LineNumberAreaWidth(), cr.height()));
  m_pFoldBlockArea->setGeometry(QRect(cr.left() + WidgetAreaWidth() + LineNumberAreaWidth(),
                                      cr.top(), FoldBlockAreaWidth(), cr.height()));
}

//----------------------------------------------------------------------------------------
//
void CWidgetArea::AddError(QString sException, qint32 iLine, QString sStack)
{
  QLabel* pLabel = new QLabel(this);
  pLabel->setVisible(true);
  pLabel->setFixedSize(m_pCodeEditor->WidgetAreaWidth(),m_pCodeEditor->WidgetAreaWidth());
  pLabel->setObjectName(QString::fromUtf8("ErrorIcon"));
  pLabel->setAccessibleName(QString::fromUtf8("ErrorIcon"));
  pLabel->setToolTip(sException + ": " + sStack);
  pLabel->installEventFilter(this);
  pLabel->raise();
  pLabel->style()->polish(pLabel);
  m_errorLabelMap.insert({ iLine, pLabel });
}

//----------------------------------------------------------------------------------------
//
void CWidgetArea::ClearAllErrors()
{
  for (auto it = m_errorLabelMap.begin(); m_errorLabelMap.end() != it; ++it)
  {
    if (nullptr != it->second)
    {
      delete it->second;
    }
  }
  m_errorLabelMap.clear();
}

//----------------------------------------------------------------------------------------
//
void CWidgetArea::HideAllErrors()
{
  for (auto it = m_errorLabelMap.begin(); m_errorLabelMap.end() != it; ++it)
  {
    if (nullptr != it->second)
    {
      it->second->hide();
    }
  }
}

//----------------------------------------------------------------------------------------
//
QPointer<QLabel> CWidgetArea::Widget(qint32 iLine)
{
  auto it = m_errorLabelMap.find(iLine);
  if (m_errorLabelMap.end() != it)
  {
    return it->second;
  }
  return nullptr;
}

//----------------------------------------------------------------------------------------
//
bool CWidgetArea::eventFilter(QObject* pObject, QEvent* pEvent)
{
  if (nullptr != pObject && nullptr != pEvent)
  {
    QLabel* pErrorLabel = qobject_cast<QLabel*>(pObject);
    if (nullptr != pErrorLabel)
    {
      if (pEvent->type() == QEvent::MouseButtonPress)
      {
        QMouseEvent* pMouseEvent = static_cast<QMouseEvent*>(pEvent);
        QToolTip::showText(pMouseEvent->globalPos(), pErrorLabel->toolTip());
      }
    }
  }
  return false;
}

//----------------------------------------------------------------------------------------
//
void CWidgetArea::paintEvent(QPaintEvent *event)
{
  m_pCodeEditor->WidgetAreaPaintEvent(event);
}

//----------------------------------------------------------------------------------------
//
void CFoldBlockArea::mousePressEvent(QMouseEvent* pEvt)
{
  if (nullptr != pEvt)
  {
    m_pCodeEditor->FoldBlockAreaMouseEvent(pEvt);
  }
}

//----------------------------------------------------------------------------------------
//
void CFoldBlockArea::paintEvent(QPaintEvent *event)
{
  m_pCodeEditor->FoldBlockAreaPaintEvent(event);
}

