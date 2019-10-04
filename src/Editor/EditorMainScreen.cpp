#include "EditorMainScreen.h"
#include "Application.h"
#include "EditorCodeWidget.h"
#include "EditorResourceDisplayWidget.h"
#include "EditorResourceWidget.h"
#include "EditorSceneNodeWidget.h"
#include "Backend/DatabaseManager.h"
#include "Backend/Project.h"
#include "Resources/ResourceTreeItemModel.h"
#include "ui_EditorMainScreen.h"
#include "ui_EditorActionBar.h"

#include <QFileInfo>

namespace
{
  const std::map<EEditorWidget, QString> m_sEditorNamesMap =
  {
    { EEditorWidget::eResourceWidget, "Resource Manager" },
    { EEditorWidget::eResourceDisplay, "Resource View" },
    { EEditorWidget::eSceneNodeWidget, "Scene Node Editor" },
    { EEditorWidget::eSceneCodeEditorWidget, "Scene Code Editor" }
  };
}

//----------------------------------------------------------------------------------------
//
CEditorMainScreen::CEditorMainScreen(QWidget* pParent) :
  QWidget(pParent),
  m_spUi(std::make_unique<Ui::CEditorMainScreen>()),
  m_spResourceTreeModel(std::make_unique<CResourceTreeItemModel>()),
  m_spWidgetsMap(),
  m_spCurrentProject(nullptr),
  m_wpDbManager(),
  m_bInitialized(false),
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

  // action Bars
  m_spUi->pProjectActionBar->Initialize();
  m_spUi->pProjectActionBar->ShowProjectActionBar();
  m_spUi->pActionBarLeft->Initialize();
  m_spUi->pActionBarRight->Initialize();

  connect(m_spUi->pProjectActionBar->m_spUi->pSaveButton, &QPushButton::clicked,
          this, &CEditorMainScreen::SlotSaveClicked);
  connect(m_spUi->pProjectActionBar->m_spUi->pTitleLineEdit, &QLineEdit::editingFinished,
          this, &CEditorMainScreen::SlotProjectNameEditingFinished);
  connect(m_spUi->pProjectActionBar->m_spUi->pExitButton, &QPushButton::clicked,
          this, &CEditorMainScreen::SlotExitClicked);

  // insert items in map
  m_spWidgetsMap.insert({EEditorWidget::eResourceWidget, std::make_unique<CEditorResourceWidget>()});
  m_spWidgetsMap.insert({EEditorWidget::eResourceDisplay, std::make_unique<CEditorResourceDisplayWidget>()});
  m_spWidgetsMap.insert({EEditorWidget::eSceneNodeWidget, std::make_unique<CEditorSceneNodeWidget>()});
  m_spWidgetsMap.insert({EEditorWidget::eSceneCodeEditorWidget, std::make_unique<CEditorCodeWidget>()});

  // initialize widgets
  m_spUi->pLeftComboBox->blockSignals(true);
  m_spUi->pRightComboBox->blockSignals(true);
  for (auto it = m_spWidgetsMap.begin(); m_spWidgetsMap.end() != it; ++it)
  {
    it->second->SetResourceModel(m_spResourceTreeModel.get());
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
  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    spDbManager->AddProject(sNewProjectName);
    m_spCurrentProject = spDbManager->FindProject(sNewProjectName);

    m_spCurrentProject->m_rwLock.lockForRead();
    qint32 iId = m_spCurrentProject->m_iId;
    m_spCurrentProject->m_rwLock.unlock();
    LoadProject(iId);
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorMainScreen::LoadProject(qint32 iId)
{
  if (!m_bInitialized) { return; }
  if (nullptr != m_spCurrentProject)
  {
    qWarning() << "Old Project was not unloaded before loading project.";
  }

  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    m_spCurrentProject = spDbManager->FindProject(iId);
    if (nullptr != m_spCurrentProject)
    {
      QReadLocker locker(&m_spCurrentProject->m_rwLock);
      m_spUi->pProjectActionBar->m_spUi->pTitleLineEdit->setText(m_spCurrentProject->m_sName);
    }
  }

  m_spResourceTreeModel->InitializeModel(m_spCurrentProject);

  for (auto it = m_spWidgetsMap.begin(); m_spWidgetsMap.end() != it; ++it)
  {
    it->second->LoadProject(m_spCurrentProject);
  }

  SlotSaveClicked(true);

  m_spUi->splitter->setSizes({ width() * 1/3 , width() * 2/3 });
}

//----------------------------------------------------------------------------------------
//
void CEditorMainScreen::UnloadProject()
{
  if (!m_bInitialized) { return; }

  // reset to what is in the database
  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager && nullptr != m_spCurrentProject)
  {
    m_spCurrentProject->m_rwLock.lockForRead();
    qint32 iId = m_spCurrentProject->m_iId;
    m_spCurrentProject->m_rwLock.unlock();

    spDbManager->DeserializeProject(iId);
  }

  m_spCurrentProject = nullptr;

  for (auto it = m_spWidgetsMap.begin(); m_spWidgetsMap.end() != it; ++it)
  {
    it->second->UnloadProject();
  }

  m_spResourceTreeModel->DeInitializeModel();
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

  UnloadProject();
  emit SignalExitClicked();
}

//----------------------------------------------------------------------------------------
//
void CEditorMainScreen::SlotProjectNameEditingFinished()
{
  if (!m_bInitialized && nullptr != m_spCurrentProject) { return; }

  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    m_spCurrentProject->m_rwLock.lockForRead();
    qint32 iId = m_spCurrentProject->m_iId;
    m_spCurrentProject->m_rwLock.unlock();

    spDbManager->RenameProject(iId, m_spUi->pProjectActionBar->m_spUi->pTitleLineEdit->text());

    m_spCurrentProject->m_rwLock.lockForRead();
    const QString sName = m_spCurrentProject->m_sName;
    m_spCurrentProject->m_rwLock.unlock();

    m_spUi->pProjectActionBar->m_spUi->pTitleLineEdit->setText(sName);
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorMainScreen::SlotSaveClicked(bool bClick)
{
  Q_UNUSED(bClick);
  if (!m_bInitialized && nullptr != m_spCurrentProject) { return; }

  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    m_spCurrentProject->m_rwLock.lockForRead();
    qint32 iId = m_spCurrentProject->m_iId;
    m_spCurrentProject->m_rwLock.unlock();

    for (auto it = m_spWidgetsMap.begin(); m_spWidgetsMap.end() != it; ++it)
    {
      it->second->SaveProject();
    }

    // save to create folder structure
    spDbManager->SerializeProject(iId);
  }
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
    pLayout->addWidget(it->second.get());
    it->second->SetActionBar(pActionBar);
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
        dynamic_cast<CEditorResourceWidget*>(it->second.get());
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
        dynamic_cast<CEditorResourceDisplayWidget*>(it->second.get());
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
        dynamic_cast<CEditorSceneNodeWidget*>(it->second.get());
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
        dynamic_cast<CEditorCodeWidget*>(it->second.get());
    return pWidget;
  }
  return nullptr;
}
