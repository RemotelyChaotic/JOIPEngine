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
  setAttribute(Qt::WA_NoMousePropagation, false);

  m_pLineNumberArea = new CLineNumberArea(this);
  m_pWidgetArea = new CWidgetArea(this);

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
void CScriptEditorWidget::LineNumberAreaPaintEvent(QPaintEvent* pEvent)
{
  QPainter painter(m_pLineNumberArea);
  painter.fillRect(pEvent->rect(), QColor(24, 24, 24));

  QTextBlock block = firstVisibleBlock();
  qint32 iBlockNumber = block.blockNumber();
  qint32 iTop = static_cast<qint32>(blockBoundingGeometry(block).translated(contentOffset()).top());
  qint32 iBottom = iTop + static_cast<qint32>(blockBoundingRect(block).height());

  while (block.isValid() && iTop <= pEvent->rect().bottom())
  {
    if (block.isVisible() && iBottom >= pEvent->rect().top())
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
  m_pWidgetArea->ClearAllErrors();
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorWidget::WidgetAreaPaintEvent(QPaintEvent* pEvent)
{
  m_pWidgetArea->HideAllErrors();

  QPainter painter(m_pWidgetArea);
  painter.fillRect(pEvent->rect(), QColor(24, 24, 24));

  QTextBlock block = firstVisibleBlock();
  qint32 iBlockNumber = block.blockNumber();
  qint32 iTop = static_cast<qint32>(blockBoundingGeometry(block).translated(contentOffset()).top());
  qint32 iBottom = iTop + static_cast<qint32>(blockBoundingRect(block).height());

  while (block.isValid() && iTop <= pEvent->rect().bottom())
  {
    if (block.isVisible() && iBottom >= pEvent->rect().top())
    {
      QPointer<QLabel> pWidget = m_pWidgetArea->Widget(iBlockNumber);
      if (nullptr != pWidget)
      {
        pWidget->setGeometry(0, iTop, pWidget->width(), pWidget->height());
        pWidget->setVisible(true);
        pWidget->raise();
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
  pLabel->setAttribute(Qt::WA_TranslucentBackground);
  pLabel->setVisible(false);
  pLabel->setFixedSize(m_pCodeEditor->WidgetAreaWidth(),m_pCodeEditor->WidgetAreaWidth());
  pLabel->setScaledContents(true);
  pLabel->setPixmap(QPixmap("://resources/img/ButtonIcon.png"));
  pLabel->setToolTip(sException + ": " + sStack);
  pLabel->installEventFilter(this);
  m_errorLabelMap.insert({ iLine, pLabel });
}

//----------------------------------------------------------------------------------------
//
void CWidgetArea::ClearAllErrors()
{
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
