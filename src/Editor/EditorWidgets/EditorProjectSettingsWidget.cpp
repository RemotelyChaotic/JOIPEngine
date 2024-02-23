#include "EditorProjectSettingsWidget.h"
#include "Application.h"
#include "SVersion.h"
#include "out_Version.h"
#include "Editor/EditorActionBar.h"
#include "Editor/EditorModel.h"
#include "Editor/EditorWidgetTypes.h"
#include "Editor/Project/CommandChangeDescribtion.h"
#include "Editor/Project/CommandChangeEmitterCount.h"
#include "Editor/Project/CommandChangeFetishes.h"
#include "Editor/Project/CommandChangeFont.h"
#include "Editor/Project/CommandChangeLayout.h"
#include "Editor/Project/CommandChangeProjectName.h"
#include "Editor/Project/CommandChangeVersion.h"
#include "Editor/Project/KinkCompleter.h"
#include "Editor/Project/KinkSelectionOverlay.h"
#include "Editor/Project/KinkTreeModel.h"
#include "Editor/Tutorial/ProjectSettingsTutorialStateSwitchHandler.h"
#include "Systems/DatabaseManager.h"
#include "Systems/HelpFactory.h"
#include "Systems/Kink.h"
#include "Systems/Project.h"
#include "Utils/UndoRedoFilter.h"
#include "Widgets/FlowLayout.h"
#include "Widgets/HelpOverlay.h"

#include <QCompleter>
#include <QDebug>
#include <QMenu>
#include <QTextDocument>
#include <QUndoStack>

DECLARE_EDITORWIDGET(CEditorProjectSettingsWidget, EEditorWidget::eProjectSettings)

namespace
{
  const qint32 c_iCurrentDefaultLayout = 1;
  const char c_sKinkRootWidgetProperty[] = "KinkRootWidget";

  const QString c_sRenameProjectHelpId =    "Editor/RenameProject";
  const QString c_sProjectVersionHelpId =   "Editor/ProjectVersion";
  const QString c_sEngineVersionHelpId =    "Editor/EngineVersion";
  const QString c_sSoundEmitterCountHelpId ="Editor/SoundEmitterCount";
  const QString c_sLayoutHelpId =           "Editor/Layout";
  const QString c_sProjectFontHelpId       ="Editor/ProjectFont";
  const QString c_sProjectDescribtionHelpId="Editor/ProjectDescribtion";
  const QString c_sFetishListHelpId =       "Editor/FetishList";
  const QString c_sAddFetishHelpId =        "Editor/AddFetish";
}

//----------------------------------------------------------------------------------------
//
CEditorProjectSettingsWidget::CEditorProjectSettingsWidget(QWidget *parent) :
  CEditorWidgetBase(parent),
  m_spKinkSelectionOverlay(std::make_unique<CKinkSelectionOverlay>(this)),
  m_spUi(std::make_shared<Ui::CEditorProjectSettingsWidget>()),
  m_spTutorialStateSwitchHandler(nullptr),
  m_spCurrentProject(nullptr)
{
  m_spUi->setupUi(this);
}

CEditorProjectSettingsWidget::~CEditorProjectSettingsWidget()
{
  m_spUi->pFetishLineEdit->completer()->setModel(nullptr);
  m_spKinkSelectionOverlay.reset();
}

