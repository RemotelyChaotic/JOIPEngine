#include "EditorSearchBar.h"
#include "Widgets/SearchWidget.h"
#include <QHBoxLayout>
#include <QPushButton>

CEditorSearchBar::CEditorSearchBar(QWidget* pParent) :
  COverlayBase(0, pParent),
  m_searchDir(eNone)
{
  QHBoxLayout* pLayout = new QHBoxLayout(this);
  m_pSearchWidget = new CSearchWidget(this);
  connect(m_pSearchWidget, &CSearchWidget::SignalFilterChanged,
          this, &CEditorSearchBar::SlotFilterChanged);
  pLayout->addWidget(m_pSearchWidget);

  m_pBack = new QPushButton(this);
  m_pBack->setObjectName("BackButton");
  m_pBack->setToolTip("Search backwards");
  connect(m_pBack, &QPushButton::clicked,
          this, &CEditorSearchBar::SlotBackClicked);
  pLayout->addWidget(m_pBack);

  m_pForward = new QPushButton(this);
  m_pForward->setObjectName("ForardButton");
  m_pForward->setToolTip("Search forwards");
  connect(m_pForward, &QPushButton::clicked,
          this, &CEditorSearchBar::SlotForwardClicked);
  pLayout->addWidget(m_pForward);

  m_pCloseButton = new QPushButton(this);
  m_pCloseButton->setObjectName("CloseButton");
  m_pForward->setToolTip("Close search");
  connect(m_pCloseButton, &QPushButton::clicked, this, &CEditorSearchBar::Hide);
  pLayout->addWidget(m_pCloseButton);

  pParent->metaObject()->className();
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
}

//----------------------------------------------------------------------------------------
//
void CEditorSearchBar::SlotBackClicked()
{
  m_searchDir = ESearhDirection::eBackward;
  emit SignalFilterChanged(ESearhDirection::eBackward, m_pSearchWidget->Filter());
}

//----------------------------------------------------------------------------------------
//
void CEditorSearchBar::SlotForwardClicked()
{
  m_searchDir = ESearhDirection::eForward;
  emit SignalFilterChanged(ESearhDirection::eForward, m_pSearchWidget->Filter());
}

//----------------------------------------------------------------------------------------
//
void CEditorSearchBar::SlotFilterChanged(const QString& sText)
{
  emit SignalFilterChanged(ESearhDirection::eNone, sText);
}
