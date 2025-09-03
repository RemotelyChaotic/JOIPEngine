#include "NodeGraphicsObjectProvider.h"

CNodeGraphicsObjectProvider::CNodeGraphicsObjectProvider(const CNodeGraphicsObjectProvider::tFnCreate& fn) :
  m_fn(fn)
{
}

CNodeGraphicsObjectProvider::~CNodeGraphicsObjectProvider() = default;

//----------------------------------------------------------------------------------------
//
std::unique_ptr<QtNodes::NodeGraphicsObject>
CNodeGraphicsObjectProvider::createObject(QtNodes::FlowScene& scene,
                                          QtNodes::Node& node) const
{
  return m_fn(scene, node);
}

//----------------------------------------------------------------------------------------
//
CDefaultGraphicsObjectProvider::CDefaultGraphicsObjectProvider() :
  CTypedNodeGraphicsObjectProvider<QtNodes::NodeGraphicsObject>()
{}
CDefaultGraphicsObjectProvider::~CDefaultGraphicsObjectProvider() = default;
