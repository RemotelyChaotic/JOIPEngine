#include "ScriptEditorAddonWidgets.h"
#include "ScriptEditorWidget.h"
#include "ScriptEditorWidgetPrivate.h"
#include "ui_ScriptFooterArea.h"

#include "EditorCustomBlockUserData.h"
#include "EditorHighlighter.h"
#include "TextEditZoomEnabler.h"

#include <QLineEdit>
#include <QPainter>
#include <QScrollBar>
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
  return m_pCodeEditor->contentsRect().height();
}

//----------------------------------------------------------------------------------------
//
qint32 CLineNumberArea::AreaWidth() const
{
  qint32 iDigits = 1;
  qint32 iMax = qMax(1, m_pCodeEditor->TextEdit()->blockCount());
  while (iMax >= 10)
  {
    iMax /= 10;
    ++iDigits;
  }

#if QT_VERSION_CHECK(6, 0, 0) <= QT_VERSION
  qint32 iSpace = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')).width() * iDigits;
#else
  qint32 iSpace = 3 + fontMetrics().boundingRect(QLatin1Char('9')).width() * iDigits;
#endif
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

  if (rect.contains(m_pCodeEditor->TextEdit()->viewport()->rect()))
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

  QTextBlock block = m_pCodeEditor->TextEdit()->firstVisibleBlock();
  qint32 iBlockNumber = block.blockNumber();
  qint32 iTop = static_cast<qint32>(m_pCodeEditor->TextEdit()->blockBoundingGeometry(block)
                                        .translated(m_pCodeEditor->TextEdit()->contentOffset()).top());
  qint32 iBottom = iTop + static_cast<qint32>(m_pCodeEditor->TextEdit()->blockBoundingRect(block).height());

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
    iBottom = iTop + static_cast<qint32>(m_pCodeEditor->TextEdit()->blockBoundingRect(block).height());
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
  return m_pCodeEditor->contentsRect().height();
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

  if (rect.contains(m_pCodeEditor->TextEdit()->viewport()->rect()))
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
  emit SignalAddedError();
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
  emit SignalClearErrors();
}

//----------------------------------------------------------------------------------------
//
void CWidgetArea::ForEach(std::function<void(qint32, QPointer<QLabel>)> fn)
{
  for (const auto& [iLine, pLabel] : m_errorLabelMap)
  {
    if (nullptr != fn) { fn(iLine, pLabel); }
  }
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

  QTextBlock block = m_pCodeEditor->TextEdit()->firstVisibleBlock();
  qint32 iBlockNumber = block.blockNumber();
  qint32 iTop = static_cast<qint32>(m_pCodeEditor->TextEdit()->blockBoundingGeometry(block)
                                        .translated(m_pCodeEditor->TextEdit()->contentOffset()).top());
  const QRectF blockRect = m_pCodeEditor->TextEdit()->blockBoundingRect(block);
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
    iBottom = iTop + static_cast<qint32>(m_pCodeEditor->TextEdit()->blockBoundingRect(block).height());
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

  connect(m_pCodeEditor->TextEdit(), &QPlainTextEdit::cursorPositionChanged,
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
  return m_pCodeEditor->contentsRect().height();
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

  if (rect.contains(m_pCodeEditor->TextEdit()->viewport()->rect()))
  {
    m_pCodeEditor->UpdateArea(CScriptEditorWidget::eLeft, 0);
  }
}

//----------------------------------------------------------------------------------------
//
void CFoldBlockArea::leaveEvent(QEvent*)
{
  m_pCodeEditor->m_foldSelection = QRect();
  m_pCodeEditor->TextEdit()->viewport()->update();
  m_pCodeEditor->TextEdit()->repaint();
}

//----------------------------------------------------------------------------------------
//
void CFoldBlockArea::mousePressEvent(QMouseEvent* pEvt)
{
  QTextBlock block = m_pCodeEditor->TextEdit()->firstVisibleBlock();
  qint32 iBlockNumber = block.blockNumber();
  qint32 iTop = static_cast<qint32>(
      m_pCodeEditor->TextEdit()->blockBoundingGeometry(block)
          .translated(m_pCodeEditor->TextEdit()->contentOffset()).top());
  qint32 iBottom = iTop + static_cast<qint32>(m_pCodeEditor->TextEdit()->blockBoundingRect(block).height());

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
              pUserData = new CCustomBlockUserData(block.userData());
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
            QRect(0, m_pCodeEditor->TextEdit()->viewport()->rect().y(),
                   m_pCodeEditor->width(), m_pCodeEditor->TextEdit()->viewport()->rect().height()), 0);
        m_pCodeEditor->TextEdit()->viewport()->update();
        m_pCodeEditor->TextEdit()->repaint();
        update();
      }
    }

    block = block.next();
    iTop = iBottom;
    iBottom = iTop + static_cast<qint32>(m_pCodeEditor->TextEdit()->blockBoundingRect(block).height());
    ++iBlockNumber;
    Q_UNUSED(iBlockNumber)
  }
}

