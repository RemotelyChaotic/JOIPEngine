#include "ScripEditorAddonWidgets.h"
#include "ScriptEditorWidget.h"

#include "Widgets/Editor/EditorSearchBar.h"
#include "Widgets/Editor/EditorHighlighter.h"

#include <QPainter>
#include <QStyle>
#include <QToolTip>

//----------------------------------------------------------------------------------------
//
CLineNumberArea::CLineNumberArea(CScriptEditorWidget* pEditor) :
  QWidget(pEditor)
{
  m_pCodeEditor = pEditor;
}

CLineNumberArea::~CLineNumberArea() = default;

//----------------------------------------------------------------------------------------
//
QSize CLineNumberArea::sizeHint() const
{
  return QSize(AreaWidth(), AreaHeight());
}

//----------------------------------------------------------------------------------------
//
qint32 CLineNumberArea::AreaHeight() const
{
  return m_pCodeEditor->height();
}

//----------------------------------------------------------------------------------------
//
qint32 CLineNumberArea::AreaWidth() const
{
  qint32 iDigits = 1;
  qint32 iMax = qMax(1, m_pCodeEditor->blockCount());
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
void CLineNumberArea::Reset()
{
}

//----------------------------------------------------------------------------------------
//
void CLineNumberArea::Update(const QRect& rect, qint32 iDy)
{
  if (0 != iDy)
  {
    scroll(0, iDy);
  }
  else
  {
    update(0, rect.y(), width(), rect.height());
  }

  if (rect.contains(m_pCodeEditor->viewport()->rect()))
  {
    m_pCodeEditor->UpdateArea(CScriptEditorWidget::eLeft, 0);
  }
}

//----------------------------------------------------------------------------------------
//
void CLineNumberArea::paintEvent(QPaintEvent* pEvent)
{
  QPainter painter(this);
  QColor lineNumberBackgroundColor = m_pCodeEditor->LineNumberBackgroundColor();
  QColor lineNumberTextColor = m_pCodeEditor->LineNumberTextColor();

  painter.fillRect(pEvent->rect(), lineNumberBackgroundColor);

  QTextBlock block = m_pCodeEditor->firstVisibleBlock();
  qint32 iBlockNumber = block.blockNumber();
  qint32 iTop = static_cast<qint32>(m_pCodeEditor->blockBoundingGeometry(block)
                                        .translated(m_pCodeEditor->contentOffset()).top());
  qint32 iBottom = iTop + static_cast<qint32>(m_pCodeEditor->blockBoundingRect(block).height());

  while (block.isValid() && iTop <= pEvent->rect().bottom())
  {
    if (block.isVisible() && iBottom >= pEvent->rect().top())
    {
      QString number = QString::number(iBlockNumber + 1);
      painter.setPen(lineNumberTextColor);
      painter.drawText(0, iTop, AreaWidth(), fontMetrics().height(),
                       Qt::AlignRight, number);
    }

    block = block.next();
    iTop = iBottom;
    iBottom = iTop + static_cast<qint32>(m_pCodeEditor->blockBoundingRect(block).height());
    ++iBlockNumber;
  }
}

//----------------------------------------------------------------------------------------
//
CWidgetArea::CWidgetArea(CScriptEditorWidget* pEditor) :
  QWidget(pEditor)
{
  m_pCodeEditor = pEditor;
}
CWidgetArea::~CWidgetArea() = default;

//----------------------------------------------------------------------------------------
//
QSize CWidgetArea::sizeHint() const
{
  return QSize(AreaWidth(), AreaHeight());
}

//----------------------------------------------------------------------------------------
//
qint32 CWidgetArea::AreaHeight() const
{
  return m_pCodeEditor->height();
}

//----------------------------------------------------------------------------------------
//
qint32 CWidgetArea::AreaWidth() const
{
  return 16;
}

//----------------------------------------------------------------------------------------
//
void CWidgetArea::Reset()
{
  HideAllErrors();
  ClearAllErrors();
  repaint();
}

//----------------------------------------------------------------------------------------
//
void CWidgetArea::Update(const QRect& rect, qint32 iDy)
{
  if (0 != iDy)
  {
    scroll(0, iDy);
  }
  else
  {
    update(0, rect.y(), width(), rect.height());
  }

  if (rect.contains(m_pCodeEditor->viewport()->rect()))
  {
    m_pCodeEditor->UpdateArea(CScriptEditorWidget::eLeft, 0);
  }
}

//----------------------------------------------------------------------------------------
//
void CWidgetArea::AddError(QString sException, qint32 iLine, QString sStack)
{
  QLabel* pLabel = new QLabel(this);
  pLabel->setVisible(true);
  pLabel->setFixedSize(AreaWidth(), AreaWidth());
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
void CWidgetArea::paintEvent(QPaintEvent* pEvent)
{
  QPainter painter(this);
  QColor widgetsBackgroundColor = m_pCodeEditor->WidgetsBackgroundColor();

  painter.fillRect(pEvent->rect(), widgetsBackgroundColor);

  QTextBlock block = m_pCodeEditor->firstVisibleBlock();
  qint32 iBlockNumber = block.blockNumber();
  qint32 iTop = static_cast<qint32>(m_pCodeEditor->blockBoundingGeometry(block)
                                        .translated(m_pCodeEditor->contentOffset()).top());
  const QRectF blockRect = m_pCodeEditor->blockBoundingRect(block);
  qint32 iBlockHeight = static_cast<qint32>(blockRect.height());
  qint32 iBottom = iTop + iBlockHeight;

  while (block.isValid() && iTop <= pEvent->rect().bottom())
  {
    if (block.isVisible() && iBottom >= pEvent->rect().top())
    {
      QPointer<QLabel> pWidget = Widget(iBlockNumber);
      if (nullptr != pWidget)
      {
        pWidget->setGeometry(0, iTop, pWidget->width(), pWidget->height());
        QPixmap pixmap = pWidget->grab();
        painter.drawPixmap(pWidget->geometry(), pixmap, pixmap.rect());
      }
    }

    block = block.next();
    iTop = iBottom;
    iBottom = iTop + static_cast<qint32>(m_pCodeEditor->blockBoundingRect(block).height());
    ++iBlockNumber;
  }
}

//----------------------------------------------------------------------------------------
//
CFoldBlockArea::CFoldBlockArea(CScriptEditorWidget* pEditor) :
    QWidget(pEditor)
{
  m_pCodeEditor = pEditor;
  setMouseTracking(true);

  connect(m_pCodeEditor, &CScriptEditorWidget::cursorPositionChanged,
          this, &CFoldBlockArea::CursorPositionChanged);
}
CFoldBlockArea::~CFoldBlockArea() = default;


//----------------------------------------------------------------------------------------
//
QSize CFoldBlockArea::sizeHint() const
{
  return QSize(AreaWidth(), AreaHeight());
}

//----------------------------------------------------------------------------------------
//
qint32 CFoldBlockArea::AreaHeight() const
{
  return m_pCodeEditor->height();
}

//----------------------------------------------------------------------------------------
//
qint32 CFoldBlockArea::AreaWidth() const
{
  return 16;
}

//----------------------------------------------------------------------------------------
//
void CFoldBlockArea::Reset()
{
}

//----------------------------------------------------------------------------------------
//
void CFoldBlockArea::Update(const QRect& rect, qint32 iDy)
{
  if (0 != iDy)
  {
    scroll(0, iDy);
  }
  else
  {
    update(0, 0, width(), height());
  }

  if (rect.contains(m_pCodeEditor->viewport()->rect()))
  {
    m_pCodeEditor->UpdateArea(CScriptEditorWidget::eLeft, 0);
  }
}

//----------------------------------------------------------------------------------------
//
void CFoldBlockArea::mousePressEvent(QMouseEvent* pEvt)
{
  QTextBlock block = m_pCodeEditor->firstVisibleBlock();
  qint32 iBlockNumber = block.blockNumber();
  qint32 iTop = static_cast<qint32>(
      m_pCodeEditor->blockBoundingGeometry(block)
          .translated(m_pCodeEditor->contentOffset()).top());
  qint32 iBottom = iTop + static_cast<qint32>(m_pCodeEditor->blockBoundingRect(block).height());

  while (block.isValid() && iTop <= rect().bottom())
  {
    if (iBottom >= rect().top() &&
        m_pCodeEditor->Highlighter()->startsFoldingRegion(block))
    {
      QTextBlock blockEnd = m_pCodeEditor->Highlighter()->findFoldingRegionEnd(block);
      bool bToggleFolding = false;
      if (block.isVisible())
      {
        QRectF iconBox = QRectF(0, iTop, AreaWidth(), fontMetrics().height());
        if (iconBox.contains(pEvt->localPos()))
        {
          bToggleFolding = true;
        }
      }
      else
      {
        qint32 iEndTop =
            static_cast<qint32>(m_pCodeEditor->blockBoundingGeometry(blockEnd)
                                                 .translated(m_pCodeEditor->contentOffset()).top());
        QRectF iconBox = QRectF(0, iEndTop, AreaWidth(), fontMetrics().height());

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

        m_pCodeEditor->SlotUpdateAllAddons(
            QRect(0, m_pCodeEditor->viewport()->rect().y(),
                   m_pCodeEditor->width(), m_pCodeEditor->viewport()->rect().height()), 0);
        update();
      }
    }

    block = block.next();
    iTop = iBottom;
    iBottom = iTop + static_cast<qint32>(m_pCodeEditor->blockBoundingRect(block).height());
    ++iBlockNumber;
    Q_UNUSED(iBlockNumber)
  }
}

//----------------------------------------------------------------------------------------
//
void CFoldBlockArea::paintEvent(QPaintEvent* pEvent)
{
  QPainter painter(this);
  QColor foldAreaBackgroundColor = m_pCodeEditor->FoldAreaBackgroundColor();
  QColor lineNumberBackgroundColor = m_pCodeEditor->LineNumberBackgroundColor();

  painter.fillRect(pEvent->rect(), foldAreaBackgroundColor);

  bool bNeedsRepaintOfContent = false;
  if (!m_pCodeEditor->m_foldSelection.isNull())
  {
    m_pCodeEditor->m_foldSelection = QRect();
    bNeedsRepaintOfContent = true;
  }

  double dLuminance = (0.299 * foldAreaBackgroundColor.red() +
                       0.587 * foldAreaBackgroundColor.green() +
                       0.114 * foldAreaBackgroundColor.blue()) / 255;
  QColor selection = dLuminance > 0.5 ? lineNumberBackgroundColor.darker() :
                         lineNumberBackgroundColor.lighter();

  QTextBlock cursorBlock = m_pCodeEditor->textCursor().block();
  QTextBlock block = m_pCodeEditor->firstVisibleBlock();
  QTextBlock blockStartFolding = block;
  qint32 iBlockNumber = block.blockNumber();
  qint32 iTop = static_cast<qint32>(m_pCodeEditor->blockBoundingGeometry(block)
                                        .translated(m_pCodeEditor->contentOffset()).top());
  qint32 iBottom = iTop + static_cast<qint32>(m_pCodeEditor->blockBoundingRect(block).height());
  qint32 iTextHeight = fontMetrics().height();

  while (block.isValid() && iTop <= pEvent->rect().bottom())
  {
    if (iBottom >= pEvent->rect().top())
    {
      bool bStartsFoldingRegion = false;
      if (m_pCodeEditor->Highlighter()->startsFoldingRegion(block))
      {
        blockStartFolding = block;
        bStartsFoldingRegion = true;
      }

      /*
      if (block == cursorBlock)
      {
        QTextBlock blockEnd = m_pCodeEditor->Highlighter()->findFoldingRegionEnd(blockStartFolding);
        qint32 iFoldingBlockEnd =
            static_cast<qint32>(m_pCodeEditor->blockBoundingGeometry(blockEnd)
                                    .translated(m_pCodeEditor->contentOffset()).top()) +
                                iTextHeight;
        qint32 iFoldingBlockStart =
            static_cast<qint32>(m_pCodeEditor->blockBoundingGeometry(blockStartFolding)
                                    .translated(m_pCodeEditor->contentOffset()).top()) +
            iTextHeight;
        QRect foldingBlockRect = QRect(0, iFoldingBlockStart, AreaWidth(), iTextHeight);
        foldingBlockRect.setHeight(iFoldingBlockEnd-iFoldingBlockStart);
        painter.fillRect(foldingBlockRect, selection);
      }
      */

      if (bStartsFoldingRegion)
      {
        if (block.isVisible())
        {
          QPoint globalCursorPos = QCursor::pos();
          QRect iconBox = QRect(0, iTop, AreaWidth(), iTextHeight);
          QPoint localPos = mapFromGlobal(globalCursorPos);
          if (iconBox.contains(localPos))
          {
            QTextBlock blockEnd = m_pCodeEditor->Highlighter()->findFoldingRegionEnd(blockStartFolding);
            qint32 iFoldingBlockEnd = 0;
            iFoldingBlockEnd =
                static_cast<qint32>(m_pCodeEditor->blockBoundingGeometry(blockEnd)
                                                       .translated(m_pCodeEditor->contentOffset()).top()) +
                fontMetrics().height();
            QRect foldingBlockRect = QRect(iconBox);
            foldingBlockRect.setHeight(iFoldingBlockEnd-iTop);
            painter.fillRect(foldingBlockRect, selection);
            m_pCodeEditor->m_foldSelection = foldingBlockRect;
            bNeedsRepaintOfContent = true;
          }
          m_pCodeEditor->IconUnfolded().paint(&painter, iconBox);
        }
        else
        {
          m_pCodeEditor->IconFolded().paint(&painter,
                                            QRect(0, iTop, AreaWidth(), AreaWidth()));
        }
      }
    }

    block = block.next();
    iTop = iBottom;
    iBottom = iTop + static_cast<qint32>(m_pCodeEditor->blockBoundingRect(block).height());
    ++iBlockNumber;
    Q_UNUSED(iBlockNumber)
  }

  if (bNeedsRepaintOfContent)
  {
    m_pCodeEditor->update();
  }
}

//----------------------------------------------------------------------------------------
//
void CFoldBlockArea::CursorPositionChanged()
{
  repaint();
}
