#include "NodeDebugWidget.h"
#include "ui_NodeDebugWidget.h"

#include "Application.h"
#include "NodeEditorFlowView.h"
#include "NodeEditorFlowScene.h"
#include "NodeEditorRegistry.h"

#include "Systems/Player/SceneNodeResolver.h"

#include "Systems/Nodes/EndNodeModel.h"
#include "Systems/Nodes/PathSplitterModel.h"
#include "Systems/Nodes/SceneNodeModel.h"
#include "Systems/Nodes/SceneNodeModelWidget.h"
#include "Systems/Nodes/StartNodeModel.h"

#include "Systems/DatabaseManager.h"
#include "Systems/Project.h"
#include "Systems/Scene.h"

#include <nodes/internal/NodePainter.hpp>
#include <nodes/internal/StyleCollection.hpp>
#include <nodes/Node>

#include <QHBoxLayout>
#include <QLayout>
#include <QPainter>
#include <QPaintEvent>
#include <QPushButton>
#include <QScrollBar>
#include <QSpacerItem>
#include <QTimer>

struct SResolvementNode
{
  SResolvementNode(QtNodes::Node* pNode, const IResolverDebugger::NodeData& data) :
      m_pNode(pNode), m_data(data)
  {}

  QtNodes::Node* m_pNode = nullptr;

  IResolverDebugger::NodeData m_data;

  std::vector<std::shared_ptr<SResolvementNode>> m_vspChildren;
};

//----------------------------------------------------------------------------------------
//
class CNodeDebugger : public IResolverDebugger
{
public:
  CNodeDebugger(CNodeDebugWidget* pWidget) :
      IResolverDebugger(),
      m_pWidget(pWidget)
  {}
  ~CNodeDebugger() override = default;

  //--------------------------------------------------------------------------------------
  //
  QtNodes::Node* CurrentNode() const
  {
    return nullptr != m_spCurrentResolveRoot ? m_spCurrentResolveRoot->m_pNode : nullptr;
  }

  //--------------------------------------------------------------------------------------
  //
  std::vector<NodeData*> ChildBlocks(const QtNodes::Node* pNode) const override
  {
    std::vector<NodeData*> vpRet;
    std::shared_ptr<SResolvementNode> pResNode = const_cast<CNodeDebugger*>(this)->FindResolvementNode(pNode->id());
    if (nullptr != pResNode)
    {
      for (auto& spChild : pResNode->m_vspChildren)
      {
        vpRet.push_back(&spChild->m_data);
      }
    }
    return vpRet;
  }

  //--------------------------------------------------------------------------------------
  //
  NodeData* DataBlock(const QtNodes::Node* pNode) const override
  {
    std::shared_ptr<SResolvementNode> pResNode = const_cast<CNodeDebugger*>(this)->FindResolvementNode(pNode->id());
    if (nullptr != pResNode)
    {
      return &pResNode->m_data;
    }
    return nullptr;
  }

  //--------------------------------------------------------------------------------------
  //
  void Error(const QString& sError, QtMsgType type) override
  {
    if (nullptr != m_pWidget)
    {
      m_pWidget->SlotSceneError(sError, type);
    }
  }

  //--------------------------------------------------------------------------------------
  //
  void PoppUntilNextSelectable()
  {
    if (nullptr == m_spCurrentResolveRoot)
    {
      return;
    }

    std::map<qint32, std::vector<std::shared_ptr<SResolvementNode>>> depthMap;

    std::function<void(SResolvementNode*,qint32)> fnSearch =
        [&](SResolvementNode* pStart, qint32 iDepth) {
      for (const auto& spChild : pStart->m_vspChildren)
      {
        if (spChild->m_data.bSelection && spChild->m_data.bEnabled)
        {
          depthMap[iDepth].push_back(spChild);
        }
        fnSearch(spChild.get(), iDepth+1);
      }
    };
    fnSearch(m_spCurrentResolveRoot.get(), 1);

    if (!depthMap.empty())
    {
      m_spCurrentResolveRoot = depthMap.begin()->second.front();
    }
    else
    {
      m_spCurrentResolveRoot = nullptr;
    }
  }

  //--------------------------------------------------------------------------------------
  //
  void ResolveTo(const QtNodes::Node* const pNode) override
  {
    std::shared_ptr<SResolvementNode> pResNode = FindResolvementNode(pNode->id());
    if (nullptr != pResNode)
    {
      m_spCurrentResolveRoot = pResNode;
    }
  }