//----------------------------------------------------------------------------------------
//
void CFoldBlockArea::mouseMoveEvent(QMouseEvent* pEvt)
{
  bool bNeedsRepaintOfContent = false;
  if (!m_pCodeEditor->m_foldSelection.isNull())
  {
    m_pCodeEditor->m_foldSelection = QRect();
    bNeedsRepaintOfContent = true;
  }

  qint32 iTextHeight = fontMetrics().height();

  QRect foldingBlockRect;
  qint32 iTop = 0;
  qint32 iBottom = 0;
  std::vector<QTextBlock> vAllVisibleBlocks;
  std::set<std::pair<QTextBlock, QTextBlock>> foldingSet;

  GetBlocks(rect(), vAllVisibleBlocks, foldingSet, iTop, iBottom,
            foldingBlockRect);

  // iterate over blocks and paint arrows
  for (const QTextBlock& currentBlock : vAllVisibleBlocks)
  {
    if (iBottom >= rect().top())
    {
      auto it =
          std::find_if(foldingSet.begin(), foldingSet.end(),
                       [&currentBlock](const std::pair<QTextBlock, QTextBlock>& pair)
                       {
                         return pair.first == currentBlock;
                       });
      if (foldingSet.end() != it)
      {
        if (currentBlock.isVisible())
        {
          if (currentBlock.next().isVisible())
          {
            QPoint globalCursorPos = pEvt->globalPos();
            QRect iconBox = QRect(0, iTop, AreaWidth(), iTextHeight);
            QPoint localPos = mapFromGlobal(globalCursorPos);
            if (iconBox.contains(localPos))
            {
              QTextBlock blockEnd = it->second;
              qint32 iFoldingBlockEnd = 0;
              iFoldingBlockEnd =
                  static_cast<qint32>(
                      m_pCodeEditor->TextEdit()
                          ->blockBoundingGeometry(blockEnd)
                          .translated(m_pCodeEditor->TextEdit()->contentOffset())
                          .top()) +
                  fontMetrics().height();
              QRect foldingBlockRect = QRect(iconBox);
              foldingBlockRect.setHeight(iFoldingBlockEnd - iTop);
              m_pCodeEditor->m_foldSelection = foldingBlockRect;
              bNeedsRepaintOfContent = true;
            }
          }
        }
      }
    }

    iTop = iBottom;
    iBottom =
        iTop +
        static_cast<qint32>(
            m_pCodeEditor->TextEdit()->blockBoundingRect(currentBlock.next()).height());
  }

  if (bNeedsRepaintOfContent)
  {
    m_pCodeEditor->TextEdit()->viewport()->update();
    m_pCodeEditor->TextEdit()->repaint();
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

  double dLuminance = (0.299 * foldAreaBackgroundColor.red() +
                       0.587 * foldAreaBackgroundColor.green() +
                       0.114 * foldAreaBackgroundColor.blue()) / 255;

  QColor hoverColor = dLuminance > 0.5 ? lineNumberBackgroundColor.darker() :
                          lineNumberBackgroundColor.lighter();
  QColor selectionColor = dLuminance > 0.5 ? hoverColor.darker() :
                              hoverColor.lighter();
  qint32 iTextHeight = fontMetrics().height();

  QRect foldingBlockRect;
  qint32 iTop = 0;
  qint32 iBottom = 0;
  std::vector<QTextBlock> vAllVisibleBlocks;
  std::set<std::pair<QTextBlock, QTextBlock>> foldingSet;

  GetBlocks(pEvent->rect(), vAllVisibleBlocks, foldingSet, iTop, iBottom,
            foldingBlockRect);


  if (!foldingBlockRect.isNull())
  {
    painter.fillRect(foldingBlockRect, hoverColor);
  }

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
          QRect iconBox = QRect(0, iTop, AreaWidth(), iTextHeight);
          if (currentBlock.next().isVisible())
          {
            m_pCodeEditor->IconUnfolded().paint(&painter, iconBox);
          }
          else
          {
            m_pCodeEditor->IconFolded().paint(&painter, iconBox);
          }
        }
      }
    }

    iTop = iBottom;
    iBottom = iTop + static_cast<qint32>(m_pCodeEditor->TextEdit()->blockBoundingRect(currentBlock.next()).height());
  }
}

