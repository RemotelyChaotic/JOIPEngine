#include "EditorMainScreen.h"
#include "Application.h"
#include "EditorCodeWidget.h"
#include "EditorProjectSettingsWidget.h"
#include "EditorResourceDisplayWidget.h"
#include "EditorResourceWidget.h"
#include "EditorSceneNodeWidget.h"
#include "Systems/DatabaseManager.h"
#include "Systems/HelpFactory.h"
#include "Systems/Project.h"
#include "Tutorial/EditorTutorialOverlay.h"
#include "Tutorial/MainScreenTutorialStateSwitchHandler.h"
#include "Widgets/HelpOverlay.h"

#include <QAction>
#include <QFileInfo>
#include <QMessageBox>
#include <QPointer>

namespace
{
  const std::map<EEditorWidget, QString> m_sEditorNamesMap =
  {
    { EEditorWidget::eResourceWidget, "Resource Manager (%1)" },
    { EEditorWidget::eResourceDisplay, "Resource View (%1)" },
    { EEditorWidget::eProjectSettings, "Project Settings (%1)" },
    { EEditorWidget::eSceneNodeWidget, "Scene Node Editor (%1)" },
    { EEditorWidget::eSceneCodeEditorWidget, "Scene Code Editor (%1)" }
  };

  const std::map<EEditorWidget, QString> m_sEditorKeyBindingMap =
  {
    { EEditorWidget::eResourceWidget, "Resource" },
    { EEditorWidget::eResourceDisplay, "MediaPlayer" },
    { EEditorWidget::eProjectSettings, "Settings" },
    { EEditorWidget::eSceneNodeWidget, "Nodes" },
    { EEditorWidget::eSceneCodeEditorWidget, "Code" }
  };
  const std::map<CEditorActionBar::EActionBarPosition, QString> m_sSideKeyBindingMap =
  {
    { CEditorActionBar::eLeft, "LeftTab" },
    { CEditorActionBar::eRight, "RightTab" }
  };

  const char* c_sEditorProperty = "Editor";
  const char* c_sSideProperty = "Side";

  const QString c_sViewSelectorHelpId =  "Editor/ViewSelector";
}

//----------------------------------------------------------------------------------------
//
CEditorMainScreen::CEditorMainScreen(QWidget* pParent) :
  QWidget(pParent),
  m_spEditorModel(std::make_unique<CEditorModel>(this)),
  m_spUi(std::make_shared<Ui::CEditorMainScreen>()),
  m_spStateSwitchHandler(nullptr),
  m_vpKeyBindingActions(),
  m_pTutorialOverlay(new CEditorTutorialOverlay(this)),
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
  m_vpKeyBindingActions.clear();
}