//----------------------------------------------------------------------------------------
//
void CEditorProjectSettingsWidget::Initialize()
{
  m_bInitialized = false;

  m_spTutorialStateSwitchHandler =
      std::make_shared<CProjectSettingsTutorialStateSwitchHandler>(this, m_spUi);
  EditorModel()->AddTutorialStateSwitchHandler(m_spTutorialStateSwitchHandler);

  m_spKinkSelectionOverlay->Initialize(KinkModel());
  connect(KinkModel(), &CKinkTreeModel::SignalCheckedItem,
          this, &CEditorProjectSettingsWidget::SlotKinkChecked);

  auto wpHelpFactory = CApplication::Instance()->System<CHelpFactory>().lock();
  if (nullptr != wpHelpFactory)
  {
    m_spUi->pTitleLineEdit->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sRenameProjectHelpId);
    wpHelpFactory->RegisterHelp(c_sRenameProjectHelpId, ":/resources/help/editor/project_name_help.html");
    m_spUi->pProjectVersionContainer->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sProjectVersionHelpId);
    wpHelpFactory->RegisterHelp(c_sProjectVersionHelpId, ":/resources/help/editor/projectsettings/projectversion_help.html");
    m_spUi->pEngineVersionContainer->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sEngineVersionHelpId);
    wpHelpFactory->RegisterHelp(c_sEngineVersionHelpId, ":/resources/help/editor/projectsettings/engineversion_help.html");
    m_spUi->pSoundEmitterCount->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sSoundEmitterCountHelpId);
    wpHelpFactory->RegisterHelp(c_sSoundEmitterCountHelpId, ":/resources/help/editor/projectsettings/number_soundemitters_help.html");
    m_spUi->pFontComboBox->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sProjectFontHelpId);
    wpHelpFactory->RegisterHelp(c_sProjectFontHelpId, ":/resources/help/editor/projectsettings/font_help.html");
    m_spUi->pDefaultLayoutComboBox->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sLayoutHelpId);
    wpHelpFactory->RegisterHelp(c_sLayoutHelpId, ":/resources/help/editor/projectsettings/layout_help.html");
    m_spUi->pDescribtionTextEdit->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sProjectDescribtionHelpId);
    wpHelpFactory->RegisterHelp(c_sProjectDescribtionHelpId, ":/resources/help/editor/projectsettings/describtion_textedit_help.html");
    m_spUi->pFetishListWidget->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sFetishListHelpId);
    wpHelpFactory->RegisterHelp(c_sFetishListHelpId, ":/resources/help/editor/projectsettings/fetish_tree_help.html");
    m_spUi->pFetishLineEdit->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sAddFetishHelpId);
    wpHelpFactory->RegisterHelp(c_sAddFetishHelpId, ":/resources/help/editor/addfetish_button_help.html");
    m_spUi->FetishOverlayButton->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sAddFetishHelpId);
    wpHelpFactory->RegisterHelp(c_sAddFetishHelpId, ":/resources/help/editor/addfetish_button_help.html");
  }

  auto spDataBaseManager = CApplication::Instance()->System<CDatabaseManager>().lock();
  if (nullptr != spDataBaseManager)
  {
    connect(spDataBaseManager.get(), &CDatabaseManager::SignalProjectRenamed,
            this, &CEditorProjectSettingsWidget::SlotProjectRenamed, Qt::QueuedConnection);
    connect(spDataBaseManager.get(), &CDatabaseManager::SignalResourceAdded,
            this, &CEditorProjectSettingsWidget::SlotResourceAdded, Qt::QueuedConnection);
    connect(spDataBaseManager.get(), &CDatabaseManager::SignalResourceRemoved,
            this, &CEditorProjectSettingsWidget::SlotResourceRemoved, Qt::QueuedConnection);
    connect(spDataBaseManager.get(), &CDatabaseManager::SignalResourceRenamed,
            this, &CEditorProjectSettingsWidget::SlotResourceRenamed, Qt::QueuedConnection);
  }

  CKinkCompleter* pCompleter = new CKinkCompleter(KinkModel(), m_spUi->pFetishLineEdit);
  pCompleter->setFilterMode(Qt::MatchRecursive | Qt::MatchContains);
  pCompleter->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
  pCompleter->setCompletionMode(QCompleter::PopupCompletion);
  pCompleter->setCompletionRole(Qt::DisplayRole);
  pCompleter->setCompletionColumn(0);
  pCompleter->setMaxVisibleItems(10);
  m_spUi->pFetishLineEdit->setCompleter(pCompleter);

  m_spUi->pFetishListWidget->SetCallbacks(
      [this](QPushButton* pRemove, const QString& sTag)
      {
        if (nullptr != pRemove)
        {
          pRemove->setProperty(c_sKinkRootWidgetProperty, sTag);
          connect(pRemove, &QPushButton::clicked,
                  this, &::CEditorProjectSettingsWidget::SlotRemoveKinkClicked);
        }

        KinkModel()->SetSelections(QStringList() << sTag);
      },
      [this](const QStringList& vsTags)
      {
        KinkModel()->ResetSelections(vsTags);
        emit SignalProjectEdited();
      });

  m_spUi->pFetishListWidget->SetSortFunction(
      [](std::vector<std::shared_ptr<SLockableTagData>>& vspTags) {
        std::sort(vspTags.begin(), vspTags.end(),
                  [](const std::shared_ptr<SLockableTagData>& left,
                     const std::shared_ptr<SLockableTagData>& right) {
                    QReadLocker lockerLeft(&left->m_rwLock);
                    QReadLocker lockerRight(&right->m_rwLock);
                    return std::dynamic_pointer_cast<SKink>(left)->m_iIdForOrdering <
                           std::dynamic_pointer_cast<SKink>(right)->m_iIdForOrdering;
                  });
  });

  SVersion version(VERSION_XYZ);
  m_spUi->pEngineMajorVersion->setValue(version.m_iMajor);
  m_spUi->pEngineMinorVersion->setValue(version.m_iMinor);
  m_spUi->pEnginePatchVersion->setValue(version.m_iPatch);
  m_spUi->WarningIcon->setVisible(false);

  new CUndoRedoFilter(m_spUi->pTitleLineEdit, nullptr);
  new CUndoRedoFilter(m_spUi->pProjectMajorVersion, nullptr);
  new CUndoRedoFilter(m_spUi->pProjectMinorVersion, nullptr);
  new CUndoRedoFilter(m_spUi->pProjectPatchVersion, nullptr);
  new CUndoRedoFilter(m_spUi->pFetishLineEdit, nullptr);
  new CUndoRedoFilter(m_spUi->pFontComboBox, nullptr);

  m_spUi->pDescribtionTextEdit->setPlaceholderText("No describtion set");
  connect(m_spUi->pDescribtionTextEdit->document(), &QTextDocument::undoCommandAdded,
          this, &CEditorProjectSettingsWidget::SlotUndoForDescribtionAdded);
  connect(m_spUi->pDescribtionTextEdit, &CRichTextEdit::UndoTriggered,
          this, [this]() { UndoStack()->undo(); });
  connect(m_spUi->pDescribtionTextEdit, &CRichTextEdit::RedoTriggered,
          this, [this]() { UndoStack()->redo(); });

  m_bInitialized = true;
}