//----------------------------------------------------------------------------------------
//
void CFoldBlockArea::CursorPositionChanged()
{
  repaint();
}

//----------------------------------------------------------------------------------------
//
void CFoldBlockArea::GetBlocks(const QRect& rect,
                               std::vector<QTextBlock>& vAllVisibleBlocks,
                               std::set<std::pair<QTextBlock, QTextBlock>>& foldingSet,
                               qint32& iTop, qint32& iBottom, QRect& foldingBlockRect)
{
  std::stack<QTextBlock> startFoldingStack;
  std::stack<QTextBlock> endFoldingStack;

  QTextBlock cursorBlock = m_pCodeEditor->TextEdit()->textCursor().block();
  QTextBlock block = m_pCodeEditor->TextEdit()->firstVisibleBlock();
  iTop =
      static_cast<qint32>(m_pCodeEditor->TextEdit()
                             ->blockBoundingGeometry(block)
                             .translated(m_pCodeEditor->TextEdit()->contentOffset())
                             .top());
  iBottom = iTop + static_cast<qint32>(
                m_pCodeEditor->TextEdit()->blockBoundingRect(block).height());
  qint32 iTextHeight = fontMetrics().height();

  // first find all visible blocks
  while (block.isValid() && iTop <= rect.bottom())
  {
    vAllVisibleBlocks.push_back(block);
    block = block.next();
    iTop = iBottom;
    iBottom = iTop + static_cast<qint32>(
                  m_pCodeEditor->TextEdit()->blockBoundingRect(block).height());
  }

  if (vAllVisibleBlocks.empty()) { return; }

  // set start values
  block = vAllVisibleBlocks.front();
  iTop = static_cast<qint32>(m_pCodeEditor->TextEdit()
                                ->blockBoundingGeometry(block)
                                .translated(m_pCodeEditor->TextEdit()->contentOffset())
                                .top());
  iBottom = iTop + static_cast<qint32>(
                m_pCodeEditor->TextEdit()->blockBoundingRect(block).height());

  startFoldingStack.push(block);
  endFoldingStack.push(vAllVisibleBlocks.back());

  // iterate over blocks and find current block
  for (const QTextBlock& currentBlock : vAllVisibleBlocks)
  {
    if (iBottom >= rect.top())
    {
      if (m_pCodeEditor->Highlighter()->startsFoldingRegion(currentBlock))
      {
        QTextBlock blockStartFolding = currentBlock;
        QTextBlock endBlockFolding =
            m_pCodeEditor->Highlighter()->findFoldingRegionEnd(blockStartFolding);
        foldingSet.insert({ currentBlock, endBlockFolding });
        startFoldingStack.push(currentBlock);
        endFoldingStack.push(endBlockFolding);
      }

      if (currentBlock == cursorBlock)
      {
        QTextBlock blockEnd = endFoldingStack.top();
        qint32 iFoldingBlockEnd =
            static_cast<qint32>(m_pCodeEditor->TextEdit()
                                   ->blockBoundingGeometry(blockEnd)
                                   .translated(m_pCodeEditor->TextEdit()->contentOffset())
                                   .top()) + iTextHeight;
        qint32 iFoldingBlockStart =
            static_cast<qint32>(m_pCodeEditor->TextEdit()
                                   ->blockBoundingGeometry(startFoldingStack.top())
                                   .translated(m_pCodeEditor->TextEdit()->contentOffset())
                                   .top());
        foldingBlockRect = QRect(0, iFoldingBlockStart, AreaWidth(), iTextHeight);
        foldingBlockRect.setHeight(iFoldingBlockEnd - iFoldingBlockStart);
      }

      if (endFoldingStack.top() == currentBlock)
      {
        startFoldingStack.pop();
        endFoldingStack.pop();
      }
    }
    iTop = iBottom;
    iBottom =
        iTop +
        static_cast<qint32>(
            m_pCodeEditor->TextEdit()->blockBoundingRect(currentBlock.next()).height());
  }

  block = vAllVisibleBlocks.front();
  iTop = static_cast<qint32>(m_pCodeEditor->TextEdit()
                                ->blockBoundingGeometry(block)
                                .translated(m_pCodeEditor->TextEdit()->contentOffset())
                                .top());
  iBottom = iTop + static_cast<qint32>(
                m_pCodeEditor->TextEdit()->blockBoundingRect(block).height());
}

