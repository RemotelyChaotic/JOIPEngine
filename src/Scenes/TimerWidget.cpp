#include "TimerWidget.h"
#include "Application.h"
#include "Settings.h"
#include "ui_TimerWidget.h"

CTimerWidget::CTimerWidget(QWidget* pParent) :
  QWidget(pParent),
  IWidgetBaseInterface(),
  m_spUi(std::make_unique<Ui::CTimerWidget>()),
  m_spSettings(CApplication::Instance()->Settings())
{
  m_spUi->setupUi(this);
}

CTimerWidget::~CTimerWidget()
{
}

//----------------------------------------------------------------------------------------
//
void CTimerWidget::Initialize()
{
  m_bInitialized = false;

  // initializing done
  m_bInitialized = true;
}
