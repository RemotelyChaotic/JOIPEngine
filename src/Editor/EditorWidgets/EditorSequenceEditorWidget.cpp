#include "EditorSequenceEditorWidget.h"

#include "Editor/SequenceEditor/CommandAddNewSequence.h"
#include "Editor/SequenceEditor/CommandChangeOpenedSequence.h"
#include "Editor/SequenceEditor/SequencePropertiesOverlay.h"

#include "Editor/EditorActionBar.h"
#include "Editor/EditorEditableFileModel.h"
#include "Editor/EditorModel.h"

#include "Systems/Sequence/Sequence.h"

#include <QJsonDocument>
#include <QUndoStack>
#include <QUuid>

DECLARE_EDITORWIDGET(CEditorPatternEditorWidget, EEditorWidget::ePatternEditor)

CEditorPatternEditorWidget::CEditorPatternEditorWidget(QWidget* pParent) :
  CEditorDebuggableWidget(pParent),
  m_spUi(std::make_unique<Ui::CEditorSequenceEditorWidget>()),
  m_spOverlayProps(std::make_unique<CSequencePropertiesOverlay>(this)),
  m_pDummyModel(new QStandardItemModel(this)),
  m_sLastCachedSequence(QString()),
  m_bChangingIndex(false)
{
  m_spUi->setupUi(this);
  m_spUi->pSceneView->setVisible(false);

  m_pFilteredScriptModel = new CFilteredEditorEditableFileModel(this);

  // set initial splitter sizes
  m_spUi->pBottomSplitter->setSizes(QList<int>() << height()/2 << height()/2);

  connect(m_spUi->pTimeLineWidget, &CTimelineWidget::SignalContentsChanged,
          this, &CEditorPatternEditorWidget::SlotContentsChange);
  connect(m_spOverlayProps.get(), &CSequencePropertiesOverlay::SignalContentsChanged,
          m_spUi->pTimeLineWidget, &CTimelineWidget::SlotUpdateSequenceProperties);
  connect(m_spOverlayProps.get(), &CSequencePropertiesOverlay::SignalContentsChanged,
          this, &CEditorPatternEditorWidget::SlotContentsChange);
}

CEditorPatternEditorWidget::~CEditorPatternEditorWidget()
{
  UnloadProject();

  m_spOverlayProps.reset();
}

//----------------------------------------------------------------------------------------
//
void CEditorPatternEditorWidget::Initialize()
{
  m_bInitialized = false;

  // initalize Debugger view
  CEditorDebuggableWidget::Initalize(m_spUi->pSceneView,
                                     std::bind(&CEditorPatternEditorWidget::GetSequenceScene, this));
  //connect(this, &CEditorDebuggableWidget::SignalDebugStarted,
  //        this, &CEditorPatternEditorWidget::SlotDebugStarted);
  //connect(this, &CEditorDebuggableWidget::SignalExecutionError,
  //        this, &CEditorPatternEditorWidget::SlotExecutionError);

  m_spUi->pTimeLineWidget->SetResourceModel(ResourceTreeModel());
  m_spUi->pTimeLineWidget->SetUndoStack(UndoStack());

  EditableFileModel()->SetReloadFileWithoutQuestion(true);
  m_pFilteredScriptModel->FilterForTypes({SScriptDefinitionData::c_sFileTypeSequence});

  m_spUi->pResourceComboBox->setModel(m_pDummyModel);
  m_pFilteredScriptModel->setSourceModel(m_pDummyModel);

  connect(EditableFileModel(), &CEditorEditableFileModel::SignalFileChangedExternally,
          this, &CEditorPatternEditorWidget::SlotFileChangedExternally);

  m_spOverlayProps->SetUndoStack(UndoStack());
  m_spOverlayProps->Hide();

  m_bInitialized = true;
}

//----------------------------------------------------------------------------------------
//
void CEditorPatternEditorWidget::LoadProject(tspProject spProject)
{
  WIDGET_INITIALIZED_GUARD

  auto pModel = EditableFileModel();
  m_pFilteredScriptModel->setSourceModel(pModel);
  m_spUi->pResourceComboBox->setModel(m_pFilteredScriptModel);

  m_spCurrentProject = spProject;

  m_spUi->pTimeLineWidget->Clear();

  //m_spUi->pTopSplitter->setSizes({ width()/4, width() *3/4 });
  m_spUi->pBottomSplitter->setSizes({ height()/2, height()/2 });

  if (0 < m_pFilteredScriptModel->rowCount())
  {
    on_pResourceComboBox_currentIndexChanged(0);
  }

  SetLoaded(true);
}

