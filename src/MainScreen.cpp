#include "MainScreen.h"
#include "Application.h"
#include "version.h"
#include "WindowContext.h"
#include "ui_MainScreen.h"
#include "Systems/HelpFactory.h"
#include "Widgets/HelpOverlay.h"
#include <QApplication>

namespace {
  const QString c_sPlayHelpId = "MainScreen/Play";
  const QString c_sEditorHelpId = "MainScreen/Editor";
  const QString c_sSettingsHelpId = "MainScreen/Settings";
  const QString c_sCreditsHelpId = "MainScreen/Credits";
  const QString c_sExitHelpId = "MainScreen/Exit";
}

//----------------------------------------------------------------------------------------
//
CMainScreen::CMainScreen(const std::shared_ptr<CWindowContext>& spWindowContext,
                         QWidget* pParent) :
  QWidget(pParent),
  IAppStateScreen(spWindowContext),
  m_spUi(std::make_unique<Ui::CMainScreen>())
{
  m_spUi->setupUi(this);
  Initialize();
}

CMainScreen::~CMainScreen()
{
}

//----------------------------------------------------------------------------------------
//
void CMainScreen::Initialize()
{
  m_bInitialized = false;

  auto wpHelpFactory = CApplication::Instance()->System<CHelpFactory>().lock();
  if (nullptr != wpHelpFactory)
  {
    m_spUi->pSceneSelectButton->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sPlayHelpId);
    wpHelpFactory->RegisterHelp(c_sPlayHelpId, ":/resources/help/play_button_help.html");
    m_spUi->pEdiorButton->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sEditorHelpId);
    wpHelpFactory->RegisterHelp(c_sEditorHelpId, ":/resources/help/editor_button_help.html");
    m_spUi->pSettingsButton->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sSettingsHelpId);
    wpHelpFactory->RegisterHelp(c_sSettingsHelpId, ":/resources/help/settings_button_help.html");
    m_spUi->pCreditsButton->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sCreditsHelpId);
    wpHelpFactory->RegisterHelp(c_sCreditsHelpId, ":/resources/help/credits_button_help.html");
    m_spUi->pQuitButton->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sExitHelpId);
    wpHelpFactory->RegisterHelp(c_sExitHelpId, ":/resources/help/exit_button_help.html");
  }

  connect(CApplication::Instance(), &CApplication::StyleLoaded,
          this, &CMainScreen::SlotStyleLoaded, Qt::QueuedConnection);

  m_spUi->pVersionLabel->setText("v" VERSION_DOT);
  QFont versionFont = m_spUi->pVersionLabel->font();
  versionFont.setPixelSize(30);
  m_spUi->pVersionLabel->setFont(versionFont);

  m_bInitialized = true;
}

//----------------------------------------------------------------------------------------
//
void CMainScreen::Load()
{

}

//----------------------------------------------------------------------------------------
//
void CMainScreen::Unload()
{

}

//----------------------------------------------------------------------------------------
//
void CMainScreen::on_pSceneSelectButton_clicked()
{
  WIDGET_INITIALIZED_GUARD

  emit m_spWindowContext->SignalChangeAppState(EAppState::eSceneScreen);
}

//----------------------------------------------------------------------------------------
//
void CMainScreen::on_pEdiorButton_clicked()
{
  WIDGET_INITIALIZED_GUARD

  emit m_spWindowContext->SignalChangeAppState(EAppState::eEditorScreen);
}

//----------------------------------------------------------------------------------------
//
void CMainScreen::on_pSettingsButton_clicked()
{
  WIDGET_INITIALIZED_GUARD

  emit m_spWindowContext->SignalChangeAppState(EAppState::eSettingsScreen);
}

//----------------------------------------------------------------------------------------
//
void CMainScreen::on_pCreditsButton_clicked()
{
  WIDGET_INITIALIZED_GUARD

  emit m_spWindowContext->SignalChangeAppState(EAppState::eCreditsScreen);
}

//----------------------------------------------------------------------------------------
//
void CMainScreen::on_pQuitButton_clicked()
{
  WIDGET_INITIALIZED_GUARD

  emit QApplication::instance()->quit();
}

//----------------------------------------------------------------------------------------
//
void CMainScreen::SlotStyleLoaded()
{
  QFont versionFont = m_spUi->pVersionLabel->font();
  versionFont.setPixelSize(30);
  versionFont.setFamily(CApplication::Instance()->Settings()->Font());
  m_spUi->pVersionLabel->setFont(versionFont);
}
