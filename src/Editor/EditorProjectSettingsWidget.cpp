#include "EditorProjectSettingsWidget.h"
#include "Application.h"
#include "EditorActionBar.h"
#include "EditorModel.h"
#include "SVersion.h"
#include "version.h"
#include "Project/KinkSelectionOverlay.h"
#include "Project/KinkTreeModel.h"
#include "Systems/DatabaseManager.h"
#include "Systems/HelpFactory.h"
#include "Systems/Project.h"
#include "Widgets/HelpOverlay.h"
#include "ui_EditorProjectSettingsWidget.h"
#include "ui_EditorActionBar.h"

#include <QDebug>

namespace
{
  const QString c_sRenameProjectHelpId =    "Editor/RenameProject";
  const QString c_sProjectVersionHelpId =   "Editor/ProjectVersion";
  const QString c_sEngineVersionHelpId =    "Editor/EngineVersion";
  const QString c_sProjectDescribtionHelpId="Editor/ProjectDescribtion";
  const QString c_sFetishListHelpId =       "Editor/FetishList";
}

//----------------------------------------------------------------------------------------
//
CEditorProjectSettingsWidget::CEditorProjectSettingsWidget(QWidget *parent) :
  CEditorWidgetBase(parent),
  m_spUi(std::make_unique<Ui::CEditorProjectSettingsWidget>()),
  m_spKinkSelectionOverlay(std::make_unique<CKinkSelectionOverlay>(this)),
  m_spCurrentProject(nullptr)
{
  m_spUi->setupUi(this);
}

CEditorProjectSettingsWidget::~CEditorProjectSettingsWidget()
{
  m_spKinkSelectionOverlay.reset();
}

//----------------------------------------------------------------------------------------
//
void CEditorProjectSettingsWidget::Initialize()
{
  m_bInitialized = false;

  m_spKinkSelectionOverlay->Initialize(KinkModel());
  connect(m_spKinkSelectionOverlay.get(), &CKinkSelectionOverlay::SignalOverlayClosed,
          this, &CEditorProjectSettingsWidget::SlotKinkOverlayClosed);

  auto wpHelpFactory = CApplication::Instance()->System<CHelpFactory>().lock();
  if (nullptr != wpHelpFactory)
  {
    m_spUi->pTitleLineEdit->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sRenameProjectHelpId);
    wpHelpFactory->RegisterHelp(c_sRenameProjectHelpId, ":/resources/help/editor/project_name_help.html");
    m_spUi->pProjectVersionContainer->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sProjectVersionHelpId);
    wpHelpFactory->RegisterHelp(c_sProjectVersionHelpId, ":/resources/help/editor/projectsettings/projectversion_help.html");
    m_spUi->pEngineVersionContainer->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sEngineVersionHelpId);
    wpHelpFactory->RegisterHelp(c_sEngineVersionHelpId, ":/resources/help/editor/projectsettings/engineversion_help.html");
    m_spUi->pDescribtionTextEdit->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sProjectDescribtionHelpId);
    wpHelpFactory->RegisterHelp(c_sProjectDescribtionHelpId, ":/resources/help/editor/projectsettings/describtion_textedit_help.html");
    m_spUi->pFetishListWidget->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sFetishListHelpId);
    wpHelpFactory->RegisterHelp(c_sFetishListHelpId, ":/resources/help/editor/projectsettings/fetish_tree_help.html");
  }

  auto spDataBaseManager = CApplication::Instance()->System<CDatabaseManager>().lock();
  if (nullptr != spDataBaseManager)
  {
    connect(spDataBaseManager.get(), &CDatabaseManager::SignalProjectRenamed,
            this, &CEditorProjectSettingsWidget::SlotProjectRenamed, Qt::QueuedConnection);
  }

  m_spUi->pFetishListWidget->setSelectionMode(QAbstractItemView::MultiSelection);

  m_spUi->pEngineMajorVersion->setValue(MAJOR_VERSION);
  m_spUi->pEngineMinorVersion->setValue(MINOR_VERSION);
  m_spUi->pEnginePatchVersion->setValue(PATCH_VERSION);
  m_spUi->WarningIcon->setVisible(false);

  m_bInitialized = true;
}