//----------------------------------------------------------------------------------------
//
void CEditorProjectSettingsWidget::LoadProject(tspProject spProject)
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr != m_spCurrentProject)
  {
    assert(false);
    qWarning() << "Old Project was not unloaded before loading project.";
  }

  m_spCurrentProject = spProject;
  if (nullptr != m_spCurrentProject)
  {
    QReadLocker locker(&m_spCurrentProject->m_rwLock);

    bool bReadOnly = EditorModel()->IsReadOnly();

    m_spUi->pTitleLineEdit->blockSignals(true);
    m_spUi->pTitleLineEdit->setText(m_spCurrentProject->m_sName);
    m_spUi->pTitleLineEdit->setProperty(editor::c_sPropertyOldValue, m_spCurrentProject->m_sName);
    m_spUi->pTitleLineEdit->setReadOnly(bReadOnly);
    m_spUi->pTitleLineEdit->blockSignals(false);

    SVersion targetVersion(m_spCurrentProject->m_iTargetVersion);
    SVersion engineVersion(VERSION_XYZ);
    m_spUi->pEngineMajorVersion->setValue(static_cast<qint32>(targetVersion.m_iMajor));
    m_spUi->pEngineMinorVersion->setValue(static_cast<qint32>(targetVersion.m_iMinor));
    m_spUi->pEnginePatchVersion->setValue(static_cast<qint32>(targetVersion.m_iPatch));
    m_spUi->WarningIcon->setVisible(targetVersion != engineVersion);

    SVersion projVersion(m_spCurrentProject->m_iVersion);
    m_spUi->pProjectMajorVersion->blockSignals(true);
    m_spUi->pProjectMinorVersion->blockSignals(true);
    m_spUi->pProjectPatchVersion->blockSignals(true);
    m_spUi->pProjectMajorVersion->setProperty(editor::c_sPropertyOldValue, static_cast<qint32>(projVersion.m_iMajor));
    m_spUi->pProjectMinorVersion->setProperty(editor::c_sPropertyOldValue, static_cast<qint32>(projVersion.m_iMinor));
    m_spUi->pProjectPatchVersion->setProperty(editor::c_sPropertyOldValue, static_cast<qint32>(projVersion.m_iPatch));
    m_spUi->pProjectMajorVersion->setValue(static_cast<qint32>(projVersion.m_iMajor));
    m_spUi->pProjectMinorVersion->setValue(static_cast<qint32>(projVersion.m_iMinor));
    m_spUi->pProjectPatchVersion->setValue(static_cast<qint32>(projVersion.m_iPatch));
    m_spUi->pProjectMajorVersion->setEnabled(!bReadOnly);
    m_spUi->pProjectMinorVersion->setEnabled(!bReadOnly);
    m_spUi->pProjectPatchVersion->setEnabled(!bReadOnly);
    m_spUi->pProjectMajorVersion->blockSignals(false);
    m_spUi->pProjectMinorVersion->blockSignals(false);
    m_spUi->pProjectPatchVersion->blockSignals(false);

    m_spUi->pSoundEmitterCount->blockSignals(true);
    m_spUi->pSoundEmitterCount->setValue(m_spCurrentProject->m_iNumberOfSoundEmitters);
    m_spUi->pSoundEmitterCount->setProperty(editor::c_sPropertyOldValue, m_spCurrentProject->m_iNumberOfSoundEmitters);
    m_spUi->pSoundEmitterCount->setEnabled(!bReadOnly);
    m_spUi->pSoundEmitterCount->blockSignals(false);

    m_spUi->pFontComboBox->blockSignals(true);
    m_spUi->pFontComboBox->setCurrentFont(m_spCurrentProject->m_sFont);
    m_spUi->pFontComboBox->setProperty(editor::c_sPropertyOldValue, m_spCurrentProject->m_sFont);
    m_spUi->pFontComboBox->setEnabled(!bReadOnly);
    m_spUi->pFontComboBox->blockSignals(false);

    m_spUi->pDefaultLayoutComboBox->blockSignals(true);
    m_spUi->pDefaultLayoutComboBox->clear();
    m_spUi->pDefaultLayoutComboBox->addItem("(In-Built) Old Layout", "qrc:/qml/resources/qml/JoipEngine/PlayerDefaultLayoutClassic.qml");
    m_spUi->pDefaultLayoutComboBox->addItem("(In-Built) 1.4.0 Layout", "qrc:/qml/resources/qml/JoipEngine/PlayerDefaultLayout.qml");
    for (const auto& [sName, spResource] : m_spCurrentProject->m_spResourcesMap)
    {
      QReadLocker resLocker(&spResource->m_rwLock);
      if (EResourceType::eLayout == spResource->m_type._to_integral())
      {
        m_spUi->pDefaultLayoutComboBox->addItem(sName, sName);
      }
    }
    m_spUi->pDefaultLayoutComboBox->setCurrentIndex(
        m_spUi->pDefaultLayoutComboBox->findData(m_spCurrentProject->m_sPlayerLayout));
    m_spUi->pDefaultLayoutComboBox->setProperty(editor::c_sPropertyOldValue, m_spCurrentProject->m_sPlayerLayout);
    m_spUi->pDefaultLayoutComboBox->blockSignals(false);

    m_spUi->pDescribtionTextEdit->setPlainText(m_spCurrentProject->m_sDescribtion);
    m_spUi->pDescribtionTextEdit->setReadOnly(bReadOnly);

    m_spUi->pFetishLineEdit->setEnabled(!bReadOnly);
    m_spUi->pFetishListWidget->SetReadOnly(bReadOnly);
    m_spUi->pFetishListWidget->setEnabled(!bReadOnly);


    AddKinks(m_spCurrentProject->m_vsKinks);
    KinkModel()->SetSelections(m_spCurrentProject->m_vsKinks);

    m_spKinkSelectionOverlay->LoadProject(bReadOnly);

    SetLoaded(true);
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorProjectSettingsWidget::UnloadProject()
{
  WIDGET_INITIALIZED_GUARD

  m_spCurrentProject = nullptr;

  m_spUi->pTitleLineEdit->setReadOnly(false);

  SVersion version(VERSION_XYZ);
  m_spUi->pEngineMajorVersion->setValue(version.m_iMajor);
  m_spUi->pEngineMinorVersion->setValue(version.m_iMinor);
  m_spUi->pEnginePatchVersion->setValue(version.m_iPatch);
  m_spUi->pProjectMajorVersion->setEnabled(true);
  m_spUi->pProjectMinorVersion->setEnabled(true);
  m_spUi->pProjectPatchVersion->setEnabled(true);
  m_spUi->WarningIcon->setVisible(false);

  m_spUi->pDescribtionTextEdit->clearSource();
  m_spUi->pDescribtionTextEdit->setReadOnly(false);

  m_spUi->pFetishLineEdit->setEnabled(true);
  m_spUi->pFetishListWidget->setEnabled(true);

  KinkModel()->ResetSelections();
  m_spUi->pFetishListWidget->ClearTags();

  m_spKinkSelectionOverlay->Hide();
  m_spKinkSelectionOverlay->UnloadProject();

  SetLoaded(false);
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

  SVersion version(VERSION_XYZ);
  m_spUi->pEngineMajorVersion->setValue(version.m_iMajor);
  m_spUi->pEngineMinorVersion->setValue(version.m_iMinor);
  m_spUi->pEnginePatchVersion->setValue(version.m_iPatch);
  m_spCurrentProject->m_iTargetVersion = SVersion(static_cast<quint32>(m_spUi->pEngineMajorVersion->value()),
                                                  static_cast<quint32>(m_spUi->pEngineMinorVersion->value()),
                                                  static_cast<quint32>(m_spUi->pEnginePatchVersion->value()));
  m_spUi->WarningIcon->setVisible(false);

  m_spCurrentProject->m_iNumberOfSoundEmitters = m_spUi->pSoundEmitterCount->value();

  m_spCurrentProject->m_sFont = m_spUi->pFontComboBox->currentFont().family();

  m_spCurrentProject->m_sDescribtion = m_spUi->pDescribtionTextEdit->toPlainText();

  m_spCurrentProject->m_sPlayerLayout = m_spUi->pDefaultLayoutComboBox->currentData().toString();

  m_spCurrentProject->m_vsKinks.clear();
  const auto& vspKinks = m_spUi->pFetishListWidget->Tags();
  for (qint32 i = 0; static_cast<qint32>(vspKinks.size()) > i; ++i)
  {
    QReadLocker locker(&vspKinks[static_cast<size_t>(i)]->m_rwLock);
    m_spCurrentProject->m_vsKinks.push_back(vspKinks[static_cast<size_t>(i)]->m_sName);
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorProjectSettingsWidget::on_pTitleLineEdit_editingFinished()
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }

  UndoStack()->push(new CCommandChangeProjectName(m_spUi->pTitleLineEdit, EditorModel()));
}

//----------------------------------------------------------------------------------------
//
void CEditorProjectSettingsWidget::on_pProjectMajorVersion_valueChanged(qint32 iValue)
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }
  Q_UNUSED(iValue)

  QPointer<CEditorProjectSettingsWidget> pThis(this);
  UndoStack()->push(new CCommandChangeVersion(m_spUi->pProjectMajorVersion,
                                              m_spUi->pProjectMinorVersion,
                                              m_spUi->pProjectPatchVersion,
                                              [pThis]() {
                                                emit pThis->SignalProjectEdited();
                                              }));
}

