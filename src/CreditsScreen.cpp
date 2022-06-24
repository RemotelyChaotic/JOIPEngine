#include "CreditsScreen.h"
#include "Application.h"
#include "ui_CreditsScreen.h"

#include "Utils/WidgetHelpers.h"

#include <QFile>
#include <QFont>
#include <QMessageBox>
#include <QScroller>

CCreditsScreen::CCreditsScreen(const std::shared_ptr<CWindowContext>& spWindowContext,
                               QWidget* pParent) :
  QWidget(pParent),
  IAppStateScreen(spWindowContext),
  m_spUi(std::make_unique<Ui::CCreditsScreen>())
{
  m_spUi->setupUi(this);
  Initialize();
}

CCreditsScreen::~CCreditsScreen()
{
}

//----------------------------------------------------------------------------------------
//
void CCreditsScreen::Initialize()
{
  m_bInitialized = false;

  connect(CApplication::Instance(), &CApplication::StyleLoaded,
          this, &CCreditsScreen::SlotStyleLoaded, Qt::QueuedConnection);

#if defined(Q_OS_ANDROID)
  QScroller::grabGesture(m_spUi->pScrollArea, QScroller::LeftMouseButtonGesture);
  QScroller::grabGesture(m_spUi->pScrollArea_2, QScroller::LeftMouseButtonGesture);

  widget_helpers::RetainSizeAndHide(m_spUi->pBackButton);
#endif

  m_bInitialized = true;
}

//----------------------------------------------------------------------------------------
//
void CCreditsScreen::Load()
{
  emit m_spWindowContext->SignalSetDownloadButtonVisible(false);
  emit m_spWindowContext->SignalSetHelpButtonVisible(false);

  QFile creditsFile(":/resources/data/credits.txt");
  if (creditsFile.open(QIODevice::ReadOnly))
  {
    QString sCredits = QString::fromUtf8(creditsFile.readAll());
    m_spUi->pTextCredits->setText(sCredits);
  }

  QFile changelogFile(":/changelog/changelog.txt");
  if (changelogFile.open(QIODevice::ReadOnly))
  {
    QString sChangelog = QString::fromUtf8(changelogFile.readAll());
    m_spUi->pTextChangelog->setText(sChangelog);
  }
}

//----------------------------------------------------------------------------------------
//
void CCreditsScreen::Unload()
{
  emit UnloadFinished();
}

//----------------------------------------------------------------------------------------
//
void CCreditsScreen::on_pBackButton_clicked()
{
  WIDGET_INITIALIZED_GUARD
  emit m_spWindowContext->SignalChangeAppState(EAppState::eMainScreen);
}

//----------------------------------------------------------------------------------------
//
void CCreditsScreen::on_pAboutQtButton_clicked()
{
  WIDGET_INITIALIZED_GUARD
  QMessageBox::aboutQt(this, tr("About Qt"));
}

//----------------------------------------------------------------------------------------
//
void CCreditsScreen::SlotStyleLoaded()
{
  QFont font = m_spUi->pTextCredits->font();
  font.setFamily(CApplication::Instance()->Settings()->Font());
  m_spUi->pTextCredits->setFont(font);

  font = m_spUi->pTextChangelog->font();
  font.setFamily(CApplication::Instance()->Settings()->Font());
  m_spUi->pTextChangelog->setFont(font);
}