  //--------------------------------------------------------------------------------------
  //
  void PushNode(const QtNodes::Node* const pParent, QtNodes::Node* const pNext,
                NodeData data) override
  {
    std::shared_ptr<SResolvementNode> pResNode = FindResolvementNode(pParent->id());
    if (nullptr != pResNode)
    {
      pResNode->m_vspChildren.push_back(
          std::make_shared<SResolvementNode>(
              pNext, data));
    }
  }

  //--------------------------------------------------------------------------------------
  //
  void SetCurrentNode(QtNodes::Node* const pNode) override
  {
    m_spCurrentResolveRoot =
        std::make_shared<SResolvementNode>(const_cast<QtNodes::Node*>(pNode),
                                           NodeData{true, false, "Current", -1});
  }

private:
  //--------------------------------------------------------------------------------------
  //
  std::shared_ptr<SResolvementNode> FindResolvementNode(
      QUuid parentId, std::shared_ptr<SResolvementNode> pStart = nullptr)
  {
    if (nullptr == pStart)
    {
      pStart = m_spCurrentResolveRoot;
    }

    if (nullptr == pStart)
    {
      return nullptr;
    }

    if (pStart->m_pNode->id() == parentId)
    {
      return pStart;
    }

    for (auto& spChild : pStart->m_vspChildren)
    {
      auto pRes = FindResolvementNode(parentId, spChild);
      if (nullptr != pRes)
      {
        return pRes;
      }
    }
    return nullptr;
  }

  QPointer<CNodeDebugWidget>            m_pWidget;
  std::shared_ptr<SResolvementNode>     m_spCurrentResolveRoot;
};

//----------------------------------------------------------------------------------------
//
CNodeContainingItem::CNodeContainingItem(QtNodes::Node* pNode) :
  m_pNode(pNode)
{
}
CNodeContainingItem::~CNodeContainingItem() = default;

//----------------------------------------------------------------------------------------
//
QtNodes::Node* CNodeContainingItem::Node() const
{
  return m_pNode;
}

//----------------------------------------------------------------------------------------
//
CNodeMock::CNodeMock(std::unique_ptr<QtNodes::NodeDataModel>&& dataModel, QtNodes::Node* pNode,
                     QWidget* pParent, QWidget* pView) :
  QFrame(pParent),
  CNodeContainingItem(pNode),
  m_spDataModel(std::move(dataModel)),
  m_pActualParent(pView),
  m_geometry(m_spDataModel),
  m_state(m_spDataModel)
{
  setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

  QWidget* pWidget = m_spDataModel->embeddedWidget();
  if (nullptr != pWidget)
  {
    QHBoxLayout* pLayout = new QHBoxLayout(this);
    pLayout->addWidget(pWidget);
    pWidget->setDisabled(true);
    pWidget->setAttribute(Qt::WA_TranslucentBackground);
    pWidget->setAttribute(Qt::WA_OpaquePaintEvent, false);
    pWidget->setAttribute(Qt::WA_NoSystemBackground, false);
  }

  NodeDataModel const * model = m_spDataModel.get();
  QtNodes::NodeStyle const& nodeStyle = model->nodeStyle();
  m_iConnectionPointDiameter = nodeStyle.ConnectionPointDiameter;

  QPointer<CNodeMock> pGuard(this);
  QTimer::singleShot(0, this, [pGuard, this](){
    if (nullptr != pGuard)
    {
      Resize(QSize{static_cast<qint32>(std::ceil(sizeHint().width() + 2.0*m_iConnectionPointDiameter)),
                   sizeHint().height()});
    }
  });

  pParent->installEventFilter(this);
  pView->installEventFilter(this);
}
CNodeMock::~CNodeMock() = default;

//----------------------------------------------------------------------------------------
//
void CNodeMock::Resize(QSize s)
{
  setFixedWidth(s.width());
  resize(s);

  // Changes geometry size..
  m_geometry.recalculateSize(font());
  m_geometry.setWidth(static_cast<qint32>(s.width()-2.0*m_iConnectionPointDiameter));
  m_geometry.setHeight(static_cast<qint32>(s.height()-2.0*m_iConnectionPointDiameter));
}

//----------------------------------------------------------------------------------------
//
namespace
{
  // rebuilt what NodePainter::drawNodeRect does but without needing NodeGraphicsObject
  void
  drawNodeRect(QPainter* painter,
               QtNodes::NodeGeometry const& geom,
               QtNodes::NodeDataModel const* model)

