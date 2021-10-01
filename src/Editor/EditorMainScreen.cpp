#include "EditorMainScreen.h"
#include "Application.h"
#include "EditorWidgetRegistry.h"
#include "Settings.h"
#include "EditorLayouts/EditorLayoutBase.h"
#include "EditorLayouts/IEditorLayoutViewProvider.h"
#include "EditorWidgets/EditorWidgetBase.h"
#include "Systems/DatabaseManager.h"
#include "Systems/Project.h"
#include "Tutorial/EditorTutorialOverlay.h"
#include "Tutorial/MainScreenTutorialStateSwitchHandler.h"
#include "Widgets/HelpOverlay.h"

#include <QAction>
#include <QDebug>
#include <QFileInfo>
#include <QMessageBox>
#include <QPointer>
#include <QUndoStack>

//----------------------------------------------------------------------------------------
//
class CEditorLayoutViewProvider : public IEditorLayoutViewProvider
{
public:
  CEditorLayoutViewProvider(QPointer<CEditorMainScreen> pParent) :
    IEditorLayoutViewProvider(),
    m_pParent(pParent)
  {}
  ~CEditorLayoutViewProvider() = default;

  QPointer<CEditorTutorialOverlay> GetTutorialOverlay() const override
  {
    if (nullptr != m_pParent)
    {
      return m_pParent->m_pTutorialOverlay;
    }
    return nullptr;
  }

  QPointer<CEditorWidgetBase> GetEditorWidget(EEditorWidget widget) const override
  {
    if (nullptr != m_pParent)
    {
      auto it = m_pParent->m_spWidgetsMap.find(widget);
      if (m_pParent->m_spWidgetsMap.end() != it)
      {
        return it->second;
      }
    }
    return nullptr;
  }

  void VisitWidgets(const std::function<void(QPointer<CEditorWidgetBase>, EEditorWidget)>& fnVisitor) override
  {
    if (nullptr != m_pParent)
    {
      for (auto it = m_pParent->m_spWidgetsMap.begin(); m_pParent->m_spWidgetsMap.end() != it; ++it)
      {
        fnVisitor(it->second, it->first);
      }
    }
  }

private:
  QPointer<CEditorMainScreen> m_pParent;
};

//----------------------------------------------------------------------------------------
//
CEditorMainScreen::CEditorMainScreen(QWidget* pParent) :
  QWidget(pParent),
  m_spEditorModel(std::make_unique<CEditorModel>(this)),
  m_spViewProvider(std::make_shared<CEditorLayoutViewProvider>(this)),
  m_spUi(std::make_shared<Ui::CEditorMainScreen>()),
  m_vpKeyBindingActions(),
  m_pLayout(nullptr),
  m_pTutorialOverlay(new CEditorTutorialOverlay(this)),
  m_spWidgetsMap(),
  m_spCurrentProject(nullptr),
  m_wpDbManager(),
  m_bInitialized(false),
  m_bProjectModified(false)
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

  connect(m_spEditorModel.get(), &CEditorModel::SignalProjectEdited,
          this, &CEditorMainScreen::SlotProjectEdited);

  m_pTutorialOverlay->Initialize(m_spEditorModel.get());
  m_pTutorialOverlay->Hide();

  // action Bars
  m_spUi->pProjectActionBar->SetActionBarPosition(CEditorActionBar::eTop);
  m_spUi->pProjectActionBar->Initialize();
  m_spUi->pProjectActionBar->ShowProjectActionBar();

  m_spUi->pProjectActionBar->m_spUi->ReadOnly->setVisible(false);

  connect(m_spUi->pProjectActionBar->m_spUi->pTitleLineEdit, &QLineEdit::editingFinished,
          this, &CEditorMainScreen::SlotProjectNameEditingFinished);
  connect(m_spUi->pProjectActionBar->m_spUi->SaveButton, &QPushButton::clicked,
          this, &CEditorMainScreen::SlotSaveClicked);
  connect(m_spUi->pProjectActionBar->m_spUi->UndoButton, &QPushButton::clicked,
          m_spEditorModel->UndoStack(), &QUndoStack::undo);
  connect(m_spUi->pProjectActionBar->m_spUi->RedoButton, &QPushButton::clicked,
          m_spEditorModel->UndoStack(), &QUndoStack::redo);
  connect(m_spUi->pProjectActionBar->m_spUi->ExportButton, &QPushButton::clicked,
          this, &CEditorMainScreen::SlotExportClicked);
  connect(m_spUi->pProjectActionBar->m_spUi->HelpButton, &QPushButton::clicked,
          this, &CEditorMainScreen::SlotHelpClicked);
  connect(m_spUi->pProjectActionBar->m_spUi->ExitButton, &QPushButton::clicked,
          this, &CEditorMainScreen::SlotExitClicked);

  // insert items in map
  for (auto type : EEditorWidget::_values())
  {
    CEditorWidgetBase* pWidet = CEditorFactory::CreateWidgetInstance(this, type);
    if (nullptr != pWidet)
    {
      m_spWidgetsMap.insert({type, pWidet});
    }
    else
    {
      qCritical() << "Could not create Editor widget for" << type._to_string();
    }
  }

  // initialize widgets
  for (auto it = m_spWidgetsMap.begin(); m_spWidgetsMap.end() != it; ++it)
  {
    connect(it->second, &CEditorWidgetBase::SignalProjectEdited,
            this, &CEditorMainScreen::SlotProjectEdited);
    connect(it->second, &CEditorWidgetBase::SignalUnloadFinished,
            this, &CEditorMainScreen::SlotUnloadFinished);

    it->second->SetEditorModel(m_spEditorModel.get());
    it->second->Initialize();
    it->second->setVisible(false);
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

  CreateLayout();

  ProjectLoaded(true);
}

//----------------------------------------------------------------------------------------
//
void CEditorMainScreen::LoadProject(qint32 iId)
{
  if (!m_bInitialized) { return; }

  m_spEditorModel->LoadProject(iId);
  m_spCurrentProject = m_spEditorModel->CurrentProject();

  CreateLayout();

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

  m_pLayout->ProjectUnloaded();

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

  RemoveLayout();
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
void CEditorMainScreen::CreateLayout()
{
  CSettings::EditorType type =
      CApplication::Instance()->Settings()->PreferedEditorLayout();
  if (CSettings::eNone == type)
  {
    type = CSettings::eClassic;
  }

  m_pLayout =
      CEditorFactory::CreateLayoutInstance(m_spUi->pMainWidget, type);

  QLayout* pLayout = m_spUi->pMainWidget->layout();
  if (nullptr != pLayout)
  {
    pLayout->addWidget(m_pLayout);
  }

  m_pLayout->Initialize(m_spViewProvider, m_spEditorModel.get());
}

//----------------------------------------------------------------------------------------
//
void CEditorMainScreen::ProjectLoaded(bool bNewProject)
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

  m_pLayout->ProjectLoaded(m_spCurrentProject, bNewProject);

  if (bNewProject)
  {
    SlotSaveClicked(true);
  }

  SetModificaitonFlag(false);

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
void CEditorMainScreen::RemoveLayout()
{
  QLayout* pLayout = m_spUi->pMainWidget->layout();
  if (nullptr != pLayout)
  {
    while (auto pItem = pLayout->takeAt(0))
    {
      if (nullptr != pItem)
      {
        if (nullptr != pItem->widget()) { delete pItem->widget(); }
        delete pItem;
      }
    }
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
