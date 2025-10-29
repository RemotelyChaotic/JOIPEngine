#include "EditorNodeGraphicsObject.h"

#include "Systems/Nodes/NodeModelBase.h"

#include <nodes/internal/NodePainter.hpp>

#include <nodes/FlowScene>
#include <nodes/Node>
#include <nodes/NodeGeometry>
#include <nodes/NodeState>

#include <QPainter>
#include <QStyleOption>

CEditorNodeGraphicsObject::CEditorNodeGraphicsObject(QtNodes::FlowScene& scene,
                                                     QtNodes::Node& node) :
    QtNodes::NodeGraphicsObject(scene, node),
    m_scene(scene)
{
}

CEditorNodeGraphicsObject::~CEditorNodeGraphicsObject() = default;

//----------------------------------------------------------------------------------------
//
namespace
{
  void drawNodeRect(QPainter* painter,
                    QtNodes::NodeGeometry const& geom,
                    QtNodes::NodeDataModel const* model,
                    QtNodes::NodeGraphicsObject const & graphicsObject,
                    const QColor& nodeDebugColor,
                    bool bBeingdebugged)
  {
    QtNodes::NodeStyle const& nodeStyle = model->nodeStyle();

    auto color = graphicsObject.isSelected()
                     ? nodeStyle.SelectedBoundaryColor
                     : nodeStyle.NormalBoundaryColor;

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

    if (!bBeingdebugged)
    {
      QLinearGradient gradient(QPointF(0.0, 0.0),
                               QPointF(2.0, geom.height()));

      gradient.setColorAt(0.0, nodeStyle.GradientColor0);
      gradient.setColorAt(0.03, nodeStyle.GradientColor1);
      gradient.setColorAt(0.97, nodeStyle.GradientColor2);
      gradient.setColorAt(1.0, nodeStyle.GradientColor3);

      painter->setBrush(gradient);
    }
    else
    {
      QLinearGradient gradient(QPointF(0.0, 0.0),
                               QPointF(2.0, geom.height()));

      gradient.setColorAt(0.0, nodeDebugColor);
      gradient.setColorAt(0.03, nodeDebugColor);
      gradient.setColorAt(0.97, nodeStyle.GradientColor1);
      gradient.setColorAt(1.0, nodeStyle.GradientColor1);

      painter->setBrush(gradient);
    }

    float diam = nodeStyle.ConnectionPointDiameter;

    QRectF boundary( -diam, -diam, 2.0 * diam + geom.width(), 2.0 * diam + geom.height());

    double const radius = 3.0;

    painter->drawRoundedRect(boundary, radius, radius);
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorNodeGraphicsObject::paint(QPainter* pPainter,
                                      QStyleOptionGraphicsItem const* pOption,
                                      QWidget*                        pWidget)
{
  Q_UNUSED(pWidget)

  pPainter->setClipRect(pOption->exposedRect);

  //adjusted from 3rdparty\nodeeditor\src\NodePainter.cpp

  QtNodes::Node& _node = node();

  QtNodes::NodeGeometry const& geom = _node.nodeGeometry();

  QtNodes::NodeState const& state = _node.nodeState();

  NodeGraphicsObject const & graphicsObject = _node.nodeGraphicsObject();

  // this is slow, so don't do this, the font won't be changed anyways
  //geom.recalculateSize(pPainter->font());

  //--------------------------------------------
  QtNodes::NodeDataModel const * model = _node.nodeDataModel();

  QtNodes::NodeStyle const& nodeStyle = model->nodeStyle();

  bool bBeingdebugged = false;
  QColor col;
  if (auto pModel = dynamic_cast<CNodeModelBase const *>(model))
  {
    bBeingdebugged = pModel->DebugState() != CNodeModelBase::eNotDebugged;
    switch (pModel->DebugState())
    {
      case CNodeModelBase::eDebugPassive:
        col = nodeStyle.WarningColor;
        break;
      case CNodeModelBase::eDebugActive:
        col = nodeStyle.ErrorColor;
        break;
      case CNodeModelBase::eNotDebugged:
        break;
    }
  }
  drawNodeRect(pPainter, geom, model, graphicsObject,
               col, bBeingdebugged);

  QtNodes::NodePainter::drawConnectionPoints(pPainter, geom, state, model, m_scene);

  QtNodes::NodePainter::drawFilledConnectionPoints(pPainter, geom, state, model);

  QtNodes::NodePainter::drawModelName(pPainter, geom, state, model);

  QtNodes::NodePainter::drawEntryLabels(pPainter, geom, state, model);

  QtNodes::NodePainter::drawResizeRect(pPainter, geom, model);

  QtNodes::NodePainter::drawValidationRect(pPainter, geom, model, graphicsObject);

  /// call custom painter
  if (auto painterDelegate = model->painterDelegate())
  {
    painterDelegate->paint(pPainter, geom, model);
  }
}
