#include "EditorSearchBar.h"
#include "Widgets/SearchWidget.h"
#include <QHBoxLayout>
#include <QPushButton>

CEditorSearchBar::CEditorSearchBar(QWidget* pParent) :
  COverlayBase(0, pParent),
  m_bForward(true)
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
  ClimbToFirstInstanceOf("CScriptEditorWidget", false);
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
    move((target.width() - width()) / 2, 9);
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
  m_bForward = false;
  emit SignalFilterChanged(false, m_pSearchWidget->Filter());
}

//----------------------------------------------------------------------------------------
//
void CEditorSearchBar::SlotForwardClicked()
{
  m_bForward = true;
  emit SignalFilterChanged(true, m_pSearchWidget->Filter());
}

//----------------------------------------------------------------------------------------
//
void CEditorSearchBar::SlotFilterChanged(const QString& sText)
{
  emit SignalFilterChanged(m_bForward, sText);
}
