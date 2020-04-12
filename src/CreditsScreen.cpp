#include "CreditsScreen.h"
#include "Application.h"
#include "ui_CreditsScreen.h"
#include <QFile>
#include <QFont>
#include <QMessageBox>

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

  QFont font = m_spUi->pText->font();
  font.setPixelSize(20);
  m_spUi->pText->setFont(font);

  m_bInitialized = true;
}

//----------------------------------------------------------------------------------------
//
void CCreditsScreen::Load()
{
  QFile creditsFile("://resources/credits.txt");
  if (creditsFile.open(QIODevice::ReadOnly))
  {
    QString sCredits = QString::fromUtf8(creditsFile.readAll());
    m_spUi->pText->setText(sCredits);
  }
}

//----------------------------------------------------------------------------------------
//
void CCreditsScreen::Unload()
{

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
  QFont font = m_spUi->pText->font();
  font.setPixelSize(20);
  font.setFamily(CApplication::Instance()->Settings()->Font());
  m_spUi->pText->setFont(font);
}