//----------------------------------------------------------------------------------------
//
void CEditorPatternEditorWidget::UnloadProjectImpl()
{
  WIDGET_INITIALIZED_GUARD

  m_spCurrentProject = nullptr;

  m_spCurrentSequence = nullptr;
  m_sLastCachedSequence = QString();

  m_spUi->pResourceComboBox->setModel(m_pDummyModel);
  m_spUi->pResourceComboBox->clear();

  m_spUi->pTimeLineWidget->SetSequence(nullptr);
  m_spOverlayProps->SetSequence(nullptr);
  m_spOverlayProps->SetSequenceName(QString());

  m_spOverlayProps->Hide();
}

//----------------------------------------------------------------------------------------
//
void CEditorPatternEditorWidget::SaveProject()
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }

  // save current contents
  auto pScriptItem = EditableFileModel()->CachedFile(
        CachedResourceName(m_spUi->pResourceComboBox->currentIndex()));
  if (nullptr != pScriptItem && nullptr != m_spCurrentSequence)
  {
    QJsonObject obj = m_spCurrentSequence->ToJsonObject();
    QJsonDocument doc = QJsonDocument(obj);
    pScriptItem->m_data = doc.toJson(QJsonDocument::Indented);
    EditableFileModel()->SetSceneScriptModifiedFlag(pScriptItem->m_sId, pScriptItem->m_bChanged);
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorPatternEditorWidget::OnHidden()
{
  EditableFileModel()->SetReloadFileWithoutQuestion(true);
}

//----------------------------------------------------------------------------------------
//
void CEditorPatternEditorWidget::OnShown()
{
  EditableFileModel()->SetReloadFileWithoutQuestion(false);
  ReloadEditor(m_spUi->pResourceComboBox->currentIndex());
}

//----------------------------------------------------------------------------------------
//
void CEditorPatternEditorWidget::OnActionBarAboutToChange()
{
  if (nullptr != ActionBar())
  {
    disconnect(ActionBar()->m_spUi->DebugLayoutButton, &QPushButton::clicked,
               this, &CEditorPatternEditorWidget::SlotDebugStart);
    disconnect(ActionBar()->m_spUi->StopDebugLayoutButton, &QPushButton::clicked,
               this, &CEditorPatternEditorWidget::SlotDebugStop);
    disconnect(ActionBar()->m_spUi->NewSequence, &QPushButton::clicked,
               this, &CEditorPatternEditorWidget::SlotAddNewSequenceButtonClicked);
    disconnect(ActionBar()->m_spUi->SequenceProperties, &QPushButton::clicked,
              this, &CEditorPatternEditorWidget::SlotEditSequenceButtonClicked);
    disconnect(ActionBar()->m_spUi->AddSequenceLayer, &QPushButton::clicked,
               this, &CEditorPatternEditorWidget::SlotAddSequenceLayerButtonClicked);
    disconnect(ActionBar()->m_spUi->RemoveSelectedSequenceLayer, &QPushButton::clicked,
               this, &CEditorPatternEditorWidget::SlotRemoveSequenceLayerButtonClicked);
    disconnect(ActionBar()->m_spUi->AddSequenceElement, &QPushButton::clicked,
               this, &CEditorPatternEditorWidget::SlotAddSequenceElementButtonClicked);
    disconnect(ActionBar()->m_spUi->RemoveSelectedSequenceElements, &QPushButton::clicked,
               this, &CEditorPatternEditorWidget::SlotRemoveSequenceElementsButtonClicked);

    ActionBar()->m_spUi->StopDebugLayoutButton->setEnabled(true);
    ActionBar()->m_spUi->NewSequence->setEnabled(true);
    ActionBar()->m_spUi->AddSequenceLayer->setEnabled(true);
    ActionBar()->m_spUi->RemoveSelectedSequenceLayer->setEnabled(true);
    ActionBar()->m_spUi->AddSequenceElement->setEnabled(true);
    ActionBar()->m_spUi->RemoveSelectedSequenceElements->setEnabled(true);

    UpdateButtons(nullptr, nullptr);
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorPatternEditorWidget::OnActionBarChanged()
{
  if (nullptr != ActionBar())
  {
    ActionBar()->ShowSequenceEditorActionBar();

    UpdateButtons(ActionBar()->m_spUi->DebugLayoutButton,
                  ActionBar()->m_spUi->StopDebugLayoutButton);

    connect(ActionBar()->m_spUi->DebugLayoutButton, &QPushButton::clicked,
            this, &CEditorPatternEditorWidget::SlotDebugStart);
    connect(ActionBar()->m_spUi->StopDebugLayoutButton, &QPushButton::clicked,
            this, &CEditorPatternEditorWidget::SlotDebugStop);
    connect(ActionBar()->m_spUi->NewSequence, &QPushButton::clicked,
            this, &CEditorPatternEditorWidget::SlotAddNewSequenceButtonClicked);
    connect(ActionBar()->m_spUi->SequenceProperties, &QPushButton::clicked,
            this, &CEditorPatternEditorWidget::SlotEditSequenceButtonClicked);
    connect(ActionBar()->m_spUi->AddSequenceLayer, &QPushButton::clicked,
            this, &CEditorPatternEditorWidget::SlotAddSequenceLayerButtonClicked);
    connect(ActionBar()->m_spUi->RemoveSelectedSequenceLayer, &QPushButton::clicked,
            this, &CEditorPatternEditorWidget::SlotRemoveSequenceLayerButtonClicked);
    connect(ActionBar()->m_spUi->AddSequenceElement, &QPushButton::clicked,
            this, &CEditorPatternEditorWidget::SlotAddSequenceElementButtonClicked);
    connect(ActionBar()->m_spUi->RemoveSelectedSequenceElements, &QPushButton::clicked,
            this, &CEditorPatternEditorWidget::SlotRemoveSequenceElementsButtonClicked);

    if (EditorModel()->IsReadOnly())
    {
      ActionBar()->m_spUi->NewSequence->setEnabled(false);
      ActionBar()->m_spUi->AddSequenceLayer->setEnabled(false);
      ActionBar()->m_spUi->RemoveSelectedSequenceLayer->setEnabled(false);
      ActionBar()->m_spUi->AddSequenceElement->setEnabled(false);
      ActionBar()->m_spUi->RemoveSelectedSequenceElements->setEnabled(false);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorPatternEditorWidget::on_pResourceComboBox_currentIndexChanged(qint32 iIndex)
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }

  if (CachedResourceName(iIndex) != m_sLastCachedSequence)
  {
    UndoStack()->push(new CCommandChangeOpenedSequence(m_spUi->pResourceComboBox,
                                                       m_spUi->pTimeLineWidget,
                                                       this,
                                                       std::bind(&CEditorPatternEditorWidget::ReloadEditor, this, std::placeholders::_1),
                                                       &m_bChangingIndex, &m_sLastCachedSequence,
                                                       m_sLastCachedSequence,
                                                       CachedResourceName(iIndex)));
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorPatternEditorWidget::SlotAddNewSequenceButtonClicked()
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }

  UndoStack()->push(new CCommandAddNewSequence(m_spCurrentProject, this));
}

//----------------------------------------------------------------------------------------
//
void CEditorPatternEditorWidget::SlotEditSequenceButtonClicked()
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject || nullptr == m_spCurrentSequence) { return; }

  m_spOverlayProps->Toggle();
}

//----------------------------------------------------------------------------------------
//
void CEditorPatternEditorWidget::SlotAddSequenceLayerButtonClicked()
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }
  m_spUi->pTimeLineWidget->AddNewLayer();
}

