#include "ScriptSearchBar.h"
#include "Widgets/SearchWidget.h"
#include <QHBoxLayout>
#include <QPushButton>

CScriptSearchBar::CScriptSearchBar(QWidget* pParent) :
  COverlayBase(0, pParent),
  m_bForward(true)
{
  QHBoxLayout* pLayout = new QHBoxLayout(this);
  m_pSearchWidget = new CSearchWidget(this);
  connect(m_pSearchWidget, &CSearchWidget::SignalFilterChanged,
          this, &CScriptSearchBar::SlotFilterChanged);
  pLayout->addWidget(m_pSearchWidget);

  m_pBack = new QPushButton(this);
  m_pBack->setObjectName("BackButton");
  m_pBack->setToolTip("Search backwards");
  connect(m_pBack, &QPushButton::clicked,
          this, &CScriptSearchBar::SlotBackClicked);
  pLayout->addWidget(m_pBack);

  m_pForward = new QPushButton(this);
  m_pForward->setObjectName("ForardButton");
  m_pForward->setToolTip("Search forwards");
  connect(m_pForward, &QPushButton::clicked,
          this, &CScriptSearchBar::SlotForwardClicked);
  pLayout->addWidget(m_pForward);

  m_pCloseButton = new QPushButton(this);
  m_pCloseButton->setObjectName("CloseButton");
  m_pForward->setToolTip("Close search");
  connect(m_pCloseButton, &QPushButton::clicked, this, &CScriptSearchBar::Hide);
  pLayout->addWidget(m_pCloseButton);
}

CScriptSearchBar::~CScriptSearchBar()
{

}

//----------------------------------------------------------------------------------------
//
void CScriptSearchBar::Climb()
{
  ClimbToFirstInstanceOf("CScriptEditorWidget", false);
}

//----------------------------------------------------------------------------------------
//
void CScriptSearchBar::Hide()
{
  m_pSearchWidget->blockSignals(true);
  COverlayBase::Hide();
  m_pSearchWidget->blockSignals(false);
  emit SignalHidden();
}

//----------------------------------------------------------------------------------------
//
void CScriptSearchBar::Resize()
{
  if (nullptr != m_pTargetWidget)
  {
    QSize target = m_pTargetWidget->size();
    move((target.width() - width()) / 2, 9);
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptSearchBar::Show()
{
  COverlayBase::Show();
  m_pSearchWidget->SetFocus();
}

//----------------------------------------------------------------------------------------
//
void CScriptSearchBar::SlotBackClicked()
{
  m_bForward = false;
  emit SignalFilterChanged(false, m_pSearchWidget->Filter());
}

//----------------------------------------------------------------------------------------
//
void CScriptSearchBar::SlotForwardClicked()
{
  m_bForward = true;
  emit SignalFilterChanged(true, m_pSearchWidget->Filter());
}

//----------------------------------------------------------------------------------------
//
void CScriptSearchBar::SlotFilterChanged(const QString& sText)
{
  emit SignalFilterChanged(m_bForward, sText);
}
