#include "EditorSceneNodeWidget.h"
#include "Application.h"

#include "Editor/EditorActionBar.h"
#include "Editor/EditorEditableFileModel.h"
#include "Editor/EditorModel.h"
#include "Editor/NodeEditor/CommandAddNewFlow.h"
#include "Editor/NodeEditor/CommandChangeOpenedFlow.h"
#include "Editor/NodeEditor/NodeEditorFlowView.h"
#include "Editor/NodeEditor/NodeEditorFlowScene.h"
#include "Editor/NodeEditor/UndoPathSplitterModel.h"
#include "Editor/Resources/ResourceTreeItemModel.h"
#include "Editor/Tutorial/SceneNodeWidgetTutorialStateSwitchHandler.h"

#include "Systems/HelpFactory.h"
#include "Systems/Database/Project.h"
#include "Systems/Database/Scene.h"
#include "Systems/Database/Resource.h"

#include "Systems/Nodes/SceneNodeModel.h"
#include "Systems/Nodes/StartNodeModel.h"
#include "Systems/Nodes/SubflowNodeModel.h"

#include "Widgets/HelpOverlay.h"

#include <nodes/ConnectionStyle>
#include <nodes/FlowView>
#include <nodes/FlowViewStyle>
#include <nodes/Node>
#include <nodes/NodeStyle>

#include <QContextMenuEvent>
#include <QDebug>
#include <QFile>
#include <QFileDialog>

#include <memory>

using QtNodes::ConnectionStyle;
using QtNodes::FlowView;
using QtNodes::FlowViewStyle;
using QtNodes::Node;
using QtNodes::NodeStyle;

DECLARE_EDITORWIDGET(CEditorSceneNodeWidget, EEditorWidget::eSceneNodeWidget)

namespace
{
  const QString c_sDebugNodeHelpId = "Editor/DebugNode";
  const QString c_sNodeHelpId =      "Editor/NodeHelp";
  const QString c_sNodeNodeHelpId =  "Editor/Node/Node";

  QString ColorToString(const QColor& color)
  {
    return QString("[%1,%2,%3]").arg(color.red()).arg(color.green()).arg(color.blue());
  }

