#ifndef CEDITORNODEGRAPHICSOBJECT_H
#define CEDITORNODEGRAPHICSOBJECT_H

#include <nodes/internal/NodeGraphicsObject.hpp>

#include <QColor>

class CEditorNodeGraphicsObject : public QtNodes::NodeGraphicsObject
{
  Q_OBJECT

public:
  CEditorNodeGraphicsObject(QtNodes::FlowScene& scene,
                            QtNodes::Node& node);
  ~CEditorNodeGraphicsObject() override;

protected:
  void paint(QPainter*                       painter,
             QStyleOptionGraphicsItem const* option,
             QWidget*                        widget = 0) override;

protected:
  QtNodes::FlowScene& m_scene;
};

#endif // CEDITORNODEGRAPHICSOBJECT_H