//----------------------------------------------------------------------------------------
//
void CEditorProjectSettingsWidget::on_pProjectMinorVersion_valueChanged(qint32 iValue)
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }
  Q_UNUSED(iValue)

  QPointer<CEditorProjectSettingsWidget> pThis(this);
  UndoStack()->push(new CCommandChangeVersion(m_spUi->pProjectMajorVersion,
                                              m_spUi->pProjectMinorVersion,
                                              m_spUi->pProjectPatchVersion,
                                              [pThis]() {
                                                emit pThis->SignalProjectEdited();
                                              }));
}

//----------------------------------------------------------------------------------------
//
void CEditorProjectSettingsWidget::on_pProjectPatchVersion_valueChanged(qint32 iValue)
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }
  Q_UNUSED(iValue)

  QPointer<CEditorProjectSettingsWidget> pThis(this);
  UndoStack()->push(new CCommandChangeVersion(m_spUi->pProjectMajorVersion,
                                              m_spUi->pProjectMinorVersion,
                                              m_spUi->pProjectPatchVersion,
                                              [pThis]() {
                                                emit pThis->SignalProjectEdited();
                                              }));
}

//----------------------------------------------------------------------------------------
//
void CEditorProjectSettingsWidget::on_pSoundEmitterCount_valueChanged(qint32 iValue)
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }
  Q_UNUSED(iValue)

  QPointer<CEditorProjectSettingsWidget> pThis(this);
  UndoStack()->push(new CCommandChangeEmitterCount(m_spUi->pSoundEmitterCount,
                                                   [pThis]() {
                                                     emit pThis->SignalProjectEdited();
                                                   }));
}

