#include "InformationWidget.h"
#include "Application.h"
#include "Settings.h"
#include "ui_InformationWidget.h"

CInformationWidget::CInformationWidget(QWidget *parent) :
  QWidget(parent),
  IWidgetBaseInterface(),
  m_spUi(std::make_unique<Ui::CInformationWidget>()),
  m_spSettings(CApplication::Instance()->Settings())
{
  m_spUi->setupUi(this);
}

CInformationWidget::~CInformationWidget()
{
}

//----------------------------------------------------------------------------------------
//
void CInformationWidget::Initialize()
{
  m_bInitialized = false;

  // initializing done
  m_bInitialized = true;
}
