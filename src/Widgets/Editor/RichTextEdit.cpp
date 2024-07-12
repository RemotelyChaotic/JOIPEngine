#include "RichTextEdit.h"

#include "Utils/UndoRedoFilter.h"
#include "Widgets/FlowLayout.h"

#include <QHBoxLayout>
#include <QLayout>
#include <QToolButton>

CRichTextEdit::CRichTextEdit(QWidget* pParent)
  : MRichTextEdit{pParent}
{
  // undo redo filter to make it work with the central undo redo system
  auto pFilter =
      new CUndoRedoFilter(textEdit(),
                          [this]() -> QMenu* {
                            return textEdit()->createStandardContextMenu();
                          });

  QToolButton* pBtnUndo = findChild<QToolButton*>("f_undo");
  QToolButton* pBtnRedo = findChild<QToolButton*>("f_redo");
  if (nullptr != pBtnUndo && nullptr != pBtnRedo)
  {
    pBtnUndo->disconnect();
    pBtnRedo->disconnect();
    connect(pBtnUndo, &QToolButton::clicked, this, &CRichTextEdit::UndoTriggered);
    connect(pBtnRedo, &QToolButton::clicked, this, &CRichTextEdit::RedoTriggered);
    pBtnUndo->hide();
    pBtnRedo->hide();
  }

  connect(pFilter, &CUndoRedoFilter::UndoTriggered, this, &CRichTextEdit::UndoTriggered);
  connect(pFilter, &CUndoRedoFilter::RedoTriggered, this, &CRichTextEdit::RedoTriggered);

  connect(textEdit(), &QTextEdit::textChanged, this, &CRichTextEdit::textChanged);

  // hide unneeded buttons
  QToolButton* pPtrCut = findChild<QToolButton*>("f_cut");
  QToolButton* pPtrCopy = findChild<QToolButton*>("f_copy");
  QToolButton* pPtrPaste = findChild<QToolButton*>("f_paste");
  if (nullptr != pPtrCut && nullptr != pPtrCopy && nullptr != pPtrPaste)
  {
    pPtrCut->hide();
    pPtrCopy->hide();
    pPtrPaste->hide();
  }

  // relayout toolbar into flow layout to make it work on mobile and with little space
  QWidget* pToolBar = findChild<QWidget*>("f_toolbar");
  if (nullptr != pToolBar && nullptr != pToolBar->layout())
  {
    CFlowLayout* pFlow = new CFlowLayout();
    QLayout* pLayout = pToolBar->layout();

    QWidget* pCurrentWidget = new QWidget(pToolBar);
    QHBoxLayout* pHLayout = new QHBoxLayout(pCurrentWidget);
    pHLayout->setContentsMargins({0,0,0,0});
    pHLayout->setMargin(0);
    pCurrentWidget->setLayout(pHLayout);

    while (pLayout->count() > 0)
    {
      QLayoutItem* pItem = pLayout->takeAt(0);
      if (nullptr != pItem)
      {
        QWidget* pWidget = pItem->widget();
        if (nullptr != pWidget &&
            !(dynamic_cast<QFrame*>(pWidget) && pWidget->objectName().startsWith("line")))
        {
          // hidden widgets are kept floating without layout
          if (!pWidget->isHidden())
          {
            pHLayout->addWidget(pWidget);
          }
        }
        else
        {
          pFlow->addWidget(pCurrentWidget);
          pCurrentWidget = new QWidget(pToolBar);
          pHLayout = new QHBoxLayout(pCurrentWidget);
          pHLayout->setContentsMargins({0,0,0,0});
          pHLayout->setMargin(0);
          pCurrentWidget->setLayout(pHLayout);
          if (nullptr != pWidget)
          {
            pFlow->addWidget(pWidget);
          }
        }
        delete pItem;
      }
    }
    pFlow->addWidget(pCurrentWidget);

    delete pLayout;
    pToolBar->setLayout(pFlow);
  }
}

CRichTextEdit::~CRichTextEdit() = default;
