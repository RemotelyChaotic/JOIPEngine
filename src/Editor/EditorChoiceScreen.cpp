#include "EditorChoiceScreen.h"
#include "Application.h"
#include "ui_EditorChoiceScreen.h"

#include "Systems/DatabaseManager.h"
#include "Systems/EOS/EosHelpers.h"
#include "Systems/HelpFactory.h"
#include "Systems/Project.h"
#include "Utils/WidgetHelpers.h"
#include "Widgets/HelpOverlay.h"

namespace
{
  const qint32 c_iPageIndexChoice = 0;
  const qint32 c_iPageIndexNew    = 1;
  const qint32 c_iPageIndexOpen   = 2;

  const QString c_sNewProjectHelpId = "Editor/NewProject";
  const QString c_sOpenProjectHelpId = "Editor/OpenProject";
  const QString c_sProjectNameHelpId = "Editor/ProjectName";
  const QString c_sCreateProjectHelpId = "Editor/CreateProject";
  const QString c_sCancelHelpId = "MainScreen/Cancel";
}

//----------------------------------------------------------------------------------------
//
CEditorChoiceScreen::CEditorChoiceScreen(QWidget* pParent) :
  QWidget(pParent),
  m_spUi(std::make_unique<Ui::CEditorChoiceScreen>()),
  m_wpDbManager(),
  m_bInitialized(false)
{
  m_spUi->setupUi(this);
  m_spUi->pEnableTutorialCheckBox->hide();
  Initialize();
}

CEditorChoiceScreen::~CEditorChoiceScreen()
{
}

//----------------------------------------------------------------------------------------
//
void CEditorChoiceScreen::Initialize()
{
  m_bInitialized = false;

  m_wpDbManager = CApplication::Instance()->System<CDatabaseManager>();

  auto wpHelpFactory = CApplication::Instance()->System<CHelpFactory>().lock();
  if (nullptr != wpHelpFactory)
  {
    m_spUi->pNewProjectButton->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sNewProjectHelpId);
    wpHelpFactory->RegisterHelp(c_sNewProjectHelpId, ":/resources/help/editor/newproject_button_help.html");
    m_spUi->pOpenProjectButton->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sOpenProjectHelpId);
    wpHelpFactory->RegisterHelp(c_sOpenProjectHelpId, ":/resources/help/editor/openproject_button_help.html");
    m_spUi->pProjectNameLineEdit->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sProjectNameHelpId);
    wpHelpFactory->RegisterHelp(c_sProjectNameHelpId, ":/resources/help/editor/project_name_help.html");
    m_spUi->pCreateProjectButton->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sCreateProjectHelpId);
    wpHelpFactory->RegisterHelp(c_sCreateProjectHelpId, ":/resources/help/editor/createproject_button_help.html");
    m_spUi->pOpenExistingProjectButton->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sOpenProjectHelpId);
    m_spUi->pCancelButton->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sCancelHelpId);
    wpHelpFactory->RegisterHelp(c_sCancelHelpId, ":/resources/help/player/cancel_button_help.html");
  }

  m_spUi->pStackedWidget->setCurrentIndex(c_iPageIndexChoice);
  m_spUi->pErrorLabel->setVisible(false);

  connect(m_spUi->pProjectCardSelectionWidget, &CProjectCardSelectionWidget::SignalUnloadFinished,
          this, &CEditorChoiceScreen::SignalUnloadFinished);

#if defined(Q_OS_ANDROID)
  widget_helpers::RetainSizeAndHide(m_spUi->pCancelButton);
#endif

  m_bInitialized = true;
}

//----------------------------------------------------------------------------------------
//
void CEditorChoiceScreen::Load()
{
  m_spUi->pProjectCardSelectionWidget->LoadProjects(EDownloadStateFlag::eFinished);
}

//----------------------------------------------------------------------------------------
//
void CEditorChoiceScreen::Unload()
{
  m_spUi->pProjectCardSelectionWidget->UnloadProjects();
}

//----------------------------------------------------------------------------------------
//
void CEditorChoiceScreen::on_pNewProjectButton_clicked()
{
  if (!m_bInitialized) { return; }
  m_spUi->pStackedWidget->SlideInIdx(c_iPageIndexNew);
}

//----------------------------------------------------------------------------------------
//
void CEditorChoiceScreen::on_pOpenProjectButton_clicked()
{
  if (!m_bInitialized) { return; }
  m_spUi->pStackedWidget->SlideInIdx(c_iPageIndexOpen);
}

//----------------------------------------------------------------------------------------
//
void CEditorChoiceScreen::on_pCancelButton_clicked()
{
  if (!m_bInitialized) { return; }

  emit SignalCancelClicked();
  m_spUi->pStackedWidget->SlideInIdx(c_iPageIndexChoice);
}

//----------------------------------------------------------------------------------------
//
void CEditorChoiceScreen::on_pProjectNameLineEdit_textChanged(const QString &text)
{
  if (!m_bInitialized) { return; }

  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    QString sErrorText;
    if (!ProjectNameCheck(text, &sErrorText))
    {
      m_spUi->pErrorLabel->setText(sErrorText);
      m_spUi->pErrorLabel->setVisible(true);
    }
    else
    {
      m_spUi->pErrorLabel->setVisible(false);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorChoiceScreen::on_pCreateProjectButton_clicked()
{
  if (!m_bInitialized) { return; }
  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    QString sNewName = m_spUi->pProjectNameLineEdit->text();
    QString sErrorText;
    if (!ProjectNameCheck(sNewName, &sErrorText))
    {
      m_spUi->pErrorLabel->setText(sErrorText);
      m_spUi->pErrorLabel->setVisible(true);
    }
    else
    {
      m_spUi->pErrorLabel->setVisible(false);
      emit SignalNewClicked(sNewName, /*m_spUi->pEnableTutorialCheckBox->isChecked()*/false);
      m_spUi->pStackedWidget->SlideInIdx(c_iPageIndexChoice);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorChoiceScreen::on_pOpenExistingProjectButton_clicked()
{
  if (!m_bInitialized) { return; }

  bool bOk = true;
  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    qint32 iId = m_spUi->pProjectCardSelectionWidget->SelectedId();
    tspProject spProject = spDbManager->FindProject(iId);
    if (spProject != nullptr)
    {
#ifndef NDEBUG
#if defined(Q_OS_WIN)
      if (false)
#else
      if (!eos::VerifyProjectEditable(spProject))
#endif
#else
      if (!eos::VerifyProjectEditable(spProject))
#endif
      {
        bOk = false;
        m_spUi->pProjectCardSelectionWidget->ShowWarning(
              tr("Not allowed to open downloaded EOS Tease in editor."));
      }
      else
      {
        emit SignalOpenClicked(iId);
      }
    }
  }

  if (bOk)
  {
    m_spUi->pStackedWidget->SlideInIdx(c_iPageIndexChoice);
  }
}
