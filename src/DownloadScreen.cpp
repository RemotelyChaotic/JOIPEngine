#include "DownloadScreen.h"
#include "Application.h"
#include "Settings.h"
#include "WindowContext.h"
#include "ui_DownloadScreen.h"

#include "Systems/DatabaseManager.h"
#include "Systems/HelpFactory.h"
#include "Systems/Database/Project.h"
#include "Systems/ProjectDownloader.h"

#include "Utils/PlatformHelpers.h"
#include "Utils/WidgetHelpers.h"

#include "Widgets/HelpOverlay.h"

#include <QUrl>

namespace
{
  const QString c_sCancelHelpId = "MainScreen/Cancel";
}

CDownloadScreen::CDownloadScreen(const std::shared_ptr<CWindowContext>& spWindowContext,
                                 QWidget* pParent) :
  QWidget(pParent),
  IAppStateScreen(spWindowContext),
  m_spUi(std::make_unique<Ui::CDownloadScreen>())
{
  m_spUi->setupUi(this);
  Initialize();
}

CDownloadScreen::~CDownloadScreen()
{
}

//----------------------------------------------------------------------------------------
//
void CDownloadScreen::Initialize()
{
  m_bInitialized = false;

  m_spUi->OpenExplorerButton->setProperty("styleSmall", true);
  m_spUi->ReloadProjectButton->setProperty("styleSmall", true);

#ifdef Q_OS_ANDROID
  m_spUi->OpenExplorerButton->hide();
  m_spUi->ReloadProjectButton->hide();
#endif

  m_wpDbManager = CApplication::Instance()->System<CDatabaseManager>();

  auto wpHelpFactory = CApplication::Instance()->System<CHelpFactory>().lock();
  if (nullptr != wpHelpFactory)
  {
    m_spUi->pCancelButton->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sCancelHelpId);
    wpHelpFactory->RegisterHelp(c_sCancelHelpId, ":/resources/help/cancel_button_help.html");
  }

  connect(m_spUi->pProjectCardSelectionWidget, &CProjectCardSelectionWidget::SignalUnloadFinished,
          this, &CDownloadScreen::SlotCardsUnloadFinished);

#if defined(Q_OS_ANDROID)
  widget_helpers::RetainSizeAndHide(m_spUi->pCancelButton);
#endif

  m_bInitialized = true;
}

//----------------------------------------------------------------------------------------
//
void CDownloadScreen::Load()
{
  m_spUi->pProjectCardSelectionWidget->LoadProjects(EDownloadStateFlag::eUnstarted |
                                                    EDownloadStateFlag::eDownloadRunning);
}

//----------------------------------------------------------------------------------------
//
void CDownloadScreen::Unload()
{
  m_spUi->pProjectCardSelectionWidget->UnloadProjects();
}

//----------------------------------------------------------------------------------------
//
void CDownloadScreen::on_pDownloadButton_clicked()
{
  WIDGET_INITIALIZED_GUARD
  QUrl url = QUrl::fromUserInput(m_spUi->pUrlLineEdit->text());
  auto wpDownloader = CApplication::Instance()->System<CProjectDownloader>();
  if (auto spDownloader = wpDownloader.lock())
  {
    spDownloader->CreateNewDownloadJob(url.host(), QVariantList() << url);
  }
}

//----------------------------------------------------------------------------------------
//
void CDownloadScreen::on_OpenExplorerButton_clicked()
{
  WIDGET_INITIALIZED_GUARD

  qint32 iId = m_spUi->pProjectCardSelectionWidget->SelectedId();
  tspProject spProject = nullptr;
  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    spProject = spDbManager->FindProject(iId);
  }

  if (-1 == iId || nullptr == spProject)
  {
    QString sFolder = CApplication::Instance()->Settings()->ContentFolder();
    helpers::ShowInGraphicalShell(QFileInfo(sFolder).absoluteFilePath());
  }
  else
  {
    QString sFolder = PhysicalProjectPath(spProject);
    helpers::ShowInGraphicalShell(QFileInfo(sFolder).absoluteFilePath());
  }
}

//----------------------------------------------------------------------------------------
//
void CDownloadScreen::on_ReloadProjectButton_clicked()
{
  WIDGET_INITIALIZED_GUARD

  if (auto spManager = m_wpDbManager.lock(); nullptr != spManager)
  {
    bool bOk = QMetaObject::invokeMethod(spManager.get(), "SlotContentFolderChanged",
                                         Qt::QueuedConnection);
    assert(bOk); Q_UNUSED(bOk);
  }
}

//----------------------------------------------------------------------------------------
//
void CDownloadScreen::on_pCancelButton_clicked()
{
  WIDGET_INITIALIZED_GUARD
  emit m_spWindowContext->SignalChangeAppState(EAppState::eMainScreen, CWindowContext::eVertical);
}

//----------------------------------------------------------------------------------------
//
void CDownloadScreen::SlotExitClicked()
{
  WIDGET_INITIALIZED_GUARD
  emit m_spWindowContext->SignalChangeAppState(EAppState::eMainScreen, CWindowContext::eVertical);
}

//----------------------------------------------------------------------------------------
//
void CDownloadScreen::SlotCardsUnloadFinished()
{
  WIDGET_INITIALIZED_GUARD
  emit UnloadFinished();
}
