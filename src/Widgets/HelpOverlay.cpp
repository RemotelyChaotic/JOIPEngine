#include "HelpOverlay.h"
#include "Application.h"
#include "Systems/HelpFactory.h"
#include "ui_HelpOverlay.h"

CHelpOverlay::CHelpOverlay(QWidget* pParent) :
  COverlayBase(pParent),
  m_spUi(std::make_unique<Ui::CHelpOverlay>()),
  m_wpHelpFactory(CApplication::Instance()->System<CHelpFactory>())
{
  m_spUi->setupUi(this);
}

CHelpOverlay::~CHelpOverlay()
{
}

//----------------------------------------------------------------------------------------
//
