#include "ScriptEditorWidget.h"
#include <QtWidgets>
#include <QFontMetrics>

namespace
{
  const qint32 c_iTabStop = 2;  // 2 characters
}

CScriptEditorWidget::CScriptEditorWidget(QWidget* pParent) :
  QPlainTextEdit(pParent)
{
  m_pLineNumberArea = new CLineNumberArea(this);

  connect(this, &CScriptEditorWidget::blockCountChanged,
          this, &CScriptEditorWidget::UpdateLineNumberAreaWidth);
  connect(this, &CScriptEditorWidget::updateRequest,
          this, &CScriptEditorWidget::UpdateLineNumberArea);
  connect(this, &CScriptEditorWidget::cursorPositionChanged,
          this, &CScriptEditorWidget::HighlightCurrentLine);

  UpdateLineNumberAreaWidth(0);
  HighlightCurrentLine();

  QPalette pCode = palette();
  pCode.setColor(QPalette::Active, QPalette::Base, QColor(38, 38, 38));
  pCode.setColor(QPalette::Inactive, QPalette::Base, QColor(38, 38, 38));
  pCode.setColor(QPalette::Text, Qt::white);
  setPalette(pCode);

  QFont font;
  font.setFamily("Courier");
  font.setStyleHint(QFont::Monospace);
  font.setFixedPitch(true);
  font.setPointSize(10);
  setFont(font);

  QFontMetrics metrics(font);
  setTabStopWidth(c_iTabStop * metrics.width(' '));
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
void CScriptEditorWidget::UpdateLineNumberAreaWidth(qint32 iNewBlockCount)
{
  Q_UNUSED(iNewBlockCount)
  setViewportMargins(LineNumberAreaWidth(), 0, 0, 0);
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
    UpdateLineNumberAreaWidth(0);
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorWidget::resizeEvent(QResizeEvent* pEvent)
{
  QPlainTextEdit::resizeEvent(pEvent);

  QRect cr = contentsRect();
  m_pLineNumberArea->setGeometry(QRect(cr.left(), cr.top(), LineNumberAreaWidth(), cr.height()));
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorWidget::HighlightCurrentLine()
{
  QList<QTextEdit::ExtraSelection> vExtraSelections;

  if (!isReadOnly())
  {
    QTextEdit::ExtraSelection selection;

    QColor lineColor = QColor(68, 71, 90);

    selection.format.setBackground(lineColor);
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.cursor = textCursor();
    selection.cursor.clearSelection();
    vExtraSelections.append(selection);
  }

  setExtraSelections(vExtraSelections);
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorWidget::LineNumberAreaPaintEvent(QPaintEvent *event)
{
  QPainter painter(m_pLineNumberArea);
  painter.fillRect(event->rect(), QColor(24, 24, 24));

  QTextBlock block = firstVisibleBlock();
  qint32 iBlockNumber = block.blockNumber();
  qint32 iTop = static_cast<qint32>(blockBoundingGeometry(block).translated(contentOffset()).top());
  qint32 iBottom = iTop + static_cast<qint32>(blockBoundingRect(block).height());

  while (block.isValid() && iTop <= event->rect().bottom())
  {
    if (block.isVisible() && iBottom >= event->rect().top())
    {
      QString number = QString::number(iBlockNumber + 1);
      painter.setPen(Qt::white);
      painter.drawText(0, iTop, m_pLineNumberArea->width(), fontMetrics().height(),
                       Qt::AlignRight, number);
    }

    block = block.next();
    iTop = iBottom;
    iBottom = iTop + static_cast<qint32>(blockBoundingRect(block).height());
    ++iBlockNumber;
  }
}
