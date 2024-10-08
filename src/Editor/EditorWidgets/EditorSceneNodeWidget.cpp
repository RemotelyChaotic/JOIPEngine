#include "EditorSceneNodeWidget.h"
#include "Application.h"

#include "Editor/EditorActionBar.h"
#include "Editor/EditorModel.h"
#include "Editor/NodeEditor/FlowScene.h"
#include "Editor/NodeEditor/FlowView.h"
#include "Editor/Tutorial/SceneNodeWidgetTutorialStateSwitchHandler.h"

#include "Systems/HelpFactory.h"
#include "Systems/Project.h"
#include "Systems/Scene.h"
#include "Systems/Resource.h"

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
    const QColor& hoveredColor)
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
                                      .arg(ColorToString(connectionPointColor)));

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

  m_pFlowView = new CFlowView(this);
  m_pFlowView->setObjectName("FlowView");
  m_pFlowView->setWindowTitle("Node-based flow editor");
  m_pFlowView->show();

  QLayout* pLayout = m_spUi->pContainerWidget->layout();
  pLayout->addWidget(m_pFlowView);
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
  m_pFlowView->resetTransform();
  m_pFlowView->scale(0.8, 0.8);
  m_pFlowView->centerOn(0,0);

  m_spStateSwitchHandler =
      std::make_shared<CSceneNodeWidgetTutorialStateSwitchHandler>(this, m_spUi);
  EditorModel()->AddTutorialStateSwitchHandler(m_spStateSwitchHandler);

  connect(CApplication::Instance(), &CApplication::StyleLoaded,
          this, &CEditorSceneNodeWidget::SlotStyleChanged);
  QMetaObject::invokeMethod(this, "SlotStyleChanged", Qt::QueuedConnection);

  auto wpHelpFactory = CApplication::Instance()->System<CHelpFactory>().lock();
  if (nullptr != wpHelpFactory)
  {
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

  m_pFlowView->SetReadOnly(EditorModel()->IsReadOnly());

  SetLoaded(true);
}

//----------------------------------------------------------------------------------------
//
void CEditorSceneNodeWidget::UnloadProject()
{
  WIDGET_INITIALIZED_GUARD

  m_spCurrentProject = nullptr;

  m_pFlowView->resetTransform();
  m_pFlowView->scale(0.8, 0.8);
  m_pFlowView->centerOn(0,0);

  m_pFlowView->SetReadOnly(false);

  SetLoaded(false);
}

//----------------------------------------------------------------------------------------
//
void CEditorSceneNodeWidget::SaveProject()
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }
}

//----------------------------------------------------------------------------------------
//
void CEditorSceneNodeWidget::OnActionBarAboutToChange()
{
  if (nullptr != ActionBar())
  {
    disconnect(ActionBar()->m_spUi->AddNodeButton, &QPushButton::clicked,
            this, &CEditorSceneNodeWidget::SlotAddSceneButtonClicked);
    disconnect(ActionBar()->m_spUi->RemoveNodeButton, &QPushButton::clicked,
            this, &CEditorSceneNodeWidget::SlotRemoveNodeButtonClicked);

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
    connect(ActionBar()->m_spUi->AddNodeButton, &QPushButton::clicked,
            this, &CEditorSceneNodeWidget::SlotAddSceneButtonClicked);
    connect(ActionBar()->m_spUi->RemoveNodeButton, &QPushButton::clicked,
            this, &CEditorSceneNodeWidget::SlotRemoveNodeButtonClicked);

    if (EditorModel()->IsReadOnly())
    {
      ActionBar()->m_spUi->AddNodeButton->setEnabled(false);
      ActionBar()->m_spUi->RemoveNodeButton->setEnabled(false);
    }
  }
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
               m_hoveredColor);
}