//----------------------------------------------------------------------------------------
//
CFooterArea::CFooterArea(CScriptEditorWidget* pEditor, QScrollBar* pBottomScrollBar, CWidgetArea* pWidgetArea) :
  QWidget(pEditor),
  m_spUi(std::make_unique<Ui::CScriptFooterArea>())
{
  m_spUi->setupUi(this);
  m_pListView = new QListWidget(pEditor);
  m_pListView->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
  m_pListView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_pListView->hide();
  m_pCodeEditor = pEditor;
  m_pWidgetArea = pWidgetArea;

  QLayout* pLayoutScroll = m_spUi->pScrollAreaContainer->layout();
  if (nullptr != pLayoutScroll)
  {
    pBottomScrollBar->setParent(m_spUi->pScrollAreaContainer);
    pLayoutScroll->addWidget(pBottomScrollBar);
    pEditor->TextEdit()->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    pBottomScrollBar->show();
  }

  StyleChanged();

  m_spUi->ErrorPushButton->setProperty("styleSmall", true);
  m_spUi->ErrorPushButton->setText(QString::number(0));
  m_spUi->ErrorPushButton->setProperty("value", 0);

  m_spUi->WhitespacePushButton->setProperty("styleSmall", true);
  QHBoxLayout* pLayout = new QHBoxLayout(m_spUi->WhitespacePushButton);
  pLayout->setContentsMargins({0, 0, 0, 0});
  m_pWsButtonLabel = new QLabel("WS", m_spUi->WhitespacePushButton);
  m_pWsButtonLabel->setAttribute(Qt::WA_TranslucentBackground);
  m_pWsButtonLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
  m_pWsButtonLabel->setAlignment(Qt::AlignCenter);
  pLayout->addWidget(m_pWsButtonLabel);
  UpdateWhitespaceText(true);
  connect(m_spUi->WhitespacePushButton, &QPushButton::clicked,
          this, &CFooterArea::WhiteSpaceButtonPressed);

  ZoomChanged(m_pCodeEditor->ZoomEnabler()->Zoom());

  connect(m_pCodeEditor->ZoomEnabler(), &CTextEditZoomEnabler::SignalZoomChanged,
          this, &CFooterArea::ZoomChanged);
  connect(m_spUi->pZoomComboBox, &CZoomComboBox::SignalZoomChanged,
          this, &CFooterArea::ZoomValueChanged);
  connect(m_pWidgetArea, &CWidgetArea::SignalAddedError,
          this, &CFooterArea::ErrorAdded);
  connect(m_pWidgetArea, &CWidgetArea::SignalClearErrors,
          this, &CFooterArea::ClearAllErors);
  connect(m_spUi->ErrorPushButton, &QPushButton::clicked,
          this, &CFooterArea::ToggleErrorList);
  connect(m_pCodeEditor->TextEdit().data(), &QPlainTextEdit::cursorPositionChanged,
          this, &CFooterArea::CursorPositionChanged);
}
CFooterArea::~CFooterArea()
{
  delete m_pListView;
}

//----------------------------------------------------------------------------------------
//
QSize CFooterArea::sizeHint() const
{
  return QSize(AreaWidth(), AreaHeight());
}

//----------------------------------------------------------------------------------------
//
qint32 CFooterArea::AreaHeight() const
{
  return 18;
}

//----------------------------------------------------------------------------------------
//
qint32 CFooterArea::AreaWidth() const
{
  return m_pCodeEditor->contentsRect().width();
}

//----------------------------------------------------------------------------------------
//
void CFooterArea::Reset()
{
}