//----------------------------------------------------------------------------------------
//
void CEditorProjectSettingsWidget::LoadProject(tspProject spProject)
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr != m_spCurrentProject)
  {
    qWarning() << "Old Project was not unloaded before loading project.";
  }

  m_spCurrentProject = spProject;
  if (nullptr != m_spCurrentProject)
  {
    QReadLocker locker(&m_spCurrentProject->m_rwLock);

    m_spUi->pTitleLineEdit->blockSignals(true);
    m_spUi->pTitleLineEdit->setText(m_spCurrentProject->m_sName);
    m_spUi->pTitleLineEdit->blockSignals(false);

    SVersion targetVersion(m_spCurrentProject->m_iTargetVersion);
    SVersion engineVersion(MAJOR_VERSION, MINOR_VERSION, PATCH_VERSION);
    m_spUi->pEngineMajorVersion->setValue(static_cast<qint32>(targetVersion.m_iMajor));
    m_spUi->pEngineMinorVersion->setValue(static_cast<qint32>(targetVersion.m_iMinor));
    m_spUi->pEnginePatchVersion->setValue(static_cast<qint32>(targetVersion.m_iPatch));
    m_spUi->WarningIcon->setVisible(targetVersion != engineVersion);

    SVersion projVersion(m_spCurrentProject->m_iVersion);
    m_spUi->pProjectMajorVersion->blockSignals(true);
    m_spUi->pProjectMinorVersion->blockSignals(true);
    m_spUi->pProjectPatchVersion->blockSignals(true);
    m_spUi->pProjectMajorVersion->setValue(static_cast<qint32>(projVersion.m_iMajor));
    m_spUi->pProjectMinorVersion->setValue(static_cast<qint32>(projVersion.m_iMinor));
    m_spUi->pProjectPatchVersion->setValue(static_cast<qint32>(projVersion.m_iPatch));
    m_spUi->pProjectMajorVersion->blockSignals(false);
    m_spUi->pProjectMinorVersion->blockSignals(false);
    m_spUi->pProjectPatchVersion->blockSignals(false);

    m_spUi->pDescribtionTextEdit->setPlainText(m_spCurrentProject->m_sDescribtion);

    m_spUi->pFetishListWidget->addItems(m_spCurrentProject->m_vsKinks);
    m_spUi->pFetishListWidget->sortItems();
    KinkModel()->SetSelections(m_spCurrentProject->m_vsKinks);
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorProjectSettingsWidget::UnloadProject()
{
  WIDGET_INITIALIZED_GUARD

  m_spCurrentProject = nullptr;

  m_spUi->pEngineMajorVersion->setValue(MAJOR_VERSION);
  m_spUi->pEngineMinorVersion->setValue(MINOR_VERSION);
  m_spUi->pEnginePatchVersion->setValue(PATCH_VERSION);
  m_spUi->WarningIcon->setVisible(false);

  m_spUi->pDescribtionTextEdit->clear();

  m_spUi->pFetishListWidget->clear();
  KinkModel()->ResetSelections();
}

//----------------------------------------------------------------------------------------
//
void CEditorProjectSettingsWidget::SaveProject()
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }

  QWriteLocker locker(&m_spCurrentProject->m_rwLock);
  m_spCurrentProject->m_iVersion = SVersion(static_cast<quint32>(m_spUi->pProjectMajorVersion->value()),
                                            static_cast<quint32>(m_spUi->pProjectMinorVersion->value()),
                                            static_cast<quint32>(m_spUi->pProjectPatchVersion->value()));

  m_spUi->pEngineMajorVersion->setValue(MAJOR_VERSION);
  m_spUi->pEngineMinorVersion->setValue(MINOR_VERSION);
  m_spUi->pEnginePatchVersion->setValue(PATCH_VERSION);
  m_spCurrentProject->m_iTargetVersion = SVersion(static_cast<quint32>(m_spUi->pEngineMajorVersion->value()),
                                                  static_cast<quint32>(m_spUi->pEngineMinorVersion->value()),
                                                  static_cast<quint32>(m_spUi->pEnginePatchVersion->value()));
  m_spUi->WarningIcon->setVisible(false);

  m_spCurrentProject->m_sDescribtion = m_spUi->pDescribtionTextEdit->toPlainText();

  m_spCurrentProject->m_vsKinks.clear();
  for (qint32 i = 0; m_spUi->pFetishListWidget->count() > i; ++i)
  {
    m_spCurrentProject->m_vsKinks.push_back(m_spUi->pFetishListWidget->item(i)->text());
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorProjectSettingsWidget::on_pTitleLineEdit_editingFinished()
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }

  QString sNewName = m_spUi->pTitleLineEdit->text();
  const QString sFinalName = EditorModel()->RenameProject(sNewName);
  m_spUi->pTitleLineEdit->blockSignals(true);
  m_spUi->pTitleLineEdit->setText(sFinalName);
  m_spUi->pTitleLineEdit->blockSignals(false);
}

