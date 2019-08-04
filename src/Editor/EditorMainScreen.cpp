#include "EditorMainScreen.h"
#include "Application.h"
#include "EditorResourceDisplayWidget.h"
#include "EditorResourceWidget.h"
#include "Backend/DatabaseManager.h"
#include "Backend/Project.h"
#include "ui_EditorMainScreen.h"

namespace
{
  const std::map<EEditorWidget, QString> m_sEditorNamesMap =
  {
    { EEditorWidget::eResourceWidget, "Resource Manager" },
    { EEditorWidget::eResourceDisplay, "Resource View" }
  };
}

//----------------------------------------------------------------------------------------
//
CEditorMainScreen::CEditorMainScreen(QWidget* pParent) :
  QWidget(pParent),
  m_spUi(std::make_unique<Ui::CEditorMainScreen>()),
  m_spWidgetsMap(),
  m_spCurrentProject(nullptr),
  m_wpDbManager(),
  m_bInitialized(false)
{
  m_spUi->setupUi(this);
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

  m_spUi->pActionBar->Initialize();

  // insert items in map
  m_spWidgetsMap.insert({EEditorWidget::eResourceWidget, std::make_unique<CEditorResourceWidget>(nullptr, m_spUi->pActionBar)});
  m_spWidgetsMap.insert({EEditorWidget::eResourceDisplay, std::make_unique<CEditorResourceDisplayWidget>()});

  // initialize widgets
  m_spUi->pLeftComboBox->blockSignals(true);
  m_spUi->pRightComboBox->blockSignals(true);
  for (auto it = m_spWidgetsMap.begin(); m_spWidgetsMap.end() != it; ++it)
  {
    it->second->Initialize();
    m_spUi->pLeftComboBox->addItem(m_sEditorNamesMap.find(it->first)->second, it->first._to_integral());
    m_spUi->pRightComboBox->addItem(m_sEditorNamesMap.find(it->first)->second, it->first._to_integral());
  }
  m_spUi->pLeftComboBox->blockSignals(false);
  m_spUi->pRightComboBox->blockSignals(false);

  // initializing done
  m_bInitialized = true;

  // init indicees
  m_spUi->pLeftComboBox->setCurrentIndex(0);
  on_pLeftComboBox_currentIndexChanged(0);
  m_spUi->pRightComboBox->setCurrentIndex(1);
  on_pRightComboBox_currentIndexChanged(1);
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

    // save to create folder structure
    spDbManager->SerializeProject(iId);
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorMainScreen::LoadProject(qint32 iId)
{
  if (!m_bInitialized) { return; }
  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    m_spCurrentProject = spDbManager->FindProject(iId);
  }

  CEditorResourceWidget* pWidget = GetWidget<CEditorResourceWidget>();
  if (nullptr != pWidget)
  {
    pWidget->LoadProject(m_spCurrentProject);
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorMainScreen::UnloadProject()
{
  if (!m_bInitialized) { return; }

  m_spCurrentProject = nullptr;

  CEditorResourceWidget* pWidget = GetWidget<CEditorResourceWidget>();
  if (nullptr != pWidget)
  {
    pWidget->UnloadProject();
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorMainScreen::on_pLeftComboBox_currentIndexChanged(qint32 iIndex)
{
  if (!m_bInitialized) { return; }

  qint32 iEnumValue = m_spUi->pLeftComboBox->itemData(iIndex, Qt::UserRole).toInt();
  QLayout* pLayout = m_spUi->pLeftContainer->layout();
  while (auto item = pLayout->takeAt(0));

  auto it = m_spWidgetsMap.find(EEditorWidget::_from_integral(iEnumValue));
  if (m_spWidgetsMap.end() != it)
  {
    pLayout->addWidget(it->second.get());
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorMainScreen::on_pRightComboBox_currentIndexChanged(qint32 iIndex)
{
  if (!m_bInitialized) { return; }

  qint32 iEnumValue = m_spUi->pRightComboBox->itemData(iIndex, Qt::UserRole).toInt();
  QLayout* pLayout = m_spUi->pRightContainer->layout();
  while (auto item = pLayout->takeAt(0));

  auto it = m_spWidgetsMap.find(EEditorWidget::_from_integral(iEnumValue));
  if (m_spWidgetsMap.end() != it)
  {
    pLayout->addWidget(it->second.get());
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
