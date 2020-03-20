#include "EditorChoiceScreen.h"
#include "Application.h"
#include "Systems/DatabaseManager.h"
#include "ui_EditorChoiceScreen.h"

namespace
{
  const qint32 c_iPageIndexChoice = 0;
  const qint32 c_iPageIndexNew    = 1;
  const qint32 c_iPageIndexOpen   = 2;
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

  m_spUi->pStackedWidget->setCurrentIndex(c_iPageIndexChoice);
  m_spUi->pErrorLabel->setVisible(false);

  m_bInitialized = true;
}

//----------------------------------------------------------------------------------------
//
void CEditorChoiceScreen::Load()
{
  m_spUi->pProjectCardSelectionWidget->UnloadProjects();
  m_spUi->pProjectCardSelectionWidget->LoadProjects();
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
  m_spUi->pStackedWidget->setCurrentIndex(c_iPageIndexNew);
}

//----------------------------------------------------------------------------------------
//
void CEditorChoiceScreen::on_pOpenProjectButton_clicked()
{
  if (!m_bInitialized) { return; }
  m_spUi->pStackedWidget->setCurrentIndex(c_iPageIndexOpen);
}

//----------------------------------------------------------------------------------------
//
void CEditorChoiceScreen::on_pCancelButton_clicked()
{
  if (!m_bInitialized) { return; }

  emit SignalCancelClicked();
  m_spUi->pStackedWidget->setCurrentIndex(c_iPageIndexChoice);
}

//----------------------------------------------------------------------------------------
//
void CEditorChoiceScreen::on_pProjectNameLineEdit_editingFinished()
{
  if (!m_bInitialized) { return; }

  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    const QString sNewName = m_spUi->pProjectNameLineEdit->text();
    if (sNewName.isEmpty() || sNewName.isNull())
    {
      m_spUi->pErrorLabel->setText(QString("Cannot create a Project with an empty name."));
      m_spUi->pErrorLabel->setVisible(true);
    }
    else
    {
      if (spDbManager->FindProject(sNewName) != nullptr)
      {
        m_spUi->pErrorLabel->setText(QString("Project with the name '%1' allready exists.\n"
                                             "Please chose a different name.").arg(sNewName));
        m_spUi->pErrorLabel->setVisible(true);
      }
      else
      {
        m_spUi->pErrorLabel->setVisible(false);
      }
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
    const QString sNewName = m_spUi->pProjectNameLineEdit->text();
    if (spDbManager->FindProject(sNewName) == nullptr)
    {
      emit SignalNewClicked(sNewName);
    }
  }

  m_spUi->pStackedWidget->setCurrentIndex(c_iPageIndexChoice);
}

//----------------------------------------------------------------------------------------
//
void CEditorChoiceScreen::on_pOpenExistingProjectButton_clicked()
{
  if (!m_bInitialized) { return; }
  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    qint32 iId = m_spUi->pProjectCardSelectionWidget->SelectedId();
    if (spDbManager->FindProject(iId) != nullptr)
    {
      emit SignalOpenClicked(iId);
    }
  }

  m_spUi->pStackedWidget->setCurrentIndex(c_iPageIndexChoice);
}
