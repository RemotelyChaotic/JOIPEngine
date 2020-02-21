#include "EditorSceneNodeWidget.h"
#include "Application.h"
#include "EditorActionBar.h"
#include "Backend/Project.h"
#include "Backend/Scene.h"
#include "Backend/Resource.h"
#include "NodeEditor/EndNodeModel.h"
#include "NodeEditor/FlowView.h"
#include "NodeEditor/PathMergerModel.h"
#include "NodeEditor/PathSplitterModel.h"
#include "NodeEditor/SceneNodeModel.h"
#include "NodeEditor/SceneTranstitionData.h"
#include "NodeEditor/StartNodeModel.h"
#include "ui_EditorSceneNodeWidget.h"
#include "ui_EditorActionBar.h"

#include <nodes/ConnectionStyle>
#include <nodes/DataModelRegistry>
#include <nodes/FlowScene>
#include <nodes/FlowView>
#include <nodes/FlowViewStyle>
#include <nodes/Node>
#include <nodes/NodeData>
#include <nodes/NodeStyle>

#include <QContextMenuEvent>
#include <QDebug>
#include <QFile>
#include <QFileDialog>

#include <memory>

using QtNodes::ConnectionStyle;
using QtNodes::DataModelRegistry;
using QtNodes::FlowScene;
using QtNodes::FlowView;
using QtNodes::FlowViewStyle;
using QtNodes::Node;
using QtNodes::NodeStyle;

namespace
{
  std::shared_ptr<DataModelRegistry> RegisterDataModels()
  {
    auto ret = std::make_shared<DataModelRegistry>();
    ret->registerModel<CStartNodeModel>("Control");
    ret->registerModel<CSceneNodeModel>("Scene");
    ret->registerModel<CEndNodeModel>("Control");
    ret->registerModel<CPathMergerModel>("Path");
    ret->registerModel<CPathSplitterModel>("Path");
    return ret;
  }

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
  m_spUi(new Ui::CEditorSceneNodeWidget),
  m_spSettings(CApplication::Instance()->Settings()),
  m_spCurrentProject(nullptr),
  m_pFlowView(nullptr),
  m_pFlowScene(nullptr)
{
  m_spUi->setupUi(this);

  m_pFlowView = new CFlowView(this);
  m_pFlowView->setObjectName("FlowView");
  m_pFlowView->setWindowTitle("Node-based flow editor");
  m_pFlowView->show();
  m_pFlowView->setScene(nullptr);

  QLayout* pLayout = m_spUi->pContainerWidget->layout();
  pLayout->addWidget(m_pFlowView);
}

CEditorSceneNodeWidget::~CEditorSceneNodeWidget()
{
  UnloadProject();
}

//----------------------------------------------------------------------------------------
//
void CEditorSceneNodeWidget::Initialize()
{
  m_bInitialized = false;

  m_wpDbManager = CApplication::Instance()->System<CDatabaseManager>();

  m_pFlowScene = new FlowScene(RegisterDataModels(), this);
  m_pFlowView->setScene(m_pFlowScene);

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

  connect(m_pFlowScene, &FlowScene::nodeCreated,
          this, &CEditorSceneNodeWidget::SlotNodeCreated);
  connect(m_pFlowScene, &FlowScene::nodeDeleted,
          this, &CEditorSceneNodeWidget::SlotNodeDeleted);
  connect(CApplication::Instance(), &CApplication::StyleLoaded,
          this, &CEditorSceneNodeWidget::SlotStyleChanged);

  m_bInitialized = true;
}

//----------------------------------------------------------------------------------------
//
void CEditorSceneNodeWidget::LoadProject(tspProject spCurrentProject)
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr != m_spCurrentProject)
  {
    qWarning() << "Old Project was not unloaded before loading project.";
  }

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

  m_spCurrentProject = spCurrentProject;

  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    QReadLocker projectLocker(&m_spCurrentProject->m_rwLock);
    if (m_spCurrentProject->m_sSceneModel.isNull() ||
        m_spCurrentProject->m_sSceneModel.isEmpty())
    {
      projectLocker.unlock();
      SaveProject();
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
void CEditorSceneNodeWidget::SaveProject()
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
    disconnect(ActionBar()->m_spUi->AddNodeButton, &QPushButton::clicked,
            this, &CEditorSceneNodeWidget::SlotAddSceneButtonClicked);
    disconnect(ActionBar()->m_spUi->RemoveNodeButton, &QPushButton::clicked,
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
    connect(ActionBar()->m_spUi->AddNodeButton, &QPushButton::clicked,
            this, &CEditorSceneNodeWidget::SlotAddSceneButtonClicked);
    connect(ActionBar()->m_spUi->RemoveNodeButton, &QPushButton::clicked,
            this, &CEditorSceneNodeWidget::SlotRemoveNodeButtonClicked);
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorSceneNodeWidget::SlotAddSceneButtonClicked()
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }
  m_pFlowView->OpenContextMenuAt(QPoint(0, 0),
                                 QPoint(m_pFlowView->width() / 2, m_pFlowView->height() / 2));
}

