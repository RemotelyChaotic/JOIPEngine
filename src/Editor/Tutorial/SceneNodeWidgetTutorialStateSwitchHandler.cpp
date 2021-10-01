#include "SceneNodeWidgetTutorialStateSwitchHandler.h"
#include "Application.h"
#include "EditorTutorialOverlay.h"
#include "Editor/EditorWidgets/EditorSceneNodeWidget.h"
#include "Editor/NodeEditor/EndNodeModel.h"
#include "Editor/NodeEditor/FlowScene.h"
#include "Editor/NodeEditor/FlowView.h"
#include "Editor/NodeEditor/SceneNodeModel.h"
#include "Editor/NodeEditor/StartNodeModel.h"
#include "Systems/DatabaseManager.h"

#include <nodes/Node>
#include <QMouseEvent>

CSceneNodeWidgetTutorialStateSwitchHandler::
CSceneNodeWidgetTutorialStateSwitchHandler(QPointer<CEditorSceneNodeWidget> pParentWidget,
                                           const std::shared_ptr<Ui::CEditorSceneNodeWidget>& spUi) :
  QObject(nullptr),
  ITutorialStateSwitchHandler(),
  m_pParentWidget(pParentWidget),
  m_spUi(spUi),
  m_currentState(ETutorialState::eFinished),
  m_bMenuOpenedForFirstTime(false),
  m_bFirstStartNodeCreated(false),
  m_bFirstEndNodeCreated(false),
  m_bFirstDefaultNodeCreated(false),
  m_bFirstDefaultNodeScriptAdded(false),
  m_bCompleteConnectionCreated(false)
{

}

CSceneNodeWidgetTutorialStateSwitchHandler::~CSceneNodeWidgetTutorialStateSwitchHandler()
{

}

//----------------------------------------------------------------------------------------
//
void CSceneNodeWidgetTutorialStateSwitchHandler::OnResetStates()
{
  m_currentState = ETutorialState::eFinished;
  m_bMenuOpenedForFirstTime = false;
  m_bFirstStartNodeCreated = false;
  m_bFirstEndNodeCreated = false;
  m_bFirstDefaultNodeCreated = false;
  m_bFirstDefaultNodeScriptAdded = false;
  m_bCompleteConnectionCreated = false;
  if (m_nodeCreatedConnection)
  {
    disconnect(m_nodeCreatedConnection);
  }
  if (m_resourceAddedConnection)
  {
    disconnect(m_resourceAddedConnection);
  }
  if (m_connectionAddedConnection)
  {
    disconnect(m_connectionAddedConnection);
  }
  if (nullptr != m_pParentWidget->FlowView())
  {
    m_pParentWidget->FlowView()->removeEventFilter(this);
  }
}

//----------------------------------------------------------------------------------------
//
void CSceneNodeWidgetTutorialStateSwitchHandler::OnStateSwitch(ETutorialState newState,
                                                               ETutorialState oldstate)
{
  Q_UNUSED(oldstate)
  m_currentState = newState;

  if (m_nodeCreatedConnection)
  {
    disconnect(m_nodeCreatedConnection);
  }
  if (m_resourceAddedConnection)
  {
    disconnect(m_resourceAddedConnection);
  }
  if (m_connectionAddedConnection)
  {
    disconnect(m_connectionAddedConnection);
  }

  switch (newState)
  {
    case ETutorialState::eNodePanel:
    {
      m_bMenuOpenedForFirstTime = false;

      auto registryMap =
          m_pParentWidget->FlowSceneModel()->registry().registeredModelCreators();
      for (auto it = registryMap.begin(); registryMap.end() != it; ++it)
      {
        m_pParentWidget->FlowView()->SetModelHiddenInContextMenu(it->first, true);
      }
      m_pParentWidget->FlowView()->SetModelHiddenInContextMenu(CStartNodeModel::staticCaption(), false);

      CFlowScene* pScene = m_pParentWidget->FlowSceneModel();
      m_connectionAddedConnection =
          connect(pScene, &QtNodes::FlowScene::connectionCreated,
                  this, &CSceneNodeWidgetTutorialStateSwitchHandler::SlotConnectionCreated);
      m_nodeCreatedConnection =
          connect(pScene, &QtNodes::FlowScene::nodeCreated,
                  this, &CSceneNodeWidgetTutorialStateSwitchHandler::SlotNodeCreated);

      auto spDbManager = CApplication::Instance()->System<CDatabaseManager>().lock();
      if (nullptr != spDbManager)
      {
        m_resourceAddedConnection =
            connect(spDbManager.get(), &CDatabaseManager::SignalResourceAdded,
                    this, &CSceneNodeWidgetTutorialStateSwitchHandler::SlotResourceAdded);
      }
      m_pParentWidget->FlowView()->installEventFilter(this);
    } break;

    case ETutorialState::eNodePanelAdvanced:
    {
      CFlowScene* pScene = m_pParentWidget->FlowSceneModel();
      m_savedFlow = pScene->saveToMemory();
      pScene->clearScene();
      QFile demoFlow(":/resources/help/tutorial/DemoSceneModel.flow");
      if (demoFlow.exists() && demoFlow.open(QIODevice::ReadOnly))
      {
        pScene->loadFromMemory(demoFlow.readAll());
        m_pParentWidget->FlowView()->FitAllNodesInView();
      }
      auto registryMap =
          m_pParentWidget->FlowSceneModel()->registry().registeredModelCreators();
      for (auto it = registryMap.begin(); registryMap.end() != it; ++it)
      {
        m_pParentWidget->FlowView()->SetModelHiddenInContextMenu(it->first, false);
      }
    } break;

    case ETutorialState::eNodePanelDone:
    {
      CFlowScene* pScene = m_pParentWidget->FlowSceneModel();
      pScene->clearScene();
      if (!m_savedFlow.isEmpty())
      {
        pScene->loadFromMemory(m_savedFlow);
        m_pParentWidget->FlowView()->FitAllNodesInView();
      }
    } break;
    default: break;
  }
}