//----------------------------------------------------------------------------------------
//
void CEditorProjectSettingsWidget::on_pFontComboBox_currentFontChanged(const QFont& font)
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }
  Q_UNUSED(font)

  QPointer<CEditorProjectSettingsWidget> pThis(this);
  UndoStack()->push(new CCommandChangeFont(m_spUi->pFontComboBox,
                                           [pThis]() {
                                             emit pThis->SignalProjectEdited();
                                           }));
}

//----------------------------------------------------------------------------------------
//
void CEditorProjectSettingsWidget::on_pDefaultLayoutComboBox_currentIndexChanged(qint32 iIdx)
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }
  Q_UNUSED(iIdx)

  QPointer<CEditorProjectSettingsWidget> pThis(this);
  UndoStack()->push(new CCommandChangeLayout(m_spUi->pDefaultLayoutComboBox,
                                             m_spCurrentProject,
                                             [pThis]() {
                                               emit pThis->SignalProjectEdited();
                                               emit pThis->EditorModel()->SignalProjectPropertiesEdited();
                                             }));
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
void CEditorProjectSettingsWidget::on_pFetishLineEdit_editingFinished()
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }
  QString sKink = m_spUi->pFetishLineEdit->text();
  if (sKink.isEmpty()) { return; }

  UndoStack()->push(
        new CCommandAddFetishes(this,
                                std::bind(&CEditorProjectSettingsWidget::AddKinks, this, std::placeholders::_1),
                                std::bind(&CTagsView::RemoveTags, m_spUi->pFetishListWidget, std::placeholders::_1),
                                {sKink}));
  emit SignalProjectEdited();
}