//----------------------------------------------------------------------------------------
//
void CEditorSceneNodeWidget::SlotNodeCreated(Node &n)
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }

  auto spDbManager = m_wpDbManager.lock();
  CSceneNodeModel* pSceneModel = dynamic_cast<CSceneNodeModel*>(n.nodeDataModel());
  if (nullptr != pSceneModel && nullptr != spDbManager)
  {
    m_spCurrentProject->m_rwLock.lockForRead();
    qint32 iId = m_spCurrentProject->m_iId;
    m_spCurrentProject->m_rwLock.unlock();
    pSceneModel->SetProjectId(iId);

    qint32 iSceneId = pSceneModel->SceneId();
    auto spScene = spDbManager->FindScene(m_spCurrentProject, iSceneId);
    AddNewScriptFile(spScene);

    emit SignalProjectEdited();
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorSceneNodeWidget::SlotNodeDeleted(QtNodes::Node &n)
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }

  auto spDbManager = m_wpDbManager.lock();
  CSceneNodeModel* pSceneModel = dynamic_cast<CSceneNodeModel*>(n.nodeDataModel());
  if (nullptr != pSceneModel && nullptr != spDbManager)
  {
    qint32 iSceneId = pSceneModel->SceneId();
    spDbManager->RemoveScene(m_spCurrentProject, iSceneId);

    emit SignalProjectEdited();
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorSceneNodeWidget::SlotRemoveNodeButtonClicked()
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }

  auto vpNodes = m_pFlowScene->selectedNodes();
  for (Node* pNode : vpNodes)
  {
    if (nullptr != pNode)
    {
      m_pFlowScene->removeNode(*pNode);
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

//----------------------------------------------------------------------------------------
//
void CEditorSceneNodeWidget::AddNewScriptFile(tspScene spScene)
{
  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spScene && nullptr != spDbManager)
  {
    // if there is no script -> create
    QReadLocker locker(&spScene->m_rwLock);
    if (spScene->m_sScript.isNull() || spScene->m_sScript.isEmpty())
    {
      const QString sName = PhysicalProjectName(m_spCurrentProject);
      QString sCurrentFolder = CApplication::Instance()->Settings()->ContentFolder();
      QUrl sUrl = QFileDialog::getSaveFileUrl(this,
          QString(tr("Create Script File for %1")).arg(spScene->m_sName),
          QUrl::fromLocalFile(sCurrentFolder + "/" + sName),
          "Script Files (*.js)");

      if (sUrl.isValid())
      {
        QFileInfo info(sUrl.toLocalFile());
        QDir projectDir(m_spSettings->ContentFolder() + "/" + sName);
        if (!info.absoluteFilePath().contains(projectDir.absolutePath()))
        {
          qWarning() << "File is not in subfolder of Project.";
        }
        else
        {
          QString sRelativePath = projectDir.relativeFilePath(info.absoluteFilePath());
          QUrl sUrlToSave = QUrl::fromLocalFile(sRelativePath);
          QFile jsFile(info.absoluteFilePath());
          if (jsFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
          {
            jsFile.write(QString("// instert code to control scene").toUtf8());
            QString sResource = spDbManager->AddResource(m_spCurrentProject, sUrlToSave,
                                                         EResourceType::eOther);
            spScene->m_sScript = sResource;
          }
          else
          {
            qWarning() << "Could not write script file.";
          }
        }
      }
    }
  }
}