//----------------------------------------------------------------------------------------
//
void CEditorProjectSettingsWidget::on_pProjectMajorVersion_valueChanged(qint32 iValue)
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }
  Q_UNUSED(iValue)
  emit SignalProjectEdited();
}

//----------------------------------------------------------------------------------------
//
void CEditorProjectSettingsWidget::on_pProjectMinorVersion_valueChanged(qint32 iValue)
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }
  Q_UNUSED(iValue)
  emit SignalProjectEdited();
}

//----------------------------------------------------------------------------------------
//
void CEditorProjectSettingsWidget::on_pProjectPatchVersion_valueChanged(qint32 iValue)
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }
  Q_UNUSED(iValue)
  emit SignalProjectEdited();
}

//----------------------------------------------------------------------------------------
//
void CEditorProjectSettingsWidget::on_pDescribtionTextEdit_textChanged()
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }
  emit SignalProjectEdited();
}

//----------------------------------------------------------------------------------------
//
void CEditorProjectSettingsWidget::SlotAddKinksClicked()
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }
  m_spKinkSelectionOverlay->Show();
}

//----------------------------------------------------------------------------------------
//
void CEditorProjectSettingsWidget::SlotKinkOverlayClosed(bool bAccepted)
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }
  if (bAccepted)
  {
    m_spUi->pFetishListWidget->clear();
    std::vector<tspKink> vspKinks = KinkModel()->SelectedItems();
    for (const tspKink& spKink : vspKinks)
    {
      QReadLocker locker(&spKink->m_rwLock);
      m_spUi->pFetishListWidget->addItem(spKink->m_sName);
    }
    m_spUi->pFetishListWidget->sortItems();

    emit SignalProjectEdited();
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorProjectSettingsWidget::SlotProjectRenamed(qint32 iId)
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }

  QReadLocker locker(&m_spCurrentProject->m_rwLock);
  qint32 iThisId = m_spCurrentProject->m_iId;
  QString sName = m_spCurrentProject->m_sName;
  locker.unlock();

  if (iId == iThisId)
  {
    m_spUi->pTitleLineEdit->blockSignals(true);
    m_spUi->pTitleLineEdit->setText(sName);
    m_spUi->pTitleLineEdit->blockSignals(false);
    emit SignalProjectEdited();
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorProjectSettingsWidget::SlotRemoveKinksClicked()
{
  auto vpItems = m_spUi->pFetishListWidget->selectedItems();
  m_spUi->pFetishListWidget->clearSelection();
  QStringList vsRemovedSelections;
  for (QListWidgetItem* pItem : qAsConst(vpItems))
  {
    vsRemovedSelections << pItem->text();
    delete pItem;
  }

  KinkModel()->ResetSelections(vsRemovedSelections);
  emit SignalProjectEdited();
}

//----------------------------------------------------------------------------------------
//
void CEditorProjectSettingsWidget::OnActionBarAboutToChange()
{
  if (nullptr != ActionBar())
  {
    disconnect(ActionBar()->m_spUi->AddFetishButton, &QPushButton::clicked,
            this, &CEditorProjectSettingsWidget::SlotAddKinksClicked);
    disconnect(ActionBar()->m_spUi->RemoveFetishButton, &QPushButton::clicked,
            this, &CEditorProjectSettingsWidget::SlotRemoveKinksClicked);
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorProjectSettingsWidget::OnActionBarChanged()
{
  if (nullptr != ActionBar())
  {
    ActionBar()->ShowProjectSettingsActionBar();
    connect(ActionBar()->m_spUi->AddFetishButton, &QPushButton::clicked,
            this, &CEditorProjectSettingsWidget::SlotAddKinksClicked);
    connect(ActionBar()->m_spUi->RemoveFetishButton, &QPushButton::clicked,
            this, &CEditorProjectSettingsWidget::SlotRemoveKinksClicked);
  }
}