//----------------------------------------------------------------------------------------
//
void CEditorPatternEditorWidget::SlotContentsChange()
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject || nullptr == m_spCurrentSequence) { return; }

  QString sCachedScript = CachedResourceName(
        m_spUi->pResourceComboBox->currentIndex());
  auto pScriptItem = EditableFileModel()->CachedFile(sCachedScript);
  if (nullptr != pScriptItem)
  {
    pScriptItem->m_bChanged = true;
    QJsonObject obj = m_spCurrentSequence->ToJsonObject();
    QJsonDocument doc = QJsonDocument(obj);
    pScriptItem->m_data = doc.toJson(QJsonDocument::Indented);
    EditableFileModel()->SetSceneScriptModifiedFlag(pScriptItem->m_sId, true);
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorPatternEditorWidget::SlotFileChangedExternally(const QString& sName)
{
  qint32 index =
      m_pFilteredScriptModel->mapToSource(
          m_pFilteredScriptModel->index(m_spUi->pResourceComboBox->currentIndex(), 0)).row();
  auto pScriptItem = EditableFileModel()->CachedFile(sName);
  if (index == EditableFileModel()->FileIndex(sName) && nullptr != pScriptItem)
  {
    m_bChangingIndex = true;

    m_spUi->pTimeLineWidget->SetSequence(nullptr);
    m_spOverlayProps->SetSequence(nullptr);
    m_spOverlayProps->SetSequenceName(QString());
    // load new contents
    if (nullptr != m_spCurrentSequence)
    {
      m_spCurrentSequence->FromJsonObject(QJsonDocument::fromJson(pScriptItem->m_data).object());
      m_spUi->pTimeLineWidget->SetSequence(m_spCurrentSequence);
      m_spOverlayProps->SetSequence(m_spCurrentSequence);
      m_spOverlayProps->SetSequenceName(sName);
    }

    m_bChangingIndex = false;
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorPatternEditorWidget::SlotRemoveSequenceLayerButtonClicked()
{
  WIDGET_INITIALIZED_GUARD
      if (nullptr == m_spCurrentProject) { return; }
  m_spUi->pTimeLineWidget->RemoveSelectedLayer();
}

//----------------------------------------------------------------------------------------
//
void CEditorPatternEditorWidget::SlotAddSequenceElementButtonClicked()
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }
  if (EditorModel()->IsReadOnly()) { return; }

  m_spUi->pTimeLineWidget->SlotOpenInsertContextMenuRequested(
      m_spUi->pTimeLineWidget->SelectedIndex(),
      m_spUi->pTimeLineWidget->SelectedTimeStamp(),
      mapToGlobal(QPoint(0, 0)));
}

//----------------------------------------------------------------------------------------
//
void CEditorPatternEditorWidget::SlotRemoveSequenceElementsButtonClicked()
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }

  m_spUi->pTimeLineWidget->RemoveSelectedElement();
}

