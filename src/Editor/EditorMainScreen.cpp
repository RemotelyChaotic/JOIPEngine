#include "EditorMainScreen.h"
#include "Application.h"
#include "EditorCodeWidget.h"
#include "EditorModel.h"
#include "EditorResourceDisplayWidget.h"
#include "EditorResourceWidget.h"
#include "EditorSceneNodeWidget.h"
#include "Systems/DatabaseManager.h"
#include "Systems/HelpFactory.h"
#include "Systems/Project.h"
#include "Widgets/HelpOverlay.h"
#include "ui_EditorMainScreen.h"
#include "ui_EditorActionBar.h"

#include <QFileInfo>
#include <QMessageBox>
#include <QPointer>

namespace
{
  const std::map<EEditorWidget, QString> m_sEditorNamesMap =
  {
    { EEditorWidget::eResourceWidget, "Resource Manager" },
    { EEditorWidget::eResourceDisplay, "Resource View" },
    { EEditorWidget::eSceneNodeWidget, "Scene Node Editor" },
    { EEditorWidget::eSceneCodeEditorWidget, "Scene Code Editor" }
  };

  const QString c_sViewSelectorHelpId =  "Editor/ViewSelector";
}

//----------------------------------------------------------------------------------------
//
CEditorMainScreen::CEditorMainScreen(QWidget* pParent) :
  QWidget(pParent),
  m_spUi(std::make_unique<Ui::CEditorMainScreen>()),
  m_spEditorModel(std::make_unique<CEditorModel>(this)),
  m_spWidgetsMap(),
  m_spCurrentProject(nullptr),
  m_wpDbManager(),
  m_bInitialized(false),
  m_bProjectModified(false),
  m_iLastLeftIndex(-1),
  m_iLastRightIndex(-2)
{
  m_spUi->setupUi(this);

  // Testing
  //new ModelTest(pModel, this);

  Initialize();
}

CEditorMainScreen::~CEditorMainScreen()
{
}

//----------------------------------------------------------------------------------------
//
void CEditorMainScreen::Initialize()
{
  m_bInitialized = false;

  m_wpDbManager = CApplication::Instance()->System<CDatabaseManager>();

  connect(m_spEditorModel.get(), &CEditorModel::SignalProjectEdited,
          this, &CEditorMainScreen::SlotProjectEdited);

  // action Bars
  m_spUi->pProjectActionBar->Initialize();
  m_spUi->pProjectActionBar->ShowProjectActionBar();
  m_spUi->pActionBarLeft->Initialize();
  m_spUi->pActionBarRight->Initialize();

  connect(m_spUi->pProjectActionBar->m_spUi->pTitleLineEdit, &QLineEdit::editingFinished,
          this, &CEditorMainScreen::SlotProjectNameEditingFinished);
  connect(m_spUi->pProjectActionBar->m_spUi->SaveButton, &QPushButton::clicked,
          this, &CEditorMainScreen::SlotSaveClicked);
  connect(m_spUi->pProjectActionBar->m_spUi->HelpButton, &QPushButton::clicked,
          this, &CEditorMainScreen::SlotHelpClicked);
  connect(m_spUi->pProjectActionBar->m_spUi->ExitButton, &QPushButton::clicked,
          this, &CEditorMainScreen::SlotExitClicked);

  // insert items in map
  m_spWidgetsMap.insert({EEditorWidget::eResourceWidget, new CEditorResourceWidget(this)});
  m_spWidgetsMap.insert({EEditorWidget::eResourceDisplay, new CEditorResourceDisplayWidget(this)});
  m_spWidgetsMap.insert({EEditorWidget::eSceneNodeWidget, new CEditorSceneNodeWidget(this)});
  m_spWidgetsMap.insert({EEditorWidget::eSceneCodeEditorWidget, new CEditorCodeWidget(this)});

  // initialize widgets
  m_spUi->pLeftComboBox->blockSignals(true);
  m_spUi->pRightComboBox->blockSignals(true);
  for (auto it = m_spWidgetsMap.begin(); m_spWidgetsMap.end() != it; ++it)
  {
    connect(it->second, &CEditorWidgetBase::SignalProjectEdited,
            this, &CEditorMainScreen::SlotProjectEdited);

    it->second->SetEditorModel(m_spEditorModel.get());
    it->second->Initialize();
    it->second->setVisible(false);
    m_spUi->pLeftComboBox->addItem(m_sEditorNamesMap.find(it->first)->second, it->first._to_integral());
    m_spUi->pRightComboBox->addItem(m_sEditorNamesMap.find(it->first)->second, it->first._to_integral());
  }
  m_spUi->pLeftComboBox->blockSignals(false);
  m_spUi->pRightComboBox->blockSignals(false);

  // custom stuff
  connect(GetWidget<CEditorResourceWidget>(), &CEditorResourceWidget::SignalResourceSelected,
          this, &CEditorMainScreen::SlotDisplayResource);

  // help
  auto wpHelpFactory = CApplication::Instance()->System<CHelpFactory>().lock();
  if (nullptr != wpHelpFactory)
  {
    m_spUi->pLeftComboBox->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sViewSelectorHelpId);
    wpHelpFactory->RegisterHelp(c_sViewSelectorHelpId, ":/resources/help/editor/selection_combobox_help.html");
    m_spUi->pRightComboBox->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sViewSelectorHelpId);
  }

  // initializing done
  m_bInitialized = true;

  // init indicees
  m_spUi->pRightComboBox->blockSignals(true);
  m_spUi->pRightComboBox->setCurrentIndex(1);
  on_pRightComboBox_currentIndexChanged(1);
  m_spUi->pRightComboBox->blockSignals(false);

  m_spUi->pLeftComboBox->blockSignals(true);
  m_spUi->pLeftComboBox->setCurrentIndex(0);
  on_pLeftComboBox_currentIndexChanged(0);
  m_spUi->pLeftComboBox->blockSignals(false);
}

