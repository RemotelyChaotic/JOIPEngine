#include "ScriptEditorWidget.h"
#include "Widgets/Editor/EditorSearchBar.h"
#include "Widgets/Editor/EditorHighlighter.h"
#include <syntaxhighlighter.h>
#include <definition.h>
#include <theme.h>
#include <repository.h>
#include <QtWidgets>
#include <QFontMetrics>

namespace
{
  const qint32 c_iTabStop = 2;  // 2 characters
}

CScriptEditorWidget::CScriptEditorWidget(QWidget* pParent) :
  QPlainTextEdit(pParent),
  m_spRepository(std::make_unique<KSyntaxHighlighting::Repository>()),
  m_pHighlighter(nullptr),
  m_lineNumberBackgroundColor(24, 24, 24),
  m_lineNumberTextColor(Qt::white),
  m_highlightLineColor(68, 71, 90),
  m_widgetsBackgroundColor(24, 24, 24),
  m_highlightCursor(),
  m_sLastSearch(),
  m_previouslyClickedKey(Qt::Key(0))
{
  setAttribute(Qt::WA_NoMousePropagation, false);
  installEventFilter(this);

  m_pHighlighter = new CEditorHighlighter(document());
  m_pHighlighter->setTheme(m_spRepository->theme(m_sTheme));

  m_pLineNumberArea = new CLineNumberArea(this);
  m_pWidgetArea = new CWidgetArea(this);
  m_pSearchBar = new CEditorSearchBar(this);
  m_pSearchBar->Climb();
  m_pSearchBar->Resize();

  // reset things after closing search bar
  connect(m_pSearchBar, &CEditorSearchBar::SignalHidden,
          this, &CScriptEditorWidget::SearchAreaHidden);
  connect(m_pSearchBar, &CEditorSearchBar::SignalFilterChanged,
          this, &CScriptEditorWidget::SlotSearchFilterChanged);

  connect(this, &CScriptEditorWidget::blockCountChanged,
          this, &CScriptEditorWidget::UpdateLeftAreaWidth);
  connect(this, &CScriptEditorWidget::updateRequest,
          this, &CScriptEditorWidget::UpdateLineNumberArea);
  connect(this, &CScriptEditorWidget::updateRequest,
          this, &CScriptEditorWidget::UpdateWidgetArea);
  connect(this, &CScriptEditorWidget::cursorPositionChanged,
          this, &CScriptEditorWidget::HighlightCurrentLine);

  UpdateLeftAreaWidth(0);
  HighlightCurrentLine();

  QFont font;
  font.setFamily("Courier");
  font.setStyleHint(QFont::Monospace);
  font.setFixedPitch(true);
  font.setPointSize(10);
  setFont(font);

  QFontMetrics metrics(font);
  setTabStopWidth(c_iTabStop * metrics.width(' '));

  QTextOption option = document()->defaultTextOption();
  option.setFlags(QTextOption::ShowTabsAndSpaces);
  document()->setDefaultTextOption(option);
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorWidget::SetHighlightDefinition(const QString& sType)
{
  const auto def = m_spRepository->definitionForName(sType);
  m_pHighlighter->setDefinition(def);
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorWidget::SetHighlightSearchBackgroundColor(const QColor& color)
{
  if (m_highlightSearchBackgroundColor != color)
  {
    m_highlightSearchBackgroundColor = color;
    m_pHighlighter->SetSearchColors(m_highlightSearchBackgroundColor,
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
    m_pHighlighter->SetSearchColors(m_highlightSearchBackgroundColor,
                                    m_highlightSearchColor);
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorWidget::SetTheme(const QString& sTheme)
{
  m_sTheme = sTheme;
  m_pHighlighter->setTheme(m_spRepository->theme(m_sTheme));
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
qint32 CScriptEditorWidget::LineNumberAreaWidth()
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
qint32 CScriptEditorWidget::WidgetAreaWidth()
{
  return 16;
}

//----------------------------------------------------------------------------------------
//
QMenu* CScriptEditorWidget::CreateContextMenu()
{
  QMenu* pMenu = createStandardContextMenu();
  pMenu->addAction(tr("Search"), this, SLOT(SlotShowHideSearchFilter()),
                   QKeySequence::Find);
  return pMenu;
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
  setViewportMargins(LineNumberAreaWidth() + WidgetAreaWidth(), 0, 0, 0);
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
void CScriptEditorWidget::SearchAreaHidden()
{
  if (!m_highlightCursor.isNull())
  {
    setTextCursor(m_highlightCursor);
  }
  m_highlightCursor = QTextCursor();
  m_sLastSearch = QString();
  m_pHighlighter->SetSearchExpression(QString());
  setFocus();
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
void CScriptEditorWidget::SlotShowHideSearchFilter()
{
  if (m_pSearchBar->isVisible())
  {
    m_pSearchBar->Hide();
  }
  else
  {
    QTextCursor cursor = textCursor();
    m_pSearchBar->SetFilter(cursor.selectedText());
    m_pSearchBar->Show();
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorWidget::SlotSearchFilterChanged(bool bForward, const QString& sText)
{
  QTextDocument* pDocument = document();

  if (!sText.isEmpty())
  {
    QTextCursor highlightCursor = m_highlightCursor;
    if (m_highlightCursor.isNull() || m_sLastSearch.isEmpty() || m_sLastSearch != sText)
    {
      highlightCursor = QTextCursor(pDocument);
    }

    // update highlighting
    if (m_sLastSearch != sText)
    {
      m_pHighlighter->SetSearchExpression(sText);
    }

    m_sLastSearch = sText;
    highlightCursor =
        pDocument->find(sText, highlightCursor, bForward ? QTextDocument::FindFlags() :
                                                           QTextDocument::FindBackward);
    if (!highlightCursor.isNull())
    {
      setTextCursor(highlightCursor);
      m_highlightCursor = highlightCursor;
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorWidget::contextMenuEvent(QContextMenuEvent* pEvent)
{
  if (nullptr != pEvent)
  {
    QPointer<CScriptEditorWidget> pThisGuard(this);
    QMenu* pMenu = CreateContextMenu();
    pMenu->exec(pEvent->globalPos());
    if (nullptr == pThisGuard) { return; }
    delete pMenu;
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
      // Ctrl+F
      if (pKeyEvent->modifiers().testFlag(Qt::ControlModifier) && pKeyEvent->key() == Qt::Key_F)
      {
        SlotShowHideSearchFilter();
      }
      // Enter
      else if (pKeyEvent->key() == Qt::Key_Enter || pKeyEvent->key() == Qt::Key_Return)
      {
        if (m_pSearchBar->isVisible() && !m_sLastSearch.isEmpty() && focusWidget() != this)
        {
          SlotSearchFilterChanged(m_pSearchBar->IsSearchingForward(), m_sLastSearch);
          // don't insert line break, just jump to next search result
          m_previouslyClickedKey = Qt::Key(pKeyEvent->key());
          pEvent->ignore();
          return true;
        }
        else
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

  QTextBlock block = firstVisibleBlock();
  qint32 iBlockNumber = block.blockNumber();
  qint32 iTop = static_cast<qint32>(blockBoundingGeometry(block).translated(contentOffset()).top());
  const QRectF blockRect = blockBoundingRect(block);
  qint32 iBlockHeight = static_cast<qint32>(blockRect.height());
  qint32 iBottom = iTop + iBlockHeight;
  const qint32 iFontHeight = fontMetrics().boundingRect(QLatin1Char('9')).height();

  while (block.isValid() && iTop <= pEvent->rect().bottom())
  {
    if (block.isVisible() && iBottom >= pEvent->rect().top())
    {
      QPointer<QLabel> pWidget = m_pWidgetArea->Widget(iBlockNumber);
      if (nullptr != pWidget)
      {
        QTextCursor cursor(block);
        cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        QPoint topLeft = cursorRect(cursor).topLeft();

        QRect bgRectToDraw(topLeft.x(), topLeft.y(),
                           static_cast<qint32>(blockRect.width()) - topLeft.x(),
                           static_cast<qint32>(blockRect.height()));
        QLinearGradient linGrad(0, 0, bgRectToDraw.width(), 0);
        linGrad.setColorAt(0.0, QColor(255, 0, 0, 0));
        linGrad.setColorAt(0.4, QColor(255, 0, 0, 50));
        linGrad.setColorAt(0.6, QColor(255, 0, 0, 50));
        linGrad.setColorAt(1.0, QColor(255, 0, 0, 0));

        painter.setPen(Qt::transparent);
        painter.setBrush(linGrad);
        painter.drawRect(bgRectToDraw);

        painter.setPen(QColor(200, 100, 100));
        painter.setBrush(Qt::transparent);
        painter.drawText(QPointF(topLeft.x() + 30, topLeft.y() - iFontHeight / 2 + iBlockHeight),
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
  m_pLineNumberArea->setGeometry(QRect(cr.left(), cr.top(), LineNumberAreaWidth(), cr.height()));
  m_pWidgetArea->setGeometry(QRect(cr.left() + LineNumberAreaWidth(), cr.top(), WidgetAreaWidth(), cr.height()));
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