//----------------------------------------------------------------------------------------
//
void CSceneNodeWidgetTutorialStateSwitchHandler::SlotConnectionCreated(
    QtNodes::Connection const& c)
{
  Q_UNUSED(c)
  bool bOk = QMetaObject::invokeMethod(this,
                                       "SlotConnectionCheck",
                                       Qt::QueuedConnection);
  assert(bOk);
  Q_UNUSED(bOk);
}

//----------------------------------------------------------------------------------------
//
void CSceneNodeWidgetTutorialStateSwitchHandler::SlotConnectionCheck()
{
  CFlowScene* pScene = m_pParentWidget->FlowSceneModel();
  if (nullptr != pScene && !m_bCompleteConnectionCreated)
  {
    bool bAllOk = true;
    auto fnNodeCheck = [&bAllOk](QtNodes::Node* pNode) {
      bool bOk = nullptr != pNode->nodeDataModel() &&
          pNode->nodeDataModel()->validationState() == NodeValidationState::Valid;
      bAllOk &= bOk;
    };
    pScene->iterateOverNodes(fnNodeCheck);

    if (bAllOk)
    {
      m_bCompleteConnectionCreated = true;
      TriggerNextInstruction();
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CSceneNodeWidgetTutorialStateSwitchHandler::SlotNodeCreated(QtNodes::Node& node)
{
  if (ETutorialState::eNodePanel == m_currentState._to_integral())
  {
    if (!m_bFirstStartNodeCreated)
    {
      if (auto pModel = dynamic_cast<CStartNodeModel*>(node.nodeDataModel()))
      {
        m_bFirstStartNodeCreated = true;
        m_pParentWidget->FlowView()->SetModelHiddenInContextMenu(CEndNodeModel::staticCaption(), false);
        TriggerNextInstruction();
      }
    }
    if (!m_bFirstEndNodeCreated)
    {
      if (auto pModel = dynamic_cast<CEndNodeModel*>(node.nodeDataModel()))
      {
        m_bFirstEndNodeCreated = true;
        m_pParentWidget->FlowView()->SetModelHiddenInContextMenu(CSceneNodeModel::staticCaption(), false);
        TriggerNextInstruction();
      }
    }
    if (!m_bFirstDefaultNodeCreated)
    {
      if (auto pModel = dynamic_cast<CSceneNodeModel*>(node.nodeDataModel()))
      {
        m_bFirstDefaultNodeCreated = true;
        TriggerNextInstruction();
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CSceneNodeWidgetTutorialStateSwitchHandler::SlotResourceAdded(qint32 iProjId,
                                                                   const QString& sName)
{
  Q_UNUSED(iProjId)
  Q_UNUSED(sName)
  if (!m_bFirstDefaultNodeScriptAdded)
  {
    m_bFirstDefaultNodeScriptAdded = true;
    TriggerNextInstruction();
  }
}

//----------------------------------------------------------------------------------------
//
bool CSceneNodeWidgetTutorialStateSwitchHandler::eventFilter(QObject* pObj, QEvent* pEvt)
{
  if (nullptr != pObj && nullptr != pEvt)
  {
    if (QEvent::MouseButtonPress == pEvt->type())
    {
      QMouseEvent* pMouseEvt = static_cast<QMouseEvent*>(pEvt);
      if (Qt::RightButton == pMouseEvt->button() && !m_bMenuOpenedForFirstTime)
      {
        m_bMenuOpenedForFirstTime = true;
        TriggerNextInstruction();
      }
    }
    else if (QEvent::ContextMenu == pEvt->type() && !m_bMenuOpenedForFirstTime)
    {
      m_bMenuOpenedForFirstTime = true;
      TriggerNextInstruction();
    }
  }
  return false;
}

//----------------------------------------------------------------------------------------
//
void CSceneNodeWidgetTutorialStateSwitchHandler::TriggerNextInstruction()
{
  bool bOk = QMetaObject::invokeMethod(CEditorTutorialOverlay::Instance(),
                                       "SlotTriggerNextInstruction",
                                       Qt::QueuedConnection);
  assert(bOk);
  Q_UNUSED(bOk);
}