//----------------------------------------------------------------------------------------
//
void CEditorMainScreen::Initialize()
{
  m_bInitialized = false;

  m_wpDbManager = CApplication::Instance()->System<CDatabaseManager>();

  // state switch handler
  m_spStateSwitchHandler =
      std::make_shared<CMainScreenTutorialStateSwitchHandler>(this, m_spUi, m_pTutorialOverlay);
  m_spEditorModel->AddTutorialStateSwitchHandler(m_spStateSwitchHandler);

  connect(m_spEditorModel.get(), &CEditorModel::SignalProjectEdited,
          this, &CEditorMainScreen::SlotProjectEdited);

  m_pTutorialOverlay->Initialize(m_spEditorModel.get());
  m_pTutorialOverlay->Hide();

  // action Bars
  m_spUi->pProjectActionBar->SetActionBarPosition(CEditorActionBar::eTop);
  m_spUi->pProjectActionBar->Initialize();
  m_spUi->pProjectActionBar->ShowProjectActionBar();
  m_spUi->pActionBarLeft->SetActionBarPosition(CEditorActionBar::eLeft);
  m_spUi->pActionBarLeft->Initialize();
  m_spUi->pActionBarRight->SetActionBarPosition(CEditorActionBar::eRight);
  m_spUi->pActionBarRight->Initialize();

  m_spUi->pProjectActionBar->m_spUi->ReadOnly->setVisible(false);

  connect(m_spUi->pProjectActionBar->m_spUi->pTitleLineEdit, &QLineEdit::editingFinished,
          this, &CEditorMainScreen::SlotProjectNameEditingFinished);
  connect(m_spUi->pProjectActionBar->m_spUi->SaveButton, &QPushButton::clicked,
          this, &CEditorMainScreen::SlotSaveClicked);
  connect(m_spUi->pProjectActionBar->m_spUi->ExportButton, &QPushButton::clicked,
          this, &CEditorMainScreen::SlotExportClicked);
  connect(m_spUi->pProjectActionBar->m_spUi->HelpButton, &QPushButton::clicked,
          this, &CEditorMainScreen::SlotHelpClicked);
  connect(m_spUi->pProjectActionBar->m_spUi->ExitButton, &QPushButton::clicked,
          this, &CEditorMainScreen::SlotExitClicked);

  // insert items in map
  m_spWidgetsMap.insert({EEditorWidget::eResourceWidget, new CEditorResourceWidget(this)});
  m_spWidgetsMap.insert({EEditorWidget::eResourceDisplay, new CEditorResourceDisplayWidget(this)});
  m_spWidgetsMap.insert({EEditorWidget::eProjectSettings, new CEditorProjectSettingsWidget(this)});
  m_spWidgetsMap.insert({EEditorWidget::eSceneNodeWidget, new CEditorSceneNodeWidget(this)});
  m_spWidgetsMap.insert({EEditorWidget::eSceneCodeEditorWidget, new CEditorCodeWidget(this)});

  // initialize action map
  for (qint32 i = CEditorActionBar::eLeft; CEditorActionBar::eRight+1 > i; ++i)
  {
    for (EEditorWidget eval : EEditorWidget::_values())
    {
      QAction* pAction = new QAction(this);
      pAction->setProperty(c_sEditorProperty, eval._to_integral());
      pAction->setProperty(c_sSideProperty, static_cast<qint32>(i));
      m_vpKeyBindingActions.push_back(pAction);
      addAction(pAction);
    }
  }


  // initialize widgets
  m_spUi->pLeftComboBox->blockSignals(true);
  m_spUi->pRightComboBox->blockSignals(true);
  for (auto it = m_spWidgetsMap.begin(); m_spWidgetsMap.end() != it; ++it)
  {
    connect(it->second, &CEditorWidgetBase::SignalProjectEdited,
            this, &CEditorMainScreen::SlotProjectEdited);
    connect(it->second, &CEditorWidgetBase::SignalUnloadFinished,
            this, &CEditorMainScreen::SlotUnloadFinished);

    it->second->SetEditorModel(m_spEditorModel.get());
    it->second->Initialize();
    it->second->setVisible(false);

    // Key-bindings werden später sowieso gesetzt
    m_spUi->pLeftComboBox->addItem(m_sEditorNamesMap.find(it->first)->second.arg(""),
                                   it->first._to_integral());
    m_spUi->pRightComboBox->addItem(m_sEditorNamesMap.find(it->first)->second.arg(""),
                                    it->first._to_integral());
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
  // db manager
  auto spDataBaseManager = m_wpDbManager.lock();
  if (nullptr != spDataBaseManager)
  {
    connect(spDataBaseManager.get(), &CDatabaseManager::SignalProjectRenamed,
            this, &CEditorMainScreen::SlotProjectRenamed, Qt::QueuedConnection);
  }

  // initializing done
  m_bInitialized = true;
}

//----------------------------------------------------------------------------------------
//
void CEditorMainScreen::InitNewProject(const QString& sNewProjectName, bool bTutorial)
{
  if (!m_bInitialized) { return; }

  m_spEditorModel->InitNewProject(sNewProjectName, bTutorial);
  m_spCurrentProject = m_spEditorModel->CurrentProject();

  ProjectLoaded(true);
}

//----------------------------------------------------------------------------------------
//
void CEditorMainScreen::LoadProject(qint32 iId)
{
  if (!m_bInitialized) { return; }

  m_spEditorModel->LoadProject(iId);
  m_spCurrentProject = m_spEditorModel->CurrentProject();

  ProjectLoaded(false);
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

  // disconnect shortcuts
  for (QAction* pAction : m_vpKeyBindingActions)
  {
    pAction->disconnect();
  }

  SetModificaitonFlag(false);

  m_spUi->pProjectActionBar->m_spUi->ReadOnly->setVisible(false);
  m_spUi->pProjectActionBar->m_spUi->pTitleLineEdit->setReadOnly(false);
  m_spUi->pProjectActionBar->m_spUi->SaveButton->setEnabled(true);
  m_spUi->pProjectActionBar->m_spUi->ExportButton->setEnabled(true);
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
    // get resource type
    auto spResource = spDbManager->FindResourceInProject(m_spCurrentProject, sName);
    EResourceType type = EResourceType::eOther;
    if (nullptr != spResource)
    {
      QReadLocker rcLocker(&spResource->m_rwLock);
      type = spResource->m_type;
    }

    // script selected?
    if (EResourceType::eScript == type._to_integral())
    {
      CEditorCodeWidget* pCodeWidget = GetWidget<CEditorCodeWidget>();
      pCodeWidget->LoadResource(spResource);
    }
    // normal resource, just show in resource viewer
    else
    {
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
void CEditorMainScreen::SlotExportClicked(bool bClick)
{
  Q_UNUSED(bClick);
  if (!m_bInitialized) { return; }
  SlotSaveClicked(bClick);
  m_spEditorModel->ExportProject();
}

//----------------------------------------------------------------------------------------
//
void CEditorMainScreen::SlotHelpClicked(bool bClick)
{
  Q_UNUSED(bClick)
  CHelpOverlay::Instance()->Show(
        m_spUi->pProjectActionBar->m_spUi->HelpButton->parentWidget()->mapFromGlobal(
          mapToGlobal(m_spUi->pProjectActionBar->m_spUi->HelpButton->geometry().center())),
        this);
}

//----------------------------------------------------------------------------------------
//
void CEditorMainScreen::SlotKeyBindingsChanged()
{
  auto spSettings = CApplication::Instance()->Settings();
  for (QAction* pAction : m_vpKeyBindingActions)
  {
    EEditorWidget widget =
        EEditorWidget::_from_integral(pAction->property(c_sEditorProperty).toInt());
    CEditorActionBar::EActionBarPosition position =
        static_cast<CEditorActionBar::EActionBarPosition>(pAction->property(c_sSideProperty).toInt());

    pAction->disconnect();

    auto itSide = m_sSideKeyBindingMap.find(position);
    auto itKey = m_sEditorKeyBindingMap.find(widget);
    if (m_sSideKeyBindingMap.end() != itSide && m_sEditorKeyBindingMap.end() != itKey)
    {
      qint32 iIndex = std::distance(m_spWidgetsMap.begin(), m_spWidgetsMap.find(widget));

      QKeySequence seq = spSettings->keyBinding(QString("%1_%2").arg(itSide->second).arg(itKey->second));
      pAction->setShortcut(seq);
      if (position == CEditorActionBar::eLeft)
      {
        m_spUi->pLeftComboBox->setItemText(iIndex, m_sEditorNamesMap.find(widget)->second.arg(seq.toString()));
        connect(pAction, &QAction::triggered, this, [this, iIndex](){
          m_spUi->pLeftComboBox->setCurrentIndex(iIndex);
        });
      }
      else if (position == CEditorActionBar::eRight)
      {
        m_spUi->pRightComboBox->setItemText(iIndex, m_sEditorNamesMap.find(widget)->second.arg(seq.toString()));
        connect(pAction, &QAction::triggered, this, [this, iIndex](){
          m_spUi->pRightComboBox->setCurrentIndex(iIndex);
        });
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorMainScreen::SlotProjectEdited()
{
  if (!m_bInitialized) { return; }

  SetModificaitonFlag(true);
  for (auto it = m_spWidgetsMap.begin(); m_spWidgetsMap.end() != it; ++it)
  {
    it->second->EditedProject();
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorMainScreen::SlotProjectExportStarted()
{
  m_spUi->pProjectActionBar->m_spUi->SaveButton->setEnabled(false);
  m_spUi->pProjectActionBar->m_spUi->ExportButton->setEnabled(false);
}

//----------------------------------------------------------------------------------------
//
void CEditorMainScreen::SlotProjectExportError(CEditorModel::EExportError error, const QString& sErrorString)
{
  QMessageBox msgBox;
  msgBox.setText("Export error.");

  switch (error)
  {
    case CEditorModel::EExportError::eWriteFailed:
    msgBox.setInformativeText(sErrorString + "\n"
                              + tr("Please move the data directory to a writable location and try again."));
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    break;

    case CEditorModel::EExportError::eProcessError:
    msgBox.setInformativeText(sErrorString + "\n");
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    break;

    case CEditorModel::EExportError::eCleanupFailed:
    msgBox.setInformativeText(sErrorString + "\n"
                              + tr("Please delete the file manually."));
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    break;

    default:
    msgBox.setInformativeText(sErrorString);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    break;
  }

  msgBox.setModal(true);
  msgBox.setWindowFlag(Qt::FramelessWindowHint);

  QPointer<CEditorMainScreen> pMeMyselfMyPointerAndI(this);
  qint32 iRet = msgBox.exec();
  if (nullptr == pMeMyselfMyPointerAndI)
  {
    return;
  }

  switch (iRet) {
    case QMessageBox::Ok:
        break;
    default:
        break;
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorMainScreen::SlotProjectExportFinished()
{
  m_spUi->pProjectActionBar->m_spUi->SaveButton->setEnabled(true);
  m_spUi->pProjectActionBar->m_spUi->ExportButton->setEnabled(true);
}

//----------------------------------------------------------------------------------------
//
void CEditorMainScreen::SlotProjectNameEditingFinished()
{
  if (!m_bInitialized) { return; }

  QString sNewName = m_spUi->pProjectActionBar->m_spUi->pTitleLineEdit->text();
  const QString sFinalName = m_spEditorModel->RenameProject(sNewName);
  m_spUi->pProjectActionBar->m_spUi->pTitleLineEdit->blockSignals(true);
  m_spUi->pProjectActionBar->m_spUi->pTitleLineEdit->setText(sFinalName);
  m_spUi->pProjectActionBar->m_spUi->pTitleLineEdit->blockSignals(false);
}

//----------------------------------------------------------------------------------------
//
void CEditorMainScreen::SlotProjectRenamed(qint32 iId)
{
  if (!m_bInitialized) { return; }
  if (nullptr == m_spCurrentProject) { return; }

  QReadLocker locker(&m_spCurrentProject->m_rwLock);
  qint32 iThisId = m_spCurrentProject->m_iId;
  QString sName = m_spCurrentProject->m_sName;
  locker.unlock();

  if (iId == iThisId)
  {
    m_spUi->pProjectActionBar->m_spUi->pTitleLineEdit->blockSignals(true);
    m_spUi->pProjectActionBar->m_spUi->pTitleLineEdit->setText(sName);
    m_spUi->pProjectActionBar->m_spUi->pTitleLineEdit->blockSignals(false);
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
void CEditorMainScreen::SlotUnloadFinished()
{
  bool bAllUnloaded = true;
  for (auto it = m_spWidgetsMap.begin(); m_spWidgetsMap.end() != it; ++it)
  {
    bAllUnloaded &= !it->second->IsLoaded();
  }
  if (bAllUnloaded)
  {
    emit SignalUnloadFinished();
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
    pLayout->addWidget(it->second);
    it->second->SetActionBar(pActionBar);
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorMainScreen::ProjectLoaded(bool bNewProject)
{
  // init indicees
  m_spUi->pRightComboBox->blockSignals(true);
  m_spUi->pRightComboBox->setCurrentIndex(EEditorWidget::eProjectSettings);
  on_pRightComboBox_currentIndexChanged(EEditorWidget::eProjectSettings);
  m_spUi->pRightComboBox->blockSignals(false);

  m_spUi->pLeftComboBox->blockSignals(true);
  m_spUi->pLeftComboBox->setCurrentIndex(EEditorWidget::eResourceWidget);
  on_pLeftComboBox_currentIndexChanged(EEditorWidget::eResourceWidget);
  m_spUi->pLeftComboBox->blockSignals(false);

  SlotKeyBindingsChanged();

  if (nullptr != m_spCurrentProject)
  {
    QReadLocker locker(&m_spCurrentProject->m_rwLock);
    m_spUi->pProjectActionBar->m_spUi->pTitleLineEdit->setText(m_spCurrentProject->m_sName);
  }

  for (auto it = m_spWidgetsMap.begin(); m_spWidgetsMap.end() != it; ++it)
  {
    it->second->LoadProject(m_spCurrentProject);
  }

  if (bNewProject)
  {
    SlotSaveClicked(true);
  }

  SetModificaitonFlag(false);

  // reload action bars
  ChangeIndex(m_spUi->pRightComboBox, m_spUi->pRightContainer, m_spUi->pActionBarRight, 2);
  ChangeIndex(m_spUi->pLeftComboBox, m_spUi->pLeftContainer, m_spUi->pActionBarLeft, 0);

  m_spUi->splitter->setSizes({ width() * 1/3 , width() * 2/3 });

  // read only
  if (m_spEditorModel->IsReadOnly())
  {
    m_spUi->pProjectActionBar->m_spUi->ReadOnly->setVisible(true);
    m_spUi->pProjectActionBar->m_spUi->pTitleLineEdit->setReadOnly(true);
    m_spUi->pProjectActionBar->m_spUi->SaveButton->setEnabled(false);
    m_spUi->pProjectActionBar->m_spUi->ExportButton->setEnabled(false);
  }
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

template<> CEditorProjectSettingsWidget* CEditorMainScreen::GetWidget<CEditorProjectSettingsWidget>()
{
  auto it = m_spWidgetsMap.find(EEditorWidget::eProjectSettings);
  if (m_spWidgetsMap.end() != it)
  {
    CEditorProjectSettingsWidget* pWidget =
        dynamic_cast<CEditorProjectSettingsWidget*>(it->second.data());
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