  {
    QtNodes::NodeStyle const& nodeStyle = model->nodeStyle();

    auto color = nodeStyle.NormalBoundaryColor;

    if (geom.hovered())
    {
      QPen p(color, nodeStyle.HoveredPenWidth);
      painter->setPen(p);
    }
    else
    {
      QPen p(color, nodeStyle.PenWidth);
      painter->setPen(p);
    }

    QLinearGradient gradient(QPointF(0.0, 0.0),
                             QPointF(2.0, geom.height()));

    gradient.setColorAt(0.0, nodeStyle.GradientColor0);
    gradient.setColorAt(0.03, nodeStyle.GradientColor1);
    gradient.setColorAt(0.97, nodeStyle.GradientColor2);
    gradient.setColorAt(1.0, nodeStyle.GradientColor3);

    painter->setBrush(gradient);

    float diam = nodeStyle.ConnectionPointDiameter;

    QRectF boundary( -diam, -diam, 2.0 * diam + geom.width(), 2.0 * diam + geom.height());

    double const radius = 3.0;

    painter->drawRoundedRect(boundary, radius, radius);
  }

  void
  drawConnectionPoints(QPainter* painter,
                       QtNodes::NodeGeometry const& geom,
                       QtNodes::NodeState const& state,
                       QtNodes::NodeDataModel const * model)
  {
    QtNodes::NodeStyle const& nodeStyle      = model->nodeStyle();
    auto const     &connectionStyle = QtNodes::StyleCollection::connectionStyle();

    float diameter = nodeStyle.ConnectionPointDiameter;
    auto  reducedDiameter = diameter * 0.6;

    for (PortType portType: {PortType::Out, PortType::In})
    {
      size_t n = state.getEntries(portType).size();

      for (unsigned int i = 0; i < n; ++i)
      {
        QPointF p = geom.portScenePosition(i, portType);

        auto const & dataType = model->dataType(portType, i);

        bool canConnect = (state.getEntries(portType)[i].empty() ||
                           model->portConnectionPolicy(portType, i) == NodeDataModel::ConnectionPolicy::Many);

        double r = 1.0;
        if (state.isReacting() &&
            canConnect &&
            portType == state.reactingPortType())
        {
          auto   diff = geom.draggingPos() - p;
          double dist = std::sqrt(QPointF::dotProduct(diff, diff));
          bool   typeConvertable = true;

          /*
          {
            if (portType == PortType::In)
            {
              typeConvertable = scene.registry().getTypeConverter(state.reactingDataType(), dataType) != nullptr;
            }
            else
            {
              typeConvertable = scene.registry().getTypeConverter(dataType, state.reactingDataType()) != nullptr;
            }
          }
          */

          if (state.reactingDataType().id == dataType.id || typeConvertable)
          {
            double const thres = 40.0;
            r = (dist < thres) ?
                    (2.0 - dist / thres ) :
                    1.0;
          }
          else
          {
            double const thres = 80.0;
            r = (dist < thres) ?
                    (dist / thres) :
                    1.0;
          }
        }

        if (connectionStyle.useDataDefinedColors())
        {
          painter->setBrush(connectionStyle.normalColor(dataType.id));
        }
        else
        {
          painter->setBrush(nodeStyle.ConnectionPointColor);
        }

        painter->drawEllipse(p,
                             reducedDiameter * r,
                             reducedDiameter * r);
      }
    }

  }
}

//----------------------------------------------------------------------------------------
//
bool CNodeMock::eventFilter(QObject* pObj, QEvent* pEvt)
{
  if (nullptr != pEvt && pEvt->type() == QEvent::Resize)
  {
    Resize(QSize{static_cast<qint32>(std::ceil(sizeHint().width() + 2.0*m_iConnectionPointDiameter)),
                 sizeHint().height()});
  }
  return false;
}