//----------------------------------------------------------------------------------------
//
void CEditorMainScreen::InitNewProject(const QString& sNewProjectName)
{
  if (!m_bInitialized) { return; }

  m_spEditorModel->InitNewProject(sNewProjectName);
  m_spCurrentProject = m_spEditorModel->CurrentProject();

  ProjectLoaded();
}

//----------------------------------------------------------------------------------------
//
void CEditorMainScreen::LoadProject(qint32 iId)
{
  if (!m_bInitialized) { return; }

  m_spEditorModel->LoadProject(iId);
  m_spCurrentProject = m_spEditorModel->CurrentProject();

  ProjectLoaded();
}

//----------------------------------------------------------------------------------------
//
void CEditorMainScreen::UnloadProject()
{
  if (!m_bInitialized) { return; }

  for (auto it = m_spWidgetsMap.begin(); m_spWidgetsMap.end() != it; ++it)
  {
    it->second->UnloadProject();
  }

  m_spCurrentProject = nullptr;
  m_spEditorModel->UnloadProject();

  SetModificaitonFlag(false);
}

//----------------------------------------------------------------------------------------
//
void CEditorMainScreen::on_pLeftComboBox_currentIndexChanged(qint32 iIndex)
{
  if (!m_bInitialized) { return; }
  if (m_iLastLeftIndex == iIndex) { return; }

  ChangeIndex(m_spUi->pLeftComboBox, m_spUi->pLeftContainer, m_spUi->pActionBarLeft, iIndex);

  if (iIndex == m_spUi->pRightComboBox->currentIndex())
  {
    m_spUi->pRightComboBox->blockSignals(true);
    m_spUi->pRightComboBox->setCurrentIndex(m_iLastLeftIndex);
    ChangeIndex(m_spUi->pRightComboBox, m_spUi->pRightContainer, m_spUi->pActionBarRight, m_iLastLeftIndex);
    m_iLastRightIndex = m_iLastLeftIndex;
    m_spUi->pRightComboBox->blockSignals(false);
  }
  m_iLastLeftIndex = iIndex;
}

