#include "ScriptEditorAddonWidgets.h"
#include "ScriptEditorWidget.h"

#include "Widgets/Editor/EditorSearchBar.h"
#include "Widgets/Editor/EditorHighlighter.h"

#include <QPainter>
#include <QStyle>
#include <QToolTip>

#include <stack>
#include <set>

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
      bool bSetFoldedContent = false;
      if (block.isVisible())
      {
        QRectF iconBox = QRectF(0, iTop, AreaWidth(), fontMetrics().height());
        if (iconBox.contains(pEvt->localPos()))
        {
          bToggleFolding = true;
          bSetFoldedContent = true;
        }
      }

      if (bToggleFolding)
      {
        bool bWasVisible = true;
        QStringList vsFoldedData;
        QTextBlock current = block;
        while (blockEnd != current.next())
        {
          current = current.next();
          vsFoldedData << current.text();
          bWasVisible &= current.isVisible();
          current.setVisible(!current.isVisible());
        }

        // set fold userdata
        if (bSetFoldedContent)
        {
          if (bWasVisible)
          {
            CCustomBlockUserData* pUserData;
            if (pUserData = dynamic_cast<CCustomBlockUserData*>(block.userData());
                nullptr == pUserData)
            {
              pUserData = new CCustomBlockUserData(
                  dynamic_cast<KSyntaxHighlighting::TextBlockUserData*>(block.userData()));
              block.setUserData(pUserData);
            }
            pUserData->SetFoldedContent(vsFoldedData.join("\n"));
          }
          else
          {
            // empty userData if found
            if (CCustomBlockUserData* pUserData =
                dynamic_cast<CCustomBlockUserData*>(block.userData());
                nullptr != pUserData)
            {
              pUserData->SetFoldedContent(QString());
            }
          }
        }

        m_pCodeEditor->SlotUpdateAllAddons(
            QRect(0, m_pCodeEditor->viewport()->rect().y(),
                   m_pCodeEditor->width(), m_pCodeEditor->viewport()->rect().height()), 0);
        m_pCodeEditor->repaint();
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

  QColor hoverColor = dLuminance > 0.5 ? lineNumberBackgroundColor.darker() :
                         lineNumberBackgroundColor.lighter();
  QColor selectionColor = dLuminance > 0.5 ? hoverColor.darker() :
                              hoverColor.lighter();

  QTextBlock cursorBlock = m_pCodeEditor->textCursor().block();
  QTextBlock block = m_pCodeEditor->firstVisibleBlock();
  qint32 iTop = static_cast<qint32>(m_pCodeEditor->blockBoundingGeometry(block)
                                        .translated(m_pCodeEditor->contentOffset()).top());
  qint32 iBottom = iTop + static_cast<qint32>(m_pCodeEditor->blockBoundingRect(block).height());
  qint32 iTextHeight = fontMetrics().height();

  std::set<std::pair<QTextBlock, QTextBlock>> foldingSet;
  std::stack<QTextBlock> startFoldingStack;
  std::stack<QTextBlock> endFoldingStack;
  std::vector<QTextBlock> vAllVisibleBlocks;

  // first find all visible blocks
  while (block.isValid() && iTop <= pEvent->rect().bottom())
  {
    vAllVisibleBlocks.push_back(block);
    block = block.next();
    iTop = iBottom;
    iBottom = iTop + static_cast<qint32>(m_pCodeEditor->blockBoundingRect(block).height());
  }

  // set start values
  block = vAllVisibleBlocks.front();
  iTop = static_cast<qint32>(m_pCodeEditor->blockBoundingGeometry(block)
                                        .translated(m_pCodeEditor->contentOffset()).top());
  iBottom = iTop + static_cast<qint32>(m_pCodeEditor->blockBoundingRect(block).height());

  startFoldingStack.push(block);
  endFoldingStack.push(vAllVisibleBlocks.back());

  // iterate over blocks and find current block
  for (const QTextBlock& currentBlock : vAllVisibleBlocks)
  {
    if (iBottom >= pEvent->rect().top())
    {
      if (m_pCodeEditor->Highlighter()->startsFoldingRegion(currentBlock))
      {
        QTextBlock blockStartFolding = currentBlock;
        QTextBlock endBlockFolding =
            m_pCodeEditor->Highlighter()->findFoldingRegionEnd(blockStartFolding);
        foldingSet.insert({currentBlock, endBlockFolding});
        startFoldingStack.push(currentBlock);
        endFoldingStack.push(endBlockFolding);
      }

      if (currentBlock == cursorBlock)
      {
        QTextBlock blockEnd = endFoldingStack.top();
        qint32 iFoldingBlockEnd =
            static_cast<qint32>(m_pCodeEditor->blockBoundingGeometry(blockEnd)
                                    .translated(m_pCodeEditor->contentOffset()).top()) +
                                iTextHeight;
        qint32 iFoldingBlockStart =
            static_cast<qint32>(m_pCodeEditor->blockBoundingGeometry(startFoldingStack.top())
                                    .translated(m_pCodeEditor->contentOffset()).top());
        QRect foldingBlockRect = QRect(0, iFoldingBlockStart, AreaWidth(), iTextHeight);
        foldingBlockRect.setHeight(iFoldingBlockEnd-iFoldingBlockStart);
        painter.fillRect(foldingBlockRect, hoverColor);
      }

      if (endFoldingStack.top() == currentBlock)
      {
        startFoldingStack.pop();
        endFoldingStack.pop();
      }
    }
    iTop = iBottom;
    iBottom = iTop + static_cast<qint32>(m_pCodeEditor->blockBoundingRect(currentBlock.next()).height());
  }

  block = vAllVisibleBlocks.front();
  iTop = static_cast<qint32>(m_pCodeEditor->blockBoundingGeometry(block)
                                 .translated(m_pCodeEditor->contentOffset()).top());
  iBottom = iTop + static_cast<qint32>(m_pCodeEditor->blockBoundingRect(block).height());

  // iterate over blocks and paint arrows
  for (const QTextBlock& currentBlock : vAllVisibleBlocks)
  {
    if (iBottom >= pEvent->rect().top())
    {
      auto it = std::find_if(foldingSet.begin(), foldingSet.end(),
                             [&currentBlock](const std::pair<QTextBlock, QTextBlock>& pair) {
        return pair.first == currentBlock;
      });
      if (foldingSet.end() != it)
      {
        if (currentBlock.isVisible())
        {
          if (currentBlock.next().isVisible())
          {
            QPoint globalCursorPos = QCursor::pos();
            QRect iconBox = QRect(0, iTop, AreaWidth(), iTextHeight);
            QPoint localPos = mapFromGlobal(globalCursorPos);
            if (iconBox.contains(localPos))
            {
              QTextBlock blockEnd = it->second;
              qint32 iFoldingBlockEnd = 0;
              iFoldingBlockEnd =
                  static_cast<qint32>(m_pCodeEditor->blockBoundingGeometry(blockEnd)
                                                         .translated(m_pCodeEditor->contentOffset()).top()) +
                  fontMetrics().height();
              QRect foldingBlockRect = QRect(iconBox);
              foldingBlockRect.setHeight(iFoldingBlockEnd-iTop);
              painter.fillRect(foldingBlockRect, selectionColor);
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
    }

    iTop = iBottom;
    iBottom = iTop + static_cast<qint32>(m_pCodeEditor->blockBoundingRect(currentBlock.next()).height());
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