//----------------------------------------------------------------------------------------
//
void CFooterArea::Update(const QRect& rect, qint32 iDy)
{
  Q_UNUSED(rect)
  Q_UNUSED(iDy)
  update(0, 0, width(), height());
  m_pCodeEditor->UpdateArea(CScriptEditorWidget::eBottom, 0);
}

//----------------------------------------------------------------------------------------
//
void CFooterArea::paintEvent(QPaintEvent* pEvent)
{
  QPainter painter(this);
  QColor widgetsBackgroundColor = m_pCodeEditor->WidgetsBackgroundColor();
  painter.fillRect(pEvent->rect(), widgetsBackgroundColor);
  QWidget::paintEvent(pEvent);
}

//----------------------------------------------------------------------------------------
//
void CFooterArea::CursorPositionChanged()
{
  QTextCursor cursor = m_pCodeEditor->TextEdit()->textCursor();
  m_spUi->pCursorLabel->setText(QString("Ln:%1 Ch:%2")
                                    .arg(cursor.block().blockNumber()+1) // start at 1
                                    .arg(cursor.positionInBlock()));
}

//----------------------------------------------------------------------------------------
//
void CFooterArea::ClearAllErors()
{
  m_spUi->ErrorPushButton->setText(QString::number(0));
  m_spUi->ErrorPushButton->setProperty("value", 0);
  m_pListView->clear();
}

//----------------------------------------------------------------------------------------
//
void CFooterArea::ErrorAdded()
{
  qint32 iErrs = 0;
  qint32 iSize = 0;
  m_pWidgetArea->ForEach([&iErrs, &iSize, this](qint32 iLine, QPointer<QLabel> pLbl) mutable {
    Q_UNUSED(iLine)
    m_pListView->addItem(pLbl->toolTip());
    iSize += m_pListView->sizeHintForRow(iErrs);
    iErrs++;
  });
  m_spUi->ErrorPushButton->setText(QString::number(iErrs));
  m_spUi->ErrorPushButton->setProperty("value", iErrs);
  m_pListView->setFixedSize(m_pListView->sizeHintForColumn(0) + 10, std::min(iSize + 10, 400));
  m_pListView->updateGeometry();
}

//----------------------------------------------------------------------------------------
//
void CFooterArea::ToggleErrorList()
{
  if (0 < m_spUi->ErrorPushButton->property("value").toInt())
  {
    QSize size = m_pListView->size();
    QPoint p = mapTo(m_pCodeEditor, m_spUi->ErrorPushButton->pos());
    m_pListView->move(p.x(), p.y() - size.height());
    m_pListView->setVisible(!m_pListView->isVisible());
    m_pListView->raise();
  }
  else
  {
    m_pListView->hide();
  }
}

//----------------------------------------------------------------------------------------
//
void CFooterArea::StyleChanged()
{
  QPalette pal = m_spUi->ErrorPushButton->palette();
  pal.setColor(QPalette::Text, m_pCodeEditor->LineNumberTextColor());
  m_spUi->ErrorPushButton->setPalette(pal);

  pal = m_spUi->pCursorLabel->palette();
  pal.setColor(QPalette::Text, m_pCodeEditor->LineNumberTextColor());
  m_spUi->pCursorLabel->setPalette(pal);
}

//----------------------------------------------------------------------------------------
//
void CFooterArea::UpdateWhitespaceText(bool bWsEnabled)
{
  if (bWsEnabled)
  {
    m_pWsButtonLabel->setText("WS");
  }
  else
  {
    m_pWsButtonLabel->setText("<s>WS</s>");
  }
}

//----------------------------------------------------------------------------------------
//
void CFooterArea::WhiteSpaceButtonPressed()
{
  bool bNewValue = !m_pCodeEditor->IsShowWhitespaceEnabled();
  UpdateWhitespaceText(bNewValue);
  emit SignalShowWhiteSpaceChanged(bNewValue);
}

//----------------------------------------------------------------------------------------
//
void CFooterArea::ZoomChanged(qint32 iZoom)
{
  m_spUi->pZoomComboBox->UpdateZoomComboBox(iZoom);
}

//----------------------------------------------------------------------------------------
//
void CFooterArea::ZoomValueChanged(qint32 iZoom)
{
  m_pCodeEditor->ZoomEnabler()->SetZoom(iZoom);
}
