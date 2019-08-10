#include "EditorSceneNodeWidget.h"
#include "Application.h"
#include "EditorActionBar.h"
#include "Backend/Project.h"
#include "Backend/Scene.h"
#include "Backend/Resource.h"
#include "NodeEditor/EndNodeModel.h"
#include "NodeEditor/SceneNodeModel.h"
#include "NodeEditor/SceneTranstitionData.h"
#include "NodeEditor/StartNodeModel.h"
#include "ui_EditorSceneNodeWidget.h"
#include "ui_EditorActionBar.h"

#include <nodes/ConnectionStyle>
#include <nodes/DataModelRegistry>
#include <nodes/FlowScene>
#include <nodes/FlowView>
#include <nodes/NodeData>

#include <QDebug>
#include <QFile>

#include <memory>

using QtNodes::ConnectionStyle;
using QtNodes::DataModelRegistry;
using QtNodes::FlowScene;
using QtNodes::FlowView;

namespace
{
  std::shared_ptr<DataModelRegistry> RegisterDataModels()
  {
    auto ret = std::make_shared<DataModelRegistry>();
    ret->registerModel<CStartNodeModel>("Scene");
    ret->registerModel<CSceneNodeModel>("Scene");
    ret->registerModel<CEndNodeModel>("Scene");
    return ret;
  }

  void SetNodeStyle()
  {
    ConnectionStyle::setConnectionStyle(
    R"(
    {
      "ConnectionStyle": {
        "ConstructionColor": "gray",
        "NormalColor": "black",
        "SelectedColor": "gray",
        "SelectedHaloColor": "deepskyblue",
        "HoveredColor": "deepskyblue",

        "LineWidth": 3.0,
        "ConstructionLineWidth": 2.0,
        "PointDiameter": 10.0,

        "UseDataDefinedColors": true
      }
    }
    )");
  }
}

//----------------------------------------------------------------------------------------
//
CEditorSceneNodeWidget::CEditorSceneNodeWidget(QWidget* pParent) :
  CEditorWidgetBase(pParent),
  m_spUi(new Ui::CEditorSceneNodeWidget),
  m_spSettings(CApplication::Instance()->Settings()),
  m_spDataModelRegistry(RegisterDataModels()),
  m_spCurrentProject(nullptr),
  m_pFlowView(nullptr),
  m_pFlowScene(nullptr)
{
  m_spUi->setupUi(this);

  SetNodeStyle();

  m_pFlowView = new FlowView(this);
  m_pFlowView->setWindowTitle("Node-based flow editor");
  m_pFlowView->show();
  m_pFlowView->setScene(nullptr);

  QLayout* pLayout = m_spUi->pContainerWidget->layout();
  pLayout->addWidget(m_pFlowView);
}

CEditorSceneNodeWidget::~CEditorSceneNodeWidget()
{
  UnloadProject();
  // leads to crash on destroying widget, so we let it leak,
  // should not be a problem, since 'this' is only created once and destroyed only on
  // application exit, in which case the leak doesn't matter.
  // TODO: if crash in libarary is fixed, delete it properly
  // delete m_pFlowScene;
  delete m_pFlowView;
}

//----------------------------------------------------------------------------------------
//
void CEditorSceneNodeWidget::Initialize()
{
  m_bInitialized = false;

  m_wpDbManager = CApplication::Instance()->System<CDatabaseManager>();

  m_pFlowScene = new FlowScene(m_spDataModelRegistry);
  m_pFlowView->setScene(m_pFlowScene);

  m_bInitialized = true;
}

//----------------------------------------------------------------------------------------
//
void CEditorSceneNodeWidget::LoadProject(tspProject spCurrentProject)
{
  WIDGET_INITIALIZED_GUARD

  m_spCurrentProject = spCurrentProject;

  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    QReadLocker projectLocker(&m_spCurrentProject->m_rwLock);
    if (m_spCurrentProject->m_sSceneModel.isNull() ||
        m_spCurrentProject->m_sSceneModel.isEmpty())
    {
      projectLocker.unlock();
      SaveNodeLayout();
    }
    else
    {
      const QString sModelName = m_spCurrentProject->m_sSceneModel;
      projectLocker.unlock();

      auto spResource = spDbManager->FindResource(m_spCurrentProject, sModelName);
      if (nullptr != spResource)
      {
        QReadLocker resourceLocker(&spResource->m_rwLock);
        QUrl path = spResource->m_sPath;
        resourceLocker.unlock();

        QString sPath = ResourceUrlToAbsolutePath(path, PhysicalProjectName(m_spCurrentProject));
        QFile modelFile(sPath);
        if (modelFile.open(QIODevice::ReadOnly))
        {
          QByteArray arr = modelFile.readAll();
          m_pFlowScene->loadFromMemory(arr);
        }
        else
        {
          qWarning() << tr("Could not open save scene model file.");
        }
      }
    }
  }
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

  m_pFlowScene->clearScene();
}

//----------------------------------------------------------------------------------------
//
void CEditorSceneNodeWidget::SaveNodeLayout()
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }

  QByteArray arr = m_pFlowScene->saveToMemory();

  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    QReadLocker projectLocker(&m_spCurrentProject->m_rwLock);
    if (m_spCurrentProject->m_sSceneModel.isNull() ||
        m_spCurrentProject->m_sSceneModel.isEmpty())
    {
      projectLocker.unlock();
      spDbManager->AddResource(m_spCurrentProject, QUrl::fromLocalFile("SceneModel.flow"),
                               EResourceType::eOther, "SceneModel.flow");
      projectLocker.relock();
      m_spCurrentProject->m_sSceneModel = "SceneModel.flow";
    }
    projectLocker.unlock();

    auto spResource = spDbManager->FindResource(m_spCurrentProject, "SceneModel.flow");
    if (nullptr != spResource)
    {
      QReadLocker resourceLocker(&spResource->m_rwLock);
      QUrl path = spResource->m_sPath;
      resourceLocker.unlock();

      QString sPath = ResourceUrlToAbsolutePath(path, PhysicalProjectName(m_spCurrentProject));
      QFile modelFile(sPath);
      if (modelFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
      {
        modelFile.write(arr);
      }
      else
      {
        qWarning() << tr("Could not open save scene model file.");
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorSceneNodeWidget::OnActionBarAboutToChange()
{
  if (nullptr != ActionBar())
  {
    disconnect(ActionBar()->m_spUi->pAddNodeButton, &QPushButton::clicked,
            this, &CEditorSceneNodeWidget::SlotAddSceneButtonClicked);
    disconnect(ActionBar()->m_spUi->pRemoveNodeButton, &QPushButton::clicked,
            this, &CEditorSceneNodeWidget::SlotRemoveNodeButtonClicked);
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
    connect(ActionBar()->m_spUi->pAddNodeButton, &QPushButton::clicked,
            this, &CEditorSceneNodeWidget::SlotAddSceneButtonClicked);
    connect(ActionBar()->m_spUi->pRemoveNodeButton, &QPushButton::clicked,
            this, &CEditorSceneNodeWidget::SlotRemoveNodeButtonClicked);
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorSceneNodeWidget::SlotAddSceneButtonClicked()
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }
}

//----------------------------------------------------------------------------------------
//
void CEditorSceneNodeWidget::SlotRemoveNodeButtonClicked()
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }
}