  void SetNodeStyle(
    const QColor& normalBoundaryColor,
    const QColor& selectedBoundaryColor,
    const QColor& gradientColor0,
    const QColor& gradientColor1,
    const QColor& gradientColor2,
    const QColor& gradientColor3,
    const QColor& shadowColor,
    const QColor& fontColor,
    const QColor& fontColorFaded,
    const QColor& connectionPointColor,
    const QColor& backgroundColor,
    const QColor& fineGridColor,
    const QColor& coarseGridColor,
    const QColor& normalColor,
    const QColor& selectedColor,
    const QColor& selectedHaloColor,
    const QColor& hoveredColor,
    const QColor& warningColor,
    const QColor& errorColor)
  {
    QString sStyleNode(QStringLiteral(
    R"(
    {
      "NodeStyle": {
        "NormalBoundaryColor": %1,
        "SelectedBoundaryColor": %2,
        "GradientColor0": %3,
        "GradientColor1": %4,
        "GradientColor2": %5,
        "GradientColor3": %6,
        "ShadowColor": %7,
        "FontColor": %8,
        "FontColorFaded": %9,
        "ConnectionPointColor": %10,
        "WarningColor": %11,
        "ErrorColor": %12,
        "PenWidth": 2.0,
        "HoveredPenWidth": 2.5,
        "ConnectionPointDiameter": 10.0,
        "Opacity": 1.0
      }
    }
    )"));
    NodeStyle::setNodeStyle(sStyleNode.arg(ColorToString(normalBoundaryColor),
                                           ColorToString(selectedBoundaryColor),
                                           ColorToString(gradientColor0),
                                           ColorToString(gradientColor1),
                                           ColorToString(gradientColor2),
                                           ColorToString(gradientColor3),
                                           ColorToString(shadowColor),
                                           ColorToString(fontColor),
                                           ColorToString(fontColorFaded))
                                      .arg(ColorToString(connectionPointColor))
                                      .arg(ColorToString(warningColor))
                                      .arg(ColorToString(errorColor)));

    QString sStyleView(QStringLiteral(
      R"(
      {
        "FlowViewStyle": {
            "BackgroundColor": %1,
            "FineGridColor": %2,
            "CoarseGridColor": %3
        }
      }
      )"
    ));
    FlowViewStyle::setStyle(sStyleView.arg(ColorToString(backgroundColor),
                                           ColorToString(fineGridColor),
                                           ColorToString(coarseGridColor)));

    QString sStyleConnection(QStringLiteral(
    R"(
    {
      "ConnectionStyle": {
        "ConstructionColor": "gray",
        "NormalColor": %1,
        "SelectedColor": %2,
        "SelectedHaloColor": %3,
        "HoveredColor": %4,

        "LineWidth": 3.0,
        "ConstructionLineWidth": 2.0,
        "PointDiameter": 10.0,

        "UseDataDefinedColors": false
      }
    }
    )"));
    ConnectionStyle::setConnectionStyle(sStyleConnection.arg(ColorToString(normalColor),
                                                             ColorToString(selectedColor),
                                                             ColorToString(selectedHaloColor),
                                                             ColorToString(hoveredColor)));
  }
}

//----------------------------------------------------------------------------------------
//
CEditorSceneNodeWidget::CEditorSceneNodeWidget(QWidget* pParent) :
  CEditorWidgetBase(pParent),
  m_spUi(std::make_shared<Ui::CEditorSceneNodeWidget>()),
  m_spStateSwitchHandler(nullptr),
  m_spSettings(CApplication::Instance()->Settings()),
  m_spCurrentProject(nullptr),
  m_pFlowView(nullptr)
{
  m_spUi->setupUi(this);

  m_spUi->pDebugView->hide();

  QLayout* pLayout = m_spUi->pContainerWidget->layout();

  m_pFlowView = new CNodeEditorFlowView(m_spUi->pContainerWidget);
  m_pFlowView->setObjectName("FlowView");
  m_pFlowView->resetTransform();
  m_pFlowView->scale(0.8, 0.8);
  m_pFlowView->centerOn(0,0);

  pLayout->addWidget(m_pFlowView);

  m_pFilteredScriptModel = new CFilteredEditorEditableFileModel(this);
}

CEditorSceneNodeWidget::~CEditorSceneNodeWidget()
{
  UnloadProject();
  m_pFlowView->setScene(nullptr);
}

//----------------------------------------------------------------------------------------
//
void CEditorSceneNodeWidget::Initialize()
{
  m_bInitialized = false;

  m_wpDbManager = CApplication::Instance()->System<CDatabaseManager>();

  m_pFlowView->SetUndoStack(UndoStack());
  m_pFlowView->SetScene(FlowSceneModel());

  m_pFilteredScriptModel->FilterForTypes({SScriptDefinitionData::c_sFileTypeFlow});

  m_spUi->pResourceComboBox->setModel(m_pDummyModel);
  m_pFilteredScriptModel->setSourceModel(m_pDummyModel);

  connect(EditableFileModel(), &CEditorEditableFileModel::SignalFileChangedExternally,
          this, &CEditorSceneNodeWidget::SlotFileChangedExternally);
  connect(FlowSceneModel(), &CFlowScene::nodeCreated,
          this, &CEditorSceneNodeWidget::SlotNodeCreated);
  connect(FlowSceneModel(), &CFlowScene::nodeDeleted,
          this, &CEditorSceneNodeWidget::SlotNodeDeleted);
  connect(FlowSceneModel(), &CNodeEditorFlowScene::SignalFlowModified,
          this, [this](){
    if (!FlowSceneModel()->IsLoading())
    {
      EditableFileModel()->SetSceneScriptModifiedFlag(m_sLastCachedFlow, true);
    }
  });

  m_spUi->pDebugView->Initialize(m_pFlowView, FlowSceneModel());

  m_spStateSwitchHandler =
      std::make_shared<CSceneNodeWidgetTutorialStateSwitchHandler>(this, m_spUi);
  EditorModel()->AddTutorialStateSwitchHandler(m_spStateSwitchHandler);

  connect(CApplication::Instance(), &CApplication::StyleLoaded,
          this, &CEditorSceneNodeWidget::SlotStyleChanged);
  QMetaObject::invokeMethod(this, "SlotStyleChanged", Qt::QueuedConnection);

  auto wpHelpFactory = CApplication::Instance()->System<CHelpFactory>().lock();
  if (nullptr != wpHelpFactory)
  {
    m_spUi->pDebugView->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sDebugNodeHelpId);
    wpHelpFactory->RegisterHelp(c_sDebugNodeHelpId, ":/resources/help/editor/nodes/nodeeditor_debug_help.html");
    m_pFlowView->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sNodeHelpId);
    wpHelpFactory->RegisterHelp(c_sNodeHelpId, ":/resources/help/editor/nodes/nodeeditor_help.html");