//----------------------------------------------------------------------------------------
//
QString CEditorPatternEditorWidget::CachedResourceName(qint32 iIndex)
{
  return EditableFileModel()->CachedResourceName(
      m_pFilteredScriptModel->mapToSource(
          m_pFilteredScriptModel->index(iIndex, 0)).row());
}

//----------------------------------------------------------------------------------------
//
CEditorPatternEditorWidget::tSceneToDebug CEditorPatternEditorWidget::GetSequenceScene()
{
  if (!IsInitialized()) { return nullptr; }
  if (nullptr == m_spCurrentProject) { return nullptr; }

  // we create a mock-scene that will get injected into the runner
  std::shared_ptr<SScene> spMockScene = std::make_shared<SScene>();
  spMockScene->m_spParent = m_spCurrentProject;

  QReadLocker locker(&m_spCurrentProject->m_rwLock);
  spMockScene->m_sSceneLayout = m_spCurrentProject->m_sPlayerLayout;
  spMockScene->m_sName = QUuid::createUuid().toString();
  spMockScene->m_iId = INT_MAX;
  spMockScene->m_sScript = m_sLastCachedSequence;
  return spMockScene;
}

//----------------------------------------------------------------------------------------
//
void CEditorPatternEditorWidget::ReloadEditor(qint32 iIndex)
{
  m_bChangingIndex = true;

  m_spUi->pTimeLineWidget->SetSequence(nullptr);
  m_spOverlayProps->SetSequence(nullptr);
  m_spOverlayProps->SetSequenceName(QString());

  // load new contents
  m_sLastCachedSequence = CachedResourceName(iIndex);
  auto  pScriptItem = EditableFileModel()->CachedFile(m_sLastCachedSequence);
  if (nullptr != pScriptItem)
  {
    if (nullptr != ActionBar())
    {
      ActionBar()->m_spUi->DebugLayoutButton->setEnabled(!m_sLastCachedSequence.isEmpty());
    }
    if (nullptr == m_spCurrentSequence)
    {
      m_spCurrentSequence = std::make_shared<SSequenceFile>();
    }
    m_spCurrentSequence->FromJsonObject(QJsonDocument::fromJson(pScriptItem->m_data).object());
    m_spUi->pTimeLineWidget->SetSequence(m_spCurrentSequence);
    m_spOverlayProps->SetSequence(m_spCurrentSequence);
    m_spOverlayProps->SetSequenceName(m_sLastCachedSequence);
  }

  m_bChangingIndex = false;
}
