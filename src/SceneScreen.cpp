#include "SceneScreen.h"
#include "Application.h"
#include "Settings.h"
#include "WindowContext.h"
#include "ui_SceneScreen.h"

#include "Systems/DatabaseManager.h"
#include "Systems/HelpFactory.h"
#include "Systems/Project.h"

#include "Utils/WidgetHelpers.h"

#include "Widgets/HelpOverlay.h"

#include <QMessageBox>

namespace
{
  const qint32 c_iPageIndexChoice = 0;
  const qint32 c_iPageIndexScene = 1;

  const QString c_sOpenHelpId = "Player/Open";
  const QString c_sCancelHelpId = "MainScreen/Cancel";
}

CSceneScreen::CSceneScreen(const std::shared_ptr<CWindowContext>& spWindowContext,
                                             QWidget* pParent) :
  QWidget(pParent),
  IAppStateScreen(spWindowContext),
  m_spUi(std::make_unique<Ui::CSceneScreen>()),
  m_wpDbManager()
{
  m_spUi->setupUi(this);
  Initialize();
}

CSceneScreen::~CSceneScreen()
{
}

//----------------------------------------------------------------------------------------
//
bool CSceneScreen::CloseApplication()
{
  return m_spUi->pMainSceneScreen->CloseApplication();
}

//----------------------------------------------------------------------------------------
//
void CSceneScreen::Initialize()
{
  m_bInitialized = false;

  m_spUi->pMainSceneScreen->Initialize(m_spWindowContext, false);

  m_wpDbManager = CApplication::Instance()->System<CDatabaseManager>();

  auto wpHelpFactory = CApplication::Instance()->System<CHelpFactory>().lock();
  if (nullptr != wpHelpFactory)
  {
    m_spUi->pOpenExistingProjectButton->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sOpenHelpId);
    wpHelpFactory->RegisterHelp(c_sOpenHelpId, ":/resources/help/player/open_button_help.html");
    m_spUi->pCancelButton->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sCancelHelpId);
    wpHelpFactory->RegisterHelp(c_sCancelHelpId, ":/resources/help/cancel_button_help.html");
  }

  connect(m_spUi->pMainSceneScreen, &CSceneMainScreen::SignalExitClicked,
          this, &CSceneScreen::SlotExitClicked, Qt::DirectConnection);
  connect(m_spUi->pMainSceneScreen, &CSceneMainScreen::SignalUnloadFinished,
          this, &CSceneScreen::SlotSceneUnloadFinished, Qt::DirectConnection);
  connect(m_spUi->pProjectCardSelectionWidget, &CProjectCardSelectionWidget::SignalUnloadFinished,
          this, &CSceneScreen::SlotCardsUnloadFinished);

  m_spUi->pStackedWidget->setCurrentIndex(c_iPageIndexChoice);

#if defined(Q_OS_ANDROID)
  widget_helpers::RetainSizeAndHide(m_spUi->pCancelButton);
#endif

  m_bInitialized = true;
}

//----------------------------------------------------------------------------------------
//
void CSceneScreen::Load()
{
  m_spUi->pProjectCardSelectionWidget->LoadProjects(EDownloadStateFlag::eFinished);
}

//----------------------------------------------------------------------------------------
//
void CSceneScreen::Unload()
{
  m_spUi->pProjectCardSelectionWidget->UnloadProjects();
}

//----------------------------------------------------------------------------------------
//
void CSceneScreen::on_pOpenExistingProjectButton_clicked()
{
  WIDGET_INITIALIZED_GUARD

  qint32 iId = -1;
  tspProject spProject = nullptr;
  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    iId = m_spUi->pProjectCardSelectionWidget->SelectedId();
    spProject = spDbManager->FindProject(iId);
    if (nullptr == spProject)
    {
      return;
    }
  }

  auto spSettings = CApplication::Instance()->Settings();

  QReadLocker locker(&spProject->m_rwLock);
  if (spProject->m_bUsesWeb && spSettings->Offline())
  {
    m_spUi->pProjectCardSelectionWidget->ShowWarning(
          tr("Can't open JOIP-Project that requires online resources in offline mode in the player."));
  }
  else
  {
    locker.unlock();

    // set dominant hand now, if not set yet
    if (DominantHand::EDominantHand::NoDominantHand == spSettings->GetDominantHand())
    {
      QMessageBox msgBox;
      msgBox.setText("Predominantly used hand has not been configured");
      msgBox.setInformativeText("The JOIP-Project Player was developped with one-handed use in mind.\n"
                                "Please select the hand, you will be using this\n"
                                "application with predominantly, to configure it properly.\n"
                                "(You can always change this in the settings.)");
      auto pLeft = new QPushButton("Left");
      auto pRight = new QPushButton("Right");
      msgBox.addButton(pLeft, QMessageBox::ApplyRole);
      msgBox.addButton(pRight, QMessageBox::ApplyRole);
      msgBox.setModal(true);
      msgBox.setWindowFlag(Qt::FramelessWindowHint);
      QPointer<CSceneScreen> pMeMyselfMyPointerAndI(this);
      msgBox.exec();
      if (nullptr == pMeMyselfMyPointerAndI)
      {
        return;
      }

      if (msgBox.clickedButton() == pLeft)
      {
        spSettings->SetDominantHand(DominantHand::EDominantHand::Left);
      }
      else
      {
        spSettings->SetDominantHand(DominantHand::EDominantHand::Right);
      }
    }

    // will eventually emit a signal but we can ignore that
    m_spUi->pProjectCardSelectionWidget->UnloadProjects();

    emit m_spWindowContext->SignalSetLeftButtonsVisible(false);
    emit m_spWindowContext->SignalSetHelpButtonVisible(false);

    m_spUi->pMainSceneScreen->LoadProject(iId);
    m_spUi->pStackedWidget->setCurrentIndex(c_iPageIndexScene);
  }
}

//----------------------------------------------------------------------------------------
//
void CSceneScreen::on_pCancelButton_clicked()
{
  WIDGET_INITIALIZED_GUARD
  m_spUi->pStackedWidget->setCurrentIndex(c_iPageIndexChoice);
  emit m_spWindowContext->SignalChangeAppState(EAppState::eMainScreen);
}

//----------------------------------------------------------------------------------------
//
void CSceneScreen::SlotExitClicked()
{
  WIDGET_INITIALIZED_GUARD
  m_spUi->pStackedWidget->setCurrentIndex(c_iPageIndexChoice);
  emit m_spWindowContext->SignalChangeAppState(EAppState::eMainScreen);
}

//----------------------------------------------------------------------------------------
//
void CSceneScreen::SlotSceneUnloadFinished()
{
  WIDGET_INITIALIZED_GUARD
  emit UnloadFinished();
}

//----------------------------------------------------------------------------------------
//
void CSceneScreen::SlotCardsUnloadFinished()
{
  WIDGET_INITIALIZED_GUARD
  emit UnloadFinished();
}