//----------------------------------------------------------------------------------------
//
void CEditorProjectSettingsWidget::on_FetishOverlayButton_clicked()
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }
  m_spKinkSelectionOverlay->Show();
}

//----------------------------------------------------------------------------------------
//
void CEditorProjectSettingsWidget::SlotKinkChecked(const QModelIndex& index, bool bChecked)
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }
  QString sKink = KinkModel()->data(index, Qt::DisplayRole).toString();
  if (bChecked)
  {
    UndoStack()->push(
          new CCommandAddFetishes(this,
                                  std::bind(&CEditorProjectSettingsWidget::AddKinks, this, std::placeholders::_1),
                                  std::bind(&CTagsView::RemoveTags, m_spUi->pFetishListWidget, std::placeholders::_1),
                                  {sKink}));
  }
  else
  {
    UndoStack()->push(
          new CCommandRemoveFetishes(this,
                                     std::bind(&CEditorProjectSettingsWidget::AddKinks, this, std::placeholders::_1),
                                     std::bind(&CTagsView::RemoveTags, m_spUi->pFetishListWidget, std::placeholders::_1),
                                     {sKink}));
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
void CEditorProjectSettingsWidget::SlotRemoveKinkClicked()
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }

  QString sKink = sender()->property(c_sKinkRootWidgetProperty).toString();

  UndoStack()->push(
        new CCommandRemoveFetishes(this,
                                   std::bind(&CEditorProjectSettingsWidget::AddKinks, this, std::placeholders::_1),
                                   std::bind(&CTagsView::RemoveTags, m_spUi->pFetishListWidget, std::placeholders::_1),
                                   {sKink}));
}

