#include "EditorSearchBar.h"
#include "Widgets/SearchWidget.h"

#include <QAction>
#include <QHBoxLayout>
#include <QPushButton>

CEditorSearchBar::CEditorSearchBar(QWidget* pParent,
                                   ESearchDisplayElementFlags displayFlags) :
  COverlayBase(0, pParent),
  m_searchDir(eNone)
{
  QHBoxLayout* pLayout = new QHBoxLayout(this);
  m_pSearchWidget = new CSearchWidget(this);
  connect(m_pSearchWidget, &CSearchWidget::SignalFilterChanged,
          this, &CEditorSearchBar::SlotFilterChanged);
  pLayout->addWidget(m_pSearchWidget);

  if (displayFlags.testFlag(ESearchDisplayFlag::eBackButton))
  {
    m_pBack = new QPushButton(this);
    m_pBack->setObjectName("BackButton");
    m_pBack->setToolTip("Search backwards");
    connect(m_pBack, &QPushButton::clicked,
            this, &CEditorSearchBar::SlotBackClicked);
    pLayout->addWidget(m_pBack);
  }

  if (displayFlags.testFlag(ESearchDisplayFlag::eBackButton))
  {
    m_pForward = new QPushButton(this);
    m_pForward->setObjectName("ForardButton");
    m_pForward->setToolTip("Search forwards");
    connect(m_pForward, &QPushButton::clicked,
            this, &CEditorSearchBar::SlotForwardClicked);
    pLayout->addWidget(m_pForward);
  }

  if (displayFlags.testFlag(ESearchDisplayFlag::eCloseButton))
  {
    m_pCloseButton = new QPushButton(this);
    m_pCloseButton->setObjectName("CloseButton");
    m_pForward->setToolTip("Close search");
    connect(m_pCloseButton, &QPushButton::clicked, this, &CEditorSearchBar::Hide);
    pLayout->addWidget(m_pCloseButton);
  }

  pParent->metaObject()->className();

  QAction* pSearchAction = new QAction(tr("Search"), pParent);
  pSearchAction->setShortcut(QKeySequence::Find);
  pSearchAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
  connect(pSearchAction, &QAction::triggered,
          this, &CEditorSearchBar::SlotShowHideSearchFilter);
  pParent->addAction(pSearchAction);

  pSearchAction = new QAction(tr("Search backwards"), pParent);
  pSearchAction->setShortcut(QKeySequence::FindPrevious);
  pSearchAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
  connect(pSearchAction, &QAction::triggered,
          this, &CEditorSearchBar::SlotBackClicked);
  pParent->addAction(pSearchAction);

  pSearchAction = new QAction(tr("Search forwards"), pParent);
  pSearchAction->setShortcut(QKeySequence::FindNext);
  pSearchAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
  connect(pSearchAction, &QAction::triggered,
          this, &CEditorSearchBar::SlotForwardClicked);
  pParent->addAction(pSearchAction);

  pSearchAction = new QAction(tr("Close search"), pParent);
                  pSearchAction->setShortcut(Qt::Key_Escape);
  pSearchAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
  connect(pSearchAction, &QAction::triggered, this, &CEditorSearchBar::Hide);
  pParent->addAction(pSearchAction);
}

CEditorSearchBar::~CEditorSearchBar()
{

}

//----------------------------------------------------------------------------------------
//
void CEditorSearchBar::SetFilter(const QString& sString)
{
  m_pSearchWidget->blockSignals(true);
  m_pSearchWidget->SetFilter(sString);
  m_pSearchWidget->blockSignals(false);
}

//----------------------------------------------------------------------------------------
//
void CEditorSearchBar::Climb()
{
  ClimbToFirstInstanceOf(m_pOriginalParent->metaObject()->className(), false);
}

//----------------------------------------------------------------------------------------
//
void CEditorSearchBar::Hide()
{
  m_pSearchWidget->blockSignals(true);
  COverlayBase::Hide();
  m_pSearchWidget->blockSignals(false);
  emit SignalHidden();
}

//----------------------------------------------------------------------------------------
//
void CEditorSearchBar::Resize()
{
  if (nullptr != m_pTargetWidget)
  {
    QSize target = m_pTargetWidget->size();
    qint32 iXOffset = (target.width() - sizeHint().width()) / 2;
    move(iXOffset, target.height() - 9 - sizeHint().height());
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorSearchBar::Show()
{
  COverlayBase::Show();
  m_pSearchWidget->SetFocus();
  emit SignalShown();
}

//----------------------------------------------------------------------------------------
//
void CEditorSearchBar::SlotShowHideSearchFilter()
{
  if (isVisible())
  {
    Hide();
  }
  else
  {
    Show();
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorSearchBar::SlotBackClicked()
{
  m_searchDir = ESearchDirection::eBackward;
  emit SignalFilterChanged(m_searchDir, m_pSearchWidget->Filter());
}

//----------------------------------------------------------------------------------------
//
void CEditorSearchBar::SlotForwardClicked()
{
  m_searchDir = ESearchDirection::eForward;
  emit SignalFilterChanged(m_searchDir, m_pSearchWidget->Filter());
}

//----------------------------------------------------------------------------------------
//
void CEditorSearchBar::SlotFilterChanged(const QString& sText, bool bReturnPressed)
{
  m_searchDir = bReturnPressed ? ESearchDirection::eForward : ESearchDirection::eNone;
  emit SignalFilterChanged(m_searchDir, sText);
}
