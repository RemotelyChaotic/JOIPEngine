#include "TextBoxWidget.h"
#include "Application.h"
#include "Settings.h"
#include "ui_TextBoxWidget.h"

#include <QScrollBar>

CTextBoxWidget::CTextBoxWidget(QWidget* pParent) :
  QWidget(pParent),
  IWidgetBaseInterface(),
  m_spUi(std::make_unique<Ui::CTextBoxWidget>()),
  m_spSettings(CApplication::Instance()->Settings())
{
  m_spUi->setupUi(this);
}

CTextBoxWidget::~CTextBoxWidget()
{
}

//----------------------------------------------------------------------------------------
//
void CTextBoxWidget::Initialize()
{
  m_bInitialized = false;

  m_spUi->pScrollArea->horizontalScrollBar()->hide();
  m_spUi->pScrollArea->verticalScrollBar()->hide();
  m_spUi->pScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
  m_spUi->pScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);

  // initializing done
  m_bInitialized = true;
}