//----------------------------------------------------------------------------------------
//
void CEditorMainScreen::on_pRightComboBox_currentIndexChanged(qint32 iIndex)
{
  if (!m_bInitialized) { return; }
  if (m_iLastRightIndex == iIndex) { return; }

  ChangeIndex(m_spUi->pRightComboBox, m_spUi->pRightContainer, m_spUi->pActionBarRight, iIndex);

  if (iIndex == m_spUi->pLeftComboBox->currentIndex())
  {
    m_spUi->pLeftComboBox->blockSignals(true);
    m_spUi->pLeftComboBox->setCurrentIndex(m_iLastRightIndex);
    ChangeIndex(m_spUi->pLeftComboBox, m_spUi->pLeftContainer, m_spUi->pActionBarLeft, m_iLastRightIndex);
    m_iLastLeftIndex = m_iLastRightIndex;
    m_spUi->pLeftComboBox->blockSignals(false);
  }
  m_iLastRightIndex = iIndex;
}

//----------------------------------------------------------------------------------------
//
void CEditorMainScreen::SlotDisplayResource(const QString& sName)
{
  if (!m_bInitialized && nullptr != m_spCurrentProject) { return; }

  CEditorResourceDisplayWidget* pWidget = GetWidget<CEditorResourceDisplayWidget>();
  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager && nullptr != pWidget)
  {
    QReadLocker locker(&m_spCurrentProject->m_rwLock);

    // script selected?
    if (QFileInfo(sName).suffix() == "js")
    {
      locker.unlock();
      auto spResource = spDbManager->FindResource(m_spCurrentProject, sName);
      pWidget->UnloadResource();
      CEditorCodeWidget* pCodeWidget = GetWidget<CEditorCodeWidget>();
      pCodeWidget->LoadResource(spResource);
    }
    // normal resource, just show in resource viewer
    else
    {
      locker.unlock();
      auto spResource = spDbManager->FindResource(m_spCurrentProject, sName);
      pWidget->UnloadResource();
      pWidget->LoadResource(spResource);
      if (m_spUi->pRightComboBox->itemData(m_spUi->pRightComboBox->currentIndex(), Qt::UserRole).toInt() == EEditorWidget::eResourceDisplay ||
          m_spUi->pLeftComboBox->itemData(m_spUi->pLeftComboBox->currentIndex(), Qt::UserRole).toInt() == EEditorWidget::eResourceDisplay)
      {
        pWidget->UpdateActionBar();
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorMainScreen::SlotExitClicked(bool bClick)
{
  Q_UNUSED(bClick);
  if (!m_bInitialized) { return; }

  if (m_bProjectModified)
  {
    QMessageBox msgBox;
    msgBox.setText("The project has been modified.");
    msgBox.setInformativeText("Do you want to save your changes?");
    msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Save);
    msgBox.setModal(true);
    msgBox.setWindowFlag(Qt::FramelessWindowHint);

    QPointer<CEditorMainScreen> pMeMyselfMyPointerAndI(this);
    qint32 iRet = msgBox.exec();
    if (nullptr == pMeMyselfMyPointerAndI)
    {
      return;
    }

    switch (iRet) {
      case QMessageBox::Save:
          SlotSaveClicked(true);
          break;
      case QMessageBox::Cancel:
          return;
      case QMessageBox::Discard: // fallthrough
      default:
          break;
    }
  }

  SetModificaitonFlag(false);
  UnloadProject();
  emit SignalExitClicked();
}

//----------------------------------------------------------------------------------------
//
void CEditorMainScreen::SlotHelpClicked(bool bClick)
{
  Q_UNUSED(bClick)
  CHelpOverlay::Instance()->Show(
        mapToGlobal(m_spUi->pProjectActionBar->m_spUi->HelpButton->geometry().center()),
        this);
}

//----------------------------------------------------------------------------------------
//
void CEditorMainScreen::SlotProjectEdited()
{
  if (!m_bInitialized) { return; }

  SetModificaitonFlag(true);
}

//----------------------------------------------------------------------------------------
//
void CEditorMainScreen::SlotProjectNameEditingFinished()
{
  if (!m_bInitialized) { return; }

  QString sNewName = m_spUi->pProjectActionBar->m_spUi->pTitleLineEdit->text();
  const QString sFinalName = m_spEditorModel->RenameProject(sNewName);
  m_spUi->pProjectActionBar->m_spUi->pTitleLineEdit->setText(sFinalName);

  if (sFinalName == sNewName)
  {
    SlotProjectEdited();
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorMainScreen::SlotSaveClicked(bool bClick)
{
  Q_UNUSED(bClick);
  if (!m_bInitialized) { return; }

  m_spEditorModel->SaveProject();

  for (auto it = m_spWidgetsMap.begin(); m_spWidgetsMap.end() != it; ++it)
  {
    it->second->SaveProject();
  }

  m_spEditorModel->SerializeProject();

  SetModificaitonFlag(false);
}

//----------------------------------------------------------------------------------------
//
void CEditorMainScreen::ChangeIndex(QComboBox* pComboBox, QWidget* pContainer,
  CEditorActionBar* pActionBar, qint32 iIndex)
{
  qint32 iEnumValue = pComboBox->itemData(iIndex, Qt::UserRole).toInt();
  QLayout* pLayout = pContainer->layout();
  while (auto item = pLayout->takeAt(0))
  {
    item->widget()->setVisible(false);
  }

  auto it = m_spWidgetsMap.find(EEditorWidget::_from_integral(iEnumValue));
  if (m_spWidgetsMap.end() != it)
  {
    it->second->setVisible(true);
    pLayout->addWidget(it->second);
    it->second->SetActionBar(pActionBar);
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorMainScreen::ProjectLoaded()
{
  if (nullptr != m_spCurrentProject)
  {
    QReadLocker locker(&m_spCurrentProject->m_rwLock);
    m_spUi->pProjectActionBar->m_spUi->pTitleLineEdit->setText(m_spCurrentProject->m_sName);
  }

  for (auto it = m_spWidgetsMap.begin(); m_spWidgetsMap.end() != it; ++it)
  {
    it->second->LoadProject(m_spCurrentProject);
  }

  SlotSaveClicked(true);
  SetModificaitonFlag(false);

  m_spUi->splitter->setSizes({ width() * 1/3 , width() * 2/3 });
}

//----------------------------------------------------------------------------------------
//
void CEditorMainScreen::SetModificaitonFlag(bool bModified)
{
  if (m_bProjectModified != bModified)
  {
    m_bProjectModified = bModified;
    m_spUi->pProjectActionBar->m_spUi->pProjectLabel->setText(QString(tr("Project")) + (bModified ? " *" : ""));
  }
}

//----------------------------------------------------------------------------------------
//
template<> CEditorResourceWidget* CEditorMainScreen::GetWidget<CEditorResourceWidget>()
{
  auto it = m_spWidgetsMap.find(EEditorWidget::eResourceWidget);
  if (m_spWidgetsMap.end() != it)
  {
    CEditorResourceWidget* pWidget =
        dynamic_cast<CEditorResourceWidget*>(it->second.data());
    return pWidget;
  }
  return nullptr;
}

template<> CEditorResourceDisplayWidget* CEditorMainScreen::GetWidget<CEditorResourceDisplayWidget>()
{
  auto it = m_spWidgetsMap.find(EEditorWidget::eResourceDisplay);
  if (m_spWidgetsMap.end() != it)
  {
    CEditorResourceDisplayWidget* pWidget =
        dynamic_cast<CEditorResourceDisplayWidget*>(it->second.data());
    return pWidget;
  }
  return nullptr;
}

template<> CEditorSceneNodeWidget* CEditorMainScreen::GetWidget<CEditorSceneNodeWidget>()
{
  auto it = m_spWidgetsMap.find(EEditorWidget::eSceneNodeWidget);
  if (m_spWidgetsMap.end() != it)
  {
    CEditorSceneNodeWidget* pWidget =
        dynamic_cast<CEditorSceneNodeWidget*>(it->second.data());
    return pWidget;
  }
  return nullptr;
}

template<> CEditorCodeWidget* CEditorMainScreen::GetWidget<CEditorCodeWidget>()
{
  auto it = m_spWidgetsMap.find(EEditorWidget::eSceneCodeEditorWidget);
  if (m_spWidgetsMap.end() != it)
  {
    CEditorCodeWidget* pWidget =
        dynamic_cast<CEditorCodeWidget*>(it->second.data());
    return pWidget;
  }
  return nullptr;
}