//----------------------------------------------------------------------------------------
//
void CEditorProjectSettingsWidget::SlotResourceAdded(qint32 iProjId, const QString& sName)
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }
  m_spCurrentProject->m_rwLock.lockForRead();
  qint32 iCurrentId = m_spCurrentProject->m_iId;
  m_spCurrentProject->m_rwLock.unlock();

  if (iCurrentId != iProjId) { return; }

  auto spDbManager = CApplication::Instance()->System<CDatabaseManager>().lock();
  if (nullptr != spDbManager)
  {
    auto spResource = spDbManager->FindResourceInProject(m_spCurrentProject, sName);
    if (nullptr != spResource)
    {
      QReadLocker resLocker(&spResource->m_rwLock);
      if (EResourceType::eLayout == spResource->m_type._to_integral())
      {
        m_spUi->pDefaultLayoutComboBox->addItem(spResource->m_sName,spResource->m_sName);
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorProjectSettingsWidget::SlotResourceRemoved(qint32 iProjId, const QString& sName)
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }
  m_spCurrentProject->m_rwLock.lockForRead();
  qint32 iCurrentId = m_spCurrentProject->m_iId;
  m_spCurrentProject->m_rwLock.unlock();

  if (iCurrentId != iProjId) { return; }

  qint32 iIdx = m_spUi->pDefaultLayoutComboBox->findData(sName);
  if (-1 != iIdx)
  {
    m_spUi->pDefaultLayoutComboBox->removeItem(iIdx);
    if (m_spUi->pDefaultLayoutComboBox->currentIndex() == iIdx)
    {
      m_spUi->pDefaultLayoutComboBox->setCurrentIndex(c_iCurrentDefaultLayout);
      QPointer<CEditorProjectSettingsWidget> pThis(this);
      UndoStack()->push(new CCommandChangeLayout(m_spUi->pDefaultLayoutComboBox,
                                                 m_spCurrentProject,
                                                 [pThis]() {
                                                   emit pThis->SignalProjectEdited();
                                                   emit pThis->EditorModel()->SignalProjectPropertiesEdited();
                                                 }));
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorProjectSettingsWidget::SlotResourceRenamed(qint32 iProjId, const QString& sOldName,
                                                       const QString& sName)
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }
  m_spCurrentProject->m_rwLock.lockForRead();
  qint32 iCurrentId = m_spCurrentProject->m_iId;
  m_spCurrentProject->m_rwLock.unlock();

  if (iCurrentId != iProjId) { return; }

  auto spDbManager = CApplication::Instance()->System<CDatabaseManager>().lock();
  if (nullptr != spDbManager)
  {
    auto spResource = spDbManager->FindResourceInProject(m_spCurrentProject, sName);
    if (nullptr != spResource)
    {
      QReadLocker resLocker(&spResource->m_rwLock);
      if (EResourceType::eLayout == spResource->m_type._to_integral())
      {
        qint32 iIdx = m_spUi->pDefaultLayoutComboBox->findData(spResource->m_sName);
        m_spUi->pDefaultLayoutComboBox->removeItem(iIdx);
        m_spUi->pDefaultLayoutComboBox->addItem(spResource->m_sName,spResource->m_sName);
        m_spUi->pDefaultLayoutComboBox->setCurrentIndex(m_spUi->pDefaultLayoutComboBox->count());
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorProjectSettingsWidget::SlotUndoForDescribtionAdded()
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }

  QPointer<CEditorProjectSettingsWidget> pThis(this);
  UndoStack()->push(new CCommandChangeDescribtion(m_spUi->pDescribtionTextEdit->document(),
                                                  [pThis]() {
                                                    emit pThis->SignalProjectEdited();
                                                  }));
}

//----------------------------------------------------------------------------------------
//
void CEditorProjectSettingsWidget::AddKinks(QStringList vsKinks)
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }

  auto spDbManager = CApplication::Instance()->System<CDatabaseManager>().lock();
  if (nullptr!= spDbManager)
  {
    std::vector<std::shared_ptr<SLockableTagData>> vspKinksAdded;
    for (const QString& sKing : qAsConst(vsKinks))
    {
      tspKink spKink = spDbManager->FindKink(sKing);
      if(nullptr != spKink)
      {
        const auto& vspKinks = m_spUi->pFetishListWidget->Tags();
        if (vspKinks.end() != std::find_if(vspKinks.begin(), vspKinks.end(),
                                           [&spKink](const std::shared_ptr<SLockableTagData>& left) -> bool {
          QReadLocker lockerLeft(&left->m_rwLock);
          QReadLocker lockerRight(&spKink->m_rwLock);
          return left->m_sName == spKink->m_sName && left->m_sType == spKink->m_sType;
        })) continue;
        vspKinksAdded.push_back(spKink);
      }
    }

    m_spUi->pFetishListWidget->AddTags(vspKinksAdded);
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorProjectSettingsWidget::OnActionBarAboutToChange()
{
  if (nullptr != ActionBar())
  {
    // Nothing to do
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorProjectSettingsWidget::OnActionBarChanged()
{
  if (nullptr != ActionBar())
  {
    ActionBar()->ShowProjectSettingsActionBar();
  }
}