//----------------------------------------------------------------------------------------
//
void CNodeMock::mouseDoubleClickEvent(QMouseEvent* pEvt)
{
  if (nullptr != pEvt)
  {
    if (auto pDebugger = dynamic_cast<CNodeDebugWidget*>(m_pActualParent.data()))
    {
      pDebugger->FocusNode(m_pNode);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CNodeMock::paintEvent(QPaintEvent* pEvt)
{
  // rebuild what NodePainter::paint does but without needing scene or View
  QPainter painter(this);

  QtNodes::NodeGeometry const& geom = m_geometry;

  QtNodes::NodeState const& state = m_state;

  NodeDataModel const * model = m_spDataModel.get();

  QtNodes::NodeStyle const& nodeStyle = model->nodeStyle();

  float diam = nodeStyle.ConnectionPointDiameter;

  painter.translate(diam, diam);

  drawNodeRect(&painter, geom, model);

  drawConnectionPoints(&painter, geom, state, model);

  QtNodes::NodePainter::drawFilledConnectionPoints(&painter, geom, state, model);

  QtNodes::NodePainter::drawModelName(&painter, geom, state, model);

  // Entry labels clutter the widget too much
  //QtNodes::NodePainter::drawEntryLabels(&painter, geom, state, model);

  // Resize not possible
  //QtNodes::NodePainter::drawResizeRect(&painter, geom, model);

  // Validation rect not needed
  //drawValidationRect(&painter, geom, model);
}

//----------------------------------------------------------------------------------------
//
QSize CNodeMock::sizeHint() const
{
  QSize s(QFrame::sizeHint().width(),
          parentWidget()->height()-parentWidget()->layout()->margin()*2);
  return s;
}

//----------------------------------------------------------------------------------------
//
namespace
{
  enum NodeMockType
  {
    eStart,
    eEnd,
    eScene,
    eSplitter
  };

  std::unique_ptr<QtNodes::NodeDataModel> GetNodeModel(NodeMockType type)
  {
    switch(type)
    {
      case NodeMockType::eStart: return std::make_unique<CStartNodeModel>();
      case NodeMockType::eEnd: return std::make_unique<CEndNodeModel>();
      case NodeMockType::eScene: return std::make_unique<CSceneNodeModelWithWidget>();
      case NodeMockType::eSplitter: return std::make_unique<CPathSplitterModel>();
    }
    return nullptr;
  }
}

//----------------------------------------------------------------------------------------
//
CNodeDebugNodeStartEnd::CNodeDebugNodeStartEnd(bool bStart, QtNodes::Node* pNode,
                                               QWidget* pParent, QWidget* pView) :
  CNodeMock(GetNodeModel(bStart ? NodeMockType::eStart : NodeMockType::eEnd), pNode,
            pParent, pView),
  m_geometryFixed(m_spDataModel)
{
  m_geometryFixed.recalculateSize(font());
}
CNodeDebugNodeStartEnd::~CNodeDebugNodeStartEnd() = default;

//----------------------------------------------------------------------------------------
//
void CNodeDebugNodeStartEnd::paintEvent(QPaintEvent* pEvt)
{
  CNodeMock::paintEvent(pEvt);
}

//----------------------------------------------------------------------------------------
//
QSize CNodeDebugNodeStartEnd::sizeHint() const
{
  //m_geometryFixed.recalculateSize(font());
  QSize s(m_geometryFixed.width() + 2*m_iConnectionPointDiameter,
          parentWidget()->height()-parentWidget()->layout()->margin()*2);
  return s;
}

//----------------------------------------------------------------------------------------
//
CNodeDebugNode::CNodeDebugNode(tspProject spProject,
                               const QString& sScene,
                               QtNodes::Node* pNode,
                               QWidget* pParent, QWidget* pView) :
  CNodeMock(GetNodeModel(NodeMockType::eScene), pNode, pParent, pView)
{
  auto pWidget = dynamic_cast<CSceneNodeModelWidget*>(m_spDataModel->embeddedWidget());
  auto pModel = dynamic_cast<CSceneNodeModel*>(pNode->nodeDataModel());
  if (nullptr != pModel && nullptr != pWidget)
  {
    pWidget->SetProject(spProject);
    pWidget->SetName(pModel->SceneName());
    if (auto spDbManager = CApplication::Instance()->System<CDatabaseManager>().lock())
    {
      qint32 iId = pModel->SceneId();
      auto spScene = spDbManager->FindScene(spProject, iId);
      if (nullptr != spScene)
      {
        QReadLocker l(&spScene->m_rwLock);
        pWidget->SetTileResource(spScene->m_sTitleCard);
        pWidget->SetCanStartHere(spScene->m_bCanStartHere);
        pWidget->SetScript(spScene->m_sScript);
        pWidget->SetLayout(spScene->m_sSceneLayout);
      }
    }
  }
}
CNodeDebugNode::~CNodeDebugNode() = default;

//----------------------------------------------------------------------------------------
//
void CNodeDebugNode::paintEvent(QPaintEvent* pEvt)
{
  CNodeMock::paintEvent(pEvt);
}

//----------------------------------------------------------------------------------------
//
CNodeDebugError::CNodeDebugError(QString sError, QtMsgType type, QWidget* pParent,
                                 QWidget* pView) :
  QFrame(pParent)
{
  setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

  QVBoxLayout* pLayout = new QVBoxLayout(this);
  m_pLabel = new QLabel(this);
  m_pLabel->setText(sError);
  m_pLabel->setAlignment(Qt::AlignCenter);
  pLayout->addWidget(m_pLabel);

  auto spModel = GetNodeModel(NodeMockType::eStart);
  QtNodes::NodeStyle const& nodeStyle = spModel->nodeStyle();

  m_errorColor = nodeStyle.ErrorColor;
  m_normalBoundaryColor = nodeStyle.NormalBoundaryColor;
  m_iPenWidth = nodeStyle.PenWidth;
  m_iConnectionPointDiameter = nodeStyle.ConnectionPointDiameter;

  QPointer<CNodeDebugError> pGuard(this);
  QTimer::singleShot(0, this, [pGuard, this](){
    if (nullptr != pGuard)
    {
      QSize s{static_cast<qint32>(std::ceil(sizeHint().width() + 2.0*m_iConnectionPointDiameter)),
              sizeHint().height()};
      setFixedWidth(s.width());
      resize(s);
    }
  });

  pParent->installEventFilter(this);
  pView->installEventFilter(this);
}
CNodeDebugError::~CNodeDebugError() = default;

//----------------------------------------------------------------------------------------
//
void CNodeDebugError::PushError(const QString& sError)
{
  QStringList sl = QStringList() << m_pLabel->text() << sError;
  m_pLabel->setText(sl.join("\n"));
}

//----------------------------------------------------------------------------------------
//
bool CNodeDebugError::eventFilter(QObject* pObj, QEvent* pEvt)
{
  if (nullptr != pEvt && pEvt->type() == QEvent::Resize)
  {
    QSize s{static_cast<qint32>(std::ceil(sizeHint().width() + 2.0*m_iConnectionPointDiameter)),
            sizeHint().height()};
    setFixedWidth(s.width());
    resize(s);
  }
  return false;
}

//----------------------------------------------------------------------------------------
//
void CNodeDebugError::paintEvent(QPaintEvent* pEvt)
{
  // rebuild what NodePainter::paint does but without needing scene or View
  QPainter painter(this);

  painter.translate(m_iConnectionPointDiameter, m_iConnectionPointDiameter);

  QPen p(m_normalBoundaryColor, m_iPenWidth);
  painter.setPen(p);
  painter.setBrush(m_errorColor);

  QRectF boundary(0.0, 0.0,
                  width() - 2.0 * m_iConnectionPointDiameter,
                  height() - 2.0 * m_iConnectionPointDiameter);

  double const radius = 3.0;

  painter.drawRoundedRect(boundary, radius, radius);

  QFrame::paintEvent(pEvt);
}

//----------------------------------------------------------------------------------------
//
QSize CNodeDebugError::sizeHint() const
{
  QSize s(QFrame::sizeHint().width(),
          parentWidget()->height()-parentWidget()->layout()->margin()*2);
  return s;
}

//----------------------------------------------------------------------------------------
//
CNodeDebugSelection::CNodeDebugSelection(const QStringList& vsScenes, QtNodes::Node* pNode,
                                         QWidget* pParent, CNodeDebugWidget* pView) :
  CNodeMock(GetNodeModel(NodeMockType::eSplitter), pNode, pParent, pView),
  m_vsScenes(vsScenes)
{
  QVBoxLayout* pRootLayout = new QVBoxLayout(this);
  QSpacerItem* pSpacer1 = new QSpacerItem(0,0, QSizePolicy::Preferred, QSizePolicy::Expanding);
  QLabel* pLabel = new QLabel(tr("Select scene"), this);
  QWidget* pContainer = new QWidget(this);
  QSpacerItem* pSpacer2 = new QSpacerItem(0,0, QSizePolicy::Preferred, QSizePolicy::Expanding);
  pRootLayout->addItem(pSpacer1);
  pRootLayout->addWidget(pLabel);
  pRootLayout->addWidget(pContainer);
  pRootLayout->addItem(pSpacer2);

  QHBoxLayout* pLayout = new QHBoxLayout(pContainer);
  m_pComboSelection = new QComboBox(this);
  for (const QString& sScene : qAsConst(m_vsScenes))
  {
    m_pComboSelection->addItem(sScene, sScene);
  }
  QPushButton* pButtonContinue = new QPushButton(this);
  pButtonContinue->setObjectName("NextSceneButton");
  pButtonContinue->setProperty("styleSmall", true);
  pButtonContinue->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
  pButtonContinue->setFixedSize(24,24);
  pLayout->addWidget(m_pComboSelection);
  pLayout->addWidget(pButtonContinue);

  connect(pButtonContinue, &QPushButton::clicked, pView, [pView, this]() {
    pView->NextScene(CurrentIndex(), true);
  });
}
CNodeDebugSelection::~CNodeDebugSelection() = default;

//----------------------------------------------------------------------------------------
//
QString CNodeDebugSelection::CurrentScene() const
{
  return m_pComboSelection->currentData().toString();
}

//----------------------------------------------------------------------------------------
//
qint32 CNodeDebugSelection::CurrentIndex() const
{
  return m_pComboSelection->currentIndex();
}

//----------------------------------------------------------------------------------------
//
void CNodeDebugSelection::paintEvent(QPaintEvent* pEvt)
{
  CNodeMock::paintEvent(pEvt);
  QFrame::paintEvent(pEvt);
}

//----------------------------------------------------------------------------------------
//
CNodeDebugBackground::CNodeDebugBackground(QWidget* pParent) : QWidget(pParent) {}
CNodeDebugBackground::~CNodeDebugBackground() = default;

//----------------------------------------------------------------------------------------
//
void CNodeDebugBackground::SetBackgroundColor(const QColor& col)
{
  m_backgroundColor = col;
  repaint();
}

//----------------------------------------------------------------------------------------
//
void CNodeDebugBackground::paintEvent(QPaintEvent* pEvt)
{
  QPainter p(this);
  p.fillRect(pEvt->rect(), m_backgroundColor);
}

//----------------------------------------------------------------------------------------
//
CNodeDebugWidget::CNodeDebugWidget(QWidget* pParent) :
  QWidget(pParent),
  m_spUi(std::make_unique<Ui::CNodeDebugWidget>()),
  m_spNodeResolver(std::make_unique<CSceneNodeResolver>(
        CNodeEditorRegistryBase::RegisterDataModelsWithoutUi())),
  m_spNodeDebugger(std::make_shared<CNodeDebugger>(this))
{
  m_spUi->setupUi(this);

  //connect(m_spNodeResolver.get(), &CSceneNodeResolver::SignalChangeSceneRequest,
  //        this, &CNodeDebugWidget::SlotSceneError);
  //connect(m_spNodeResolver.get(), &CSceneNodeResolver::SignalError,
  //        this, &CNodeDebugWidget::SlotSceneError);

  QLayout* pLayout = m_spUi->pScrollAreaWidgetContents->layout();
  assert(nullptr != pLayout);
  if (nullptr != pLayout)
  {
    pLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Preferred));
  }

  m_spNodeResolver->AttatchDebugger(m_spNodeDebugger);
}

CNodeDebugWidget::~CNodeDebugWidget()
{
}

//----------------------------------------------------------------------------------------
//
void CNodeDebugWidget::Initialize(QPointer<CNodeEditorFlowView> pFlowView,
                                  QPointer<CNodeEditorFlowScene> pScene)
{
  m_pFlowView = pFlowView;
  m_pScene = pScene;
}

//----------------------------------------------------------------------------------------
//
void CNodeDebugWidget::NextScene(qint32 iIndex, bool bSkipLastElementcheck)
{
  if (!IsInErrorState())
  {
    if (!bSkipLastElementcheck)
    {
      if (auto pSelect = dynamic_cast<CNodeDebugSelection*>(LastWidget());
          nullptr != pSelect && -1 == iIndex)
      {
        iIndex = pSelect->CurrentIndex();
      }
    }

    std::optional<QString> unresolvedData;
    QStringList vsScenes = m_spNodeResolver->PossibleScenes(&unresolvedData);
    if (vsScenes.size() > 0)
    {
      if (!unresolvedData.has_value())
      {
        if (vsScenes.size() == 1)
        {
          bool bEnd = false;
          tspScene spScene = m_spNodeResolver->NextScene(vsScenes[0], &bEnd);
          if (nullptr != spScene)
          {
            AddWidget(new CNodeDebugNode(m_spCurrentProject, vsScenes[0],
                                         NodeFromScene(m_spNodeDebugger->CurrentNode()),
                                         m_spUi->pScrollAreaWidgetContents,
                                         this));
            m_spNodeResolver->ResolveScenes();
          }
          else if (bEnd)
          {
            AddWidget(new CNodeDebugNodeStartEnd(false,
                                                 NodeFromScene(m_spNodeDebugger->CurrentNode()),
                                                 m_spUi->pScrollAreaWidgetContents,
                                                 this));
          }
        }
        else if (-1 != iIndex)
        {
          bool bEnd = false;
          tspScene spScene = m_spNodeResolver->NextScene(vsScenes[iIndex], &bEnd);
          if (nullptr != spScene)
          {
            AddWidget(new CNodeDebugNode(m_spCurrentProject, vsScenes[iIndex],
                                         NodeFromScene(m_spNodeDebugger->CurrentNode()),
                                         m_spUi->pScrollAreaWidgetContents,
                                         this));
            m_spNodeResolver->ResolveScenes();
          }
          else if (bEnd)
          {
            AddWidget(new CNodeDebugNodeStartEnd(false,
                                                 NodeFromScene(m_spNodeDebugger->CurrentNode()),
                                                 m_spUi->pScrollAreaWidgetContents,
                                                 this));
          }
        }
        else
        {
          m_spNodeDebugger->PoppUntilNextSelectable();
          AddWidget(new CNodeDebugSelection(vsScenes, NodeFromScene(m_spNodeDebugger->CurrentNode()),
                                            m_spUi->pScrollAreaWidgetContents, this));
        }
      }
      else
      {
        QString sUnResolveData = unresolvedData.value();
        if (-1 != iIndex)
        {
          m_spNodeResolver->ResolvePossibleScenes(vsScenes, iIndex);
          // do we still have unresolved scenes?
          vsScenes = m_spNodeResolver->PossibleScenes(&unresolvedData);
          NextScene(-1, true);
        }
        else
        {
          m_spNodeDebugger->PoppUntilNextSelectable();
          AddWidget(new CNodeDebugSelection(vsScenes, NodeFromScene(m_spNodeDebugger->CurrentNode()),
                                            m_spUi->pScrollAreaWidgetContents, this));
        }
      }
    }
    else
    {
      AddWidget(new CNodeDebugNodeStartEnd(false, nullptr,
                                           m_spUi->pScrollAreaWidgetContents, this));
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CNodeDebugWidget::StartDebug(const tspProject& spProject,
                                  const std::variant<QString, QUuid>& start)
{
  m_spCurrentProject = spProject;

  Clear();
  show();

  bool bIsEmpty = false;
  if (std::holds_alternative<QString>(start))
  {
    m_spNodeResolver->LoadProject(m_spCurrentProject, std::get<QString>(start));
    bIsEmpty = std::get<QString>(start).isEmpty();
  }
  else if (std::holds_alternative<QUuid>(start))
  {
    m_spNodeResolver->LoadProject(m_spCurrentProject, std::get<QUuid>(start));
    bIsEmpty = std::get<QUuid>(start).isNull();
  }

  if (!IsInErrorState())
  {
    tspScene spStartScene = m_spNodeResolver->CurrentScene();

    if (bIsEmpty || nullptr == spStartScene)
    {
      AddWidget(new CNodeDebugNodeStartEnd(true,
                                           NodeFromScene(m_spNodeDebugger->CurrentNode()),
                                           m_spUi->pScrollAreaWidgetContents,
                                           this));
    }
    else
    {
      QString sStartScene;
      {
        QReadLocker l(&spStartScene->m_rwLock);
        sStartScene = spStartScene->m_sName;
      }
      AddWidget(new CNodeDebugNode(m_spCurrentProject,
                                   sStartScene,
                                   NodeFromScene(m_spNodeDebugger->CurrentNode()),
                                   m_spUi->pScrollAreaWidgetContents,
                                   this));
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CNodeDebugWidget::StopDebug()
{
  m_spNodeResolver->UnloadProject();

  hide();
  Clear();

  m_spCurrentProject = nullptr;
}

//----------------------------------------------------------------------------------------
//
QColor CNodeDebugWidget::BackgroundColor() const
{
  return m_spUi->pScrollAreaWidgetContents->BackgroundColor();
}

//----------------------------------------------------------------------------------------
//
void CNodeDebugWidget::SetBackgroundColor(const QColor& col)
{
  m_spUi->pScrollAreaWidgetContents->SetBackgroundColor(col);
}

//----------------------------------------------------------------------------------------
//
void CNodeDebugWidget::FocusNode(QtNodes::Node* pNode)
{
  if (nullptr != pNode && nullptr != m_pScene && nullptr != m_pFlowView)
  {
    QGraphicsObject& go = pNode->nodeGraphicsObject();
    QPointF pointCenter = go.sceneBoundingRect().center();

    QRectF r = m_pScene->sceneRect();
    r.translate(-r.topLeft());
    r.translate(pointCenter - QPointF(r.width()/2, r.height()/2));
    m_pFlowView->setSceneRect(r);
    m_pFlowView->update();
  }
}

//----------------------------------------------------------------------------------------
//
void CNodeDebugWidget::SlotSceneError(QString sError, QtMsgType type)
{
  auto pLastError = dynamic_cast<CNodeDebugError*>(LastWidget());
  if (nullptr == pLastError)
  {
    AddWidget(new CNodeDebugError(sError, type, m_spUi->pScrollAreaWidgetContents, this));
  }
  else
  {
    pLastError->PushError(sError);
  }
}

//----------------------------------------------------------------------------------------
//
void CNodeDebugWidget::SlotUpdateScene()
{
  if (auto pW = dynamic_cast<CNodeContainingItem*>(LastWidget()))
  {
    FocusNode(pW->Node());
  }
}

//----------------------------------------------------------------------------------------
//
void CNodeDebugWidget::SlotUpdateScrollAndScene()
{
  QTimer::singleShot(0, this, [this](){
    SlotUpdateScene();
    ScrollToEnd();
  });
}

//----------------------------------------------------------------------------------------
//
void CNodeDebugWidget::AddWidget(QWidget* pWidget)
{
  QHBoxLayout* pLayout = dynamic_cast<QHBoxLayout*>(m_spUi->pScrollAreaWidgetContents->layout());
  assert(nullptr != pLayout);
  if (nullptr != pLayout)
  {
    UpdateNodeContainingItemNode(dynamic_cast<CNodeContainingItem*>(LastWidget()),
                                 CNodeModelBase::eDebugPassive);

    qint32 iCount = pLayout->count();
    pLayout->insertWidget(iCount-1, pWidget);

    UpdateNodeContainingItemNode(dynamic_cast<CNodeContainingItem*>(LastWidget()),
                                 CNodeModelBase::eDebugActive);

    bool bOk = QMetaObject::invokeMethod(this, "SlotUpdateScrollAndScene", Qt::QueuedConnection);
    assert(bOk); Q_UNUSED(bOk)
  }
}

//----------------------------------------------------------------------------------------
//
void CNodeDebugWidget::Clear()
{
  QLayout* pLayout = m_spUi->pScrollAreaWidgetContents->layout();
  assert(nullptr != pLayout);
  if (nullptr != pLayout)
  {
    while (pLayout->count() > 0)
    {
      QLayoutItem* pItem = pLayout->takeAt(0);
      if (nullptr != pItem)
      {
        if (nullptr != pItem->widget())
        {
          UpdateNodeContainingItemNode(dynamic_cast<CNodeContainingItem*>(pItem->widget()),
                                       CNodeModelBase::eNotDebugged);
          delete pItem->widget();
        }
        delete pItem;
      }
    }
    pLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Preferred));
  }
}

//----------------------------------------------------------------------------------------
//
bool CNodeDebugWidget::IsInErrorState() const
{
  return dynamic_cast<CNodeDebugError*>(LastWidget()) != nullptr;
}

//----------------------------------------------------------------------------------------
//
QWidget* CNodeDebugWidget::LastWidget() const
{
  QLayout* pLayout = m_spUi->pScrollAreaWidgetContents->layout();
  assert(nullptr != pLayout);
  if (nullptr != pLayout)
  {
    auto pItem = pLayout->itemAt(pLayout->count()-2);
    if (nullptr != pItem)
    {
      return pItem->widget();
    }
  }
  return nullptr;
}

//----------------------------------------------------------------------------------------
//
QtNodes::Node* CNodeDebugWidget::NodeFromScene(QtNodes::Node* pLocalNode)
{
  if (nullptr != pLocalNode && nullptr != m_pScene)
  {
    QUuid id = pLocalNode->id();
    const auto& nodes = m_pScene->nodes();
    auto it = nodes.find(id);
    if (nodes.end() != it)
    {
      return it->second.get();
    }
  }
  return nullptr;
}

//----------------------------------------------------------------------------------------
//
void CNodeDebugWidget::ScrollToEnd()
{
  QScrollBar* pScroll = m_spUi->pScrollArea->horizontalScrollBar();
  pScroll->setValue(pScroll->maximum());
}

//----------------------------------------------------------------------------------------
//
void CNodeDebugWidget::UpdateNodeContainingItemNode(CNodeContainingItem* pItem,
                                                    CNodeModelBase::EDebugState state)
{
  if (nullptr != pItem)
  {
    QtNodes::Node* pNode = pItem->Node();
    if (nullptr != pNode)
    {
      if (auto pDebuggableModel = dynamic_cast<CNodeModelBase*>(pNode->nodeDataModel()))
      {
        pDebuggableModel->SetDebuggState(state);
        pNode->nodeGraphicsObject().update();
      }
    }
  }
}