    wpHelpFactory->RegisterHelp(c_sNodeNodeHelpId, ":/resources/help/editor/nodes/node_help.html");
  }

  m_bInitialized = true;
}

//----------------------------------------------------------------------------------------
//
void CEditorSceneNodeWidget::LoadProject(tspProject spCurrentProject)
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr != m_spCurrentProject)
  {
    assert(false);
    qWarning() << "Old Project was not unloaded before loading project.";
  }

  m_spCurrentProject = spCurrentProject;
  SlotStyleChanged();

  auto pModel = EditableFileModel();
  m_pFilteredScriptModel->setSourceModel(pModel);
  m_spUi->pResourceComboBox->setModel(m_pFilteredScriptModel);

  if (0 < m_pFilteredScriptModel->rowCount())
  {
    on_pResourceComboBox_currentIndexChanged(0);
  }

  m_pFlowView->SetReadOnly(EditorModel()->IsReadOnly());

  SetLoaded(true);
}

//----------------------------------------------------------------------------------------
//
void CEditorSceneNodeWidget::UnloadProject()
{
  WIDGET_INITIALIZED_GUARD

  m_spUi->pDebugView->StopDebug();

  m_spCurrentProject = nullptr;
  m_sLastCachedFlow = QString();

  m_pFlowView->resetTransform();
  m_pFlowView->scale(0.8, 0.8);
  m_pFlowView->centerOn(0,0);

  m_pFlowView->SetReadOnly(false);

  m_spUi->pResourceComboBox->setModel(m_pDummyModel);
  m_spUi->pResourceComboBox->clear();

  SetLoaded(false);
}

