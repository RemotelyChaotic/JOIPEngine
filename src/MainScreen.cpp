#include "MainScreen.h"
#include "Application.h"
#include "out_Version.h"
#include "WindowContext.h"
#include "ui_MainScreen.h"
#include "Systems/HelpFactory.h"
#include "Widgets/DownloadButtonOverlay.h"
#include "Widgets/HelpOverlay.h"
#include <QApplication>
#include <QDebug>
#include <QResizeEvent>
#include <QSpacerItem>

namespace
{
  const QString c_sPlayHelpId = "MainScreen/Play";
  const QString c_sEditorHelpId = "MainScreen/Editor";
  const QString c_sSettingsHelpId = "MainScreen/Settings";
  const QString c_sCreditsHelpId = "MainScreen/Credits";
  const QString c_sExitHelpId = "MainScreen/Exit";
  const QString c_sProjectHelpId = "Project/JOIPProject";
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

    wpHelpFactory->RegisterHelp(c_sProjectHelpId, ":/resources/help/project/joi_project_help.html");
  }

  connect(CApplication::Instance(), &CApplication::StyleLoaded,
          this, &CMainScreen::SlotStyleLoaded, Qt::QueuedConnection);

  m_spUi->pVersionLabel->setText("v" VERSION_XYZ);
  m_spUi->pVersionLabel->SetFontSize(30);

  m_pHelpButtonOverlay = window()->findChild<CHelpButtonOverlay*>();
  m_pDownloadButtonOverlay = window()->findChild<CDownloadButtonOverlay*>();

#if defined(Q_OS_ANDROID)
  // on android we don't need a quit button
  m_spUi->pQuitButton->parentWidget()->layout()->removeWidget(m_spUi->pQuitButton);
  m_spUi->pQuitButton->setParent(this);
  m_spUi->pQuitButton->hide();
 #endif

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
  emit UnloadFinished();
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
  versionFont.setFamily(CApplication::Instance()->Settings()->Font());
  m_spUi->pVersionLabel->SetFontSize(30);
}

//----------------------------------------------------------------------------------------
//
void CMainScreen::resizeEvent(QResizeEvent* pEvt)
{
  if (nullptr == pEvt) { return; }

  if (nullptr != m_pDownloadButtonOverlay && nullptr != m_pHelpButtonOverlay)
  {
    const QSize titleSize = m_spUi->pTitle->SuggestedSize();
    const qint32 iSpaceInMiddle = parentWidget()->width()-
        (m_pDownloadButtonOverlay->x() + m_pDownloadButtonOverlay->width()) -
        (parentWidget()->width() - m_pHelpButtonOverlay->x());
#if defined(Q_OS_ANDROID)
    if (height() > width())
#else
    if (titleSize.width() > iSpaceInMiddle)
#endif
    {
      m_spUi->pTopVerticalSpacer->changeSize(20, m_pHelpButtonOverlay->y() + m_pHelpButtonOverlay->height() + 10,
                                             QSizePolicy::Minimum, QSizePolicy::Maximum);
    }
    else
    {
      m_spUi->pTopVerticalSpacer->changeSize(20, 20, QSizePolicy::Minimum, QSizePolicy::Maximum);
    }
    layout()->invalidate();
  }
}