//----------------------------------------------------------------------------------------
//
void CEditorSceneNodeWidget::SaveProject()
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }

  // save current contents
  auto pScriptItem = EditableFileModel()->CachedFile(
      CachedResourceName(m_spUi->pResourceComboBox->currentIndex()));
  if (nullptr != pScriptItem && nullptr != m_sLastCachedFlow)
  {
    pScriptItem->m_data = FlowSceneModel()->saveToMemory();
    EditableFileModel()->SetSceneScriptModifiedFlag(pScriptItem->m_sId, pScriptItem->m_bChanged);
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorSceneNodeWidget::OnHidden()
{
  EditableFileModel()->SetReloadFileWithoutQuestion(true);
}

//----------------------------------------------------------------------------------------
//
void CEditorSceneNodeWidget::OnShown()
{
  EditableFileModel()->SetReloadFileWithoutQuestion(false);
  ReloadEditor(
    m_pFilteredScriptModel->mapToSource(
      m_pFilteredScriptModel->index(m_spUi->pResourceComboBox->currentIndex(), 0)).row());
}

//----------------------------------------------------------------------------------------
//
void CEditorSceneNodeWidget::OnActionBarAboutToChange()
{
  m_spUi->pDebugView->StopDebug();

  if (nullptr != ActionBar())
  {
    disconnect(ActionBar()->m_spUi->CreateNewFlow, &QPushButton::clicked,
               this, &CEditorSceneNodeWidget::SlotCreateNewFlowClicked);
    disconnect(ActionBar()->m_spUi->DebugNodeButton, &QPushButton::clicked,
               this, &CEditorSceneNodeWidget::SlotStartDebugClicked);
    disconnect(ActionBar()->m_spUi->StopDebugNodeButton, &QPushButton::clicked,
               this, &CEditorSceneNodeWidget::SlotStopDebugClicked);
    disconnect(ActionBar()->m_spUi->NextSceneButton, &QPushButton::clicked,
               this, &CEditorSceneNodeWidget::SlotNextSceneClicked);
    disconnect(ActionBar()->m_spUi->AddNodeButton, &QPushButton::clicked,
            this, &CEditorSceneNodeWidget::SlotAddSceneButtonClicked);
    disconnect(ActionBar()->m_spUi->RemoveNodeButton, &QPushButton::clicked,
            this, &CEditorSceneNodeWidget::SlotRemoveNodeButtonClicked);

    ActionBar()->m_spUi->CreateNewFlow->setEnabled(true);
    ActionBar()->m_spUi->NextSceneButton->setEnabled(false);
    ActionBar()->m_spUi->AddNodeButton->setEnabled(true);
    ActionBar()->m_spUi->RemoveNodeButton->setEnabled(true);
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorSceneNodeWidget::OnActionBarChanged()
{
  // connections for actionbar
  if (nullptr != ActionBar())
  {
    ActionBar()->ShowNodeEditorActionBar();

    connect(ActionBar()->m_spUi->CreateNewFlow, &QPushButton::clicked,
            this, &CEditorSceneNodeWidget::SlotCreateNewFlowClicked);
    connect(ActionBar()->m_spUi->DebugNodeButton, &QPushButton::clicked,
            this, &CEditorSceneNodeWidget::SlotStartDebugClicked);
    connect(ActionBar()->m_spUi->StopDebugNodeButton, &QPushButton::clicked,
            this, &CEditorSceneNodeWidget::SlotStopDebugClicked);
    connect(ActionBar()->m_spUi->NextSceneButton, &QPushButton::clicked,
            this, &CEditorSceneNodeWidget::SlotNextSceneClicked);
    connect(ActionBar()->m_spUi->AddNodeButton, &QPushButton::clicked,
            this, &CEditorSceneNodeWidget::SlotAddSceneButtonClicked);
    connect(ActionBar()->m_spUi->RemoveNodeButton, &QPushButton::clicked,
            this, &CEditorSceneNodeWidget::SlotRemoveNodeButtonClicked);

    ActionBar()->m_spUi->NextSceneButton->setEnabled(false);
    if (EditorModel()->IsReadOnly())
    {
      ActionBar()->m_spUi->CreateNewFlow->setEnabled(false);
      ActionBar()->m_spUi->AddNodeButton->setEnabled(false);
      ActionBar()->m_spUi->RemoveNodeButton->setEnabled(false);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorSceneNodeWidget::on_pResourceComboBox_currentIndexChanged(qint32 iIndex)
{
  WIDGET_INITIALIZED_GUARD
      if (nullptr == m_spCurrentProject) { return; }

  if (CachedResourceName(iIndex) != m_sLastCachedFlow)
  {
    UndoStack()->push(new CCommandChangeOpenedFlow(m_spUi->pResourceComboBox,
                                                   FlowSceneModel(),
                                                   this,
                                                   std::bind(&CEditorSceneNodeWidget::ReloadEditor, this, std::placeholders::_1),
                                                   &m_bChangingIndex, &m_sLastCachedFlow,
                                                   m_sLastCachedFlow,
                                                   CachedResourceName(iIndex)));
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorSceneNodeWidget::SlotAddNewScriptFileToScene(const QString& sCustomInitContent)
{
  if (nullptr == m_spCurrentProject)
  {
    qWarning() << "Node created in null-project.";
    return;
  }

  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    CSceneNodeModel* pSceneModel = dynamic_cast<CSceneNodeModel*>(sender());
    if (nullptr != pSceneModel)
    {
      qint32 iSceneId = pSceneModel->SceneId();
      auto spScene = spDbManager->FindScene(m_spCurrentProject, iSceneId);
      EditorModel()->AddNewFileToScene(this, spScene, EResourceType::eScript, sCustomInitContent);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorSceneNodeWidget::SlotAddNewLayoutFileToScene(const QString& sCustomInitContent)
{
  if (nullptr == m_spCurrentProject)
  {
    qWarning() << "Node created in null-project.";
    return;
  }

  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    CSceneNodeModel* pSceneModel = dynamic_cast<CSceneNodeModel*>(sender());
    if (nullptr != pSceneModel)
    {
      qint32 iSceneId = pSceneModel->SceneId();
      auto spScene = spDbManager->FindScene(m_spCurrentProject, iSceneId);
      EditorModel()->AddNewFileToScene(this, spScene, EResourceType::eLayout, sCustomInitContent);
    }
    CPathSplitterModel* pSplitterModel = dynamic_cast<CPathSplitterModel*>(sender());
    if (nullptr != pSplitterModel)
    {
      EditorModel()->AddNewFileToScene(this, nullptr, EResourceType::eLayout, sCustomInitContent);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorSceneNodeWidget::SlotFileChangedExternally(const QString& sName)
{
  qint32 index =
      m_pFilteredScriptModel->mapToSource(
                                m_pFilteredScriptModel->index(m_spUi->pResourceComboBox->currentIndex(), 0)).row();
  auto pFlowItem = EditableFileModel()->CachedFile(sName);

  if (index == EditableFileModel()->FileIndex(sName))
  {
    m_bChangingIndex = true;

    {
      QSignalBlocker b(FlowSceneModel());
      FlowSceneModel()->clearScene();
    }
    FlowSceneModel()->loadFromMemory(pFlowItem->m_data);

    m_bChangingIndex = false;
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorSceneNodeWidget::SlotCreateNewFlowClicked()
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }

  UndoStack()->push(new CCommandAddNewFlow(m_spCurrentProject, this));
}

//----------------------------------------------------------------------------------------
//
void CEditorSceneNodeWidget::SlotStartDebugClicked()
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }
  std::vector<QtNodes::Node*> vpNodes = FlowSceneModel()->selectedNodes();
  if (!vpNodes.empty())
  {
    QtNodes::Node* pNode = vpNodes.front();
    if (dynamic_cast<CStartNodeModel*>(pNode->nodeDataModel()))
    {
      m_spUi->pDebugView->StartDebug(m_spCurrentProject, pNode->id());
    }
    else if (auto pSceneModel = dynamic_cast<CSceneNodeModel*>(vpNodes.front()->nodeDataModel()))
    {
      m_spUi->pDebugView->StartDebug(m_spCurrentProject, pSceneModel->SceneName());
    }
    else
    {
      m_spUi->pDebugView->StartDebug(m_spCurrentProject, pNode->id());
    }
  }
  else
  {
    m_spUi->pDebugView->StartDebug(m_spCurrentProject, QString());
  }

  if (nullptr != ActionBar())
  {
    ActionBar()->m_spUi->pNodeDebugStack->setCurrentIndex(1);
    ActionBar()->m_spUi->NextSceneButton->setEnabled(true);
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorSceneNodeWidget::SlotStopDebugClicked()
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }

  m_spUi->pDebugView->StopDebug();

  if (nullptr != ActionBar())
  {
    ActionBar()->m_spUi->pNodeDebugStack->setCurrentIndex(0);
    ActionBar()->m_spUi->NextSceneButton->setEnabled(false);
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorSceneNodeWidget::SlotNodeCreated(Node &n)
{
  if (nullptr == m_spCurrentProject)
  {
    qWarning() << "Node created in null-project.";
    return;
  }

  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    m_spCurrentProject->m_rwLock.lockForRead();
    qint32 iId = m_spCurrentProject->m_iId;
    m_spCurrentProject->m_rwLock.unlock();

    CSceneNodeModel* pSceneModel = dynamic_cast<CSceneNodeModel*>(n.nodeDataModel());
    if (nullptr != pSceneModel)
    {
      pSceneModel->SetProjectId(iId);
      pSceneModel->SetResourceItemModel(ResourceTreeModel());
      connect(pSceneModel, &CSceneNodeModel::SignalAddScriptFileRequested,
              this, &CEditorSceneNodeWidget::SlotAddNewScriptFileToScene, Qt::UniqueConnection);
      connect(pSceneModel, &CSceneNodeModel::SignalAddLayoutFileRequested,
              this, &CEditorSceneNodeWidget::SlotAddNewLayoutFileToScene, Qt::UniqueConnection);
    }
    CPathSplitterModel* pPathSplitterModel = dynamic_cast<CPathSplitterModel*>(n.nodeDataModel());
    if (nullptr != pPathSplitterModel)
    {
      pPathSplitterModel->SetProjectId(iId);
      connect(pPathSplitterModel, &CPathSplitterModel::SignalAddLayoutFileRequested,
              this, &CEditorSceneNodeWidget::SlotAddNewLayoutFileToScene, Qt::UniqueConnection);
    }
    CSubflowNodeModel* pSubflowModel = dynamic_cast<CSubflowNodeModel*>(n.nodeDataModel());
    if (nullptr != pSubflowModel)
    {
      pSubflowModel->SetProjectId(iId);
      connect(pSubflowModel, &CSubflowNodeModel::SignalAddNodeFileRequested,
              this, &CEditorSceneNodeWidget::SlotCreateNewFlowClicked, Qt::UniqueConnection);
    }

    if (!FlowSceneModel()->IsLoading())
    {
      EditableFileModel()->SetSceneScriptModifiedFlag(m_sLastCachedFlow, true);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorSceneNodeWidget::SlotNodeDeleted(QtNodes::Node &n)
{
  if (nullptr == m_spCurrentProject)
  {
    qWarning() << "Node created in null-project.";
    return;
  }

  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    CSceneNodeModel* pSceneModel = dynamic_cast<CSceneNodeModel*>(n.nodeDataModel());
    if (nullptr != pSceneModel)
    {
      qint32 iSceneId = pSceneModel->SceneId();
      spDbManager->RemoveScene(m_spCurrentProject, iSceneId);
    }

    if (!FlowSceneModel()->IsLoading())
    {
      EditableFileModel()->SetSceneScriptModifiedFlag(m_sLastCachedFlow, true);
    }
  }
}


//----------------------------------------------------------------------------------------
//
void CEditorSceneNodeWidget::SlotNextSceneClicked()
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }
  m_spUi->pDebugView->NextScene();
}


//----------------------------------------------------------------------------------------
//
void CEditorSceneNodeWidget::SlotAddSceneButtonClicked()
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }
  m_pFlowView->OpenContextMenuAt(QPoint(0, 0), QPoint(0, 0));
}

//----------------------------------------------------------------------------------------
//
void CEditorSceneNodeWidget::SlotRemoveNodeButtonClicked()
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }

  auto vpNodes = FlowSceneModel()->selectedNodes();
  for (Node* pNode : vpNodes)
  {
    if (nullptr != pNode)
    {
      FlowSceneModel()->removeNode(*pNode);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorSceneNodeWidget::SlotStyleChanged()
{
  WIDGET_INITIALIZED_GUARD

  SetNodeStyle(m_normalBoundaryColor,
               m_selectedBoundaryColor,
               m_gradientColor0,
               m_gradientColor1,
               m_gradientColor2,
               m_gradientColor3,
               m_shadowColor,
               m_fontColor,
               m_fontColorFaded,
               m_connectionPointColor,
               m_backgroundColor,
               m_fineGridColor,
               m_coarseGridColor,
               m_normalColor,
               m_selectedColor,
               m_selectedHaloColor,
               m_hoveredConnectionColor,
               m_warningColor,
               m_errorColor);
}

//----------------------------------------------------------------------------------------
//
QString CEditorSceneNodeWidget::CachedResourceName(qint32 iIndex)
{
  return EditableFileModel()->CachedResourceName(
      m_pFilteredScriptModel->mapToSource(
                                m_pFilteredScriptModel->index(iIndex, 0)).row());
}

//----------------------------------------------------------------------------------------
//
void CEditorSceneNodeWidget::ReloadEditor(qint32 iIndex)
{
  m_bChangingIndex = true;

  // load new contents
  m_sLastCachedFlow = CachedResourceName(iIndex);
  auto  pFlowItem = EditableFileModel()->CachedFile(m_sLastCachedFlow);
  if (nullptr != pFlowItem)
  {
    if (nullptr != ActionBar())
    {
      ActionBar()->m_spUi->DebugLayoutButton->setEnabled(!m_sLastCachedFlow.isEmpty());
    }

    {
      QSignalBlocker b(FlowSceneModel());
      FlowSceneModel()->clearScene();
    }
    FlowSceneModel()->loadFromMemory(pFlowItem->m_data);
  }

  m_bChangingIndex = false;
}

