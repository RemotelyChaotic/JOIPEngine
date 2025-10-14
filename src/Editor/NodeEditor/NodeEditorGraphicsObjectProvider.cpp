#include "NodeEditorGraphicsObjectProvider.h"
#include "EditorNodeGraphicsObject.h"

#include <nodes/internal/NodeGraphicsObject.hpp>

CNodeEditorGraphicsObjectProvider::CNodeEditorGraphicsObjectProvider() :
    CNodeGraphicsObjectProvider([](QtNodes::FlowScene& scene, QtNodes::Node& node)
                                  -> std::unique_ptr<QtNodes::NodeGraphicsObject>{
      auto spGo = std::make_unique<CEditorNodeGraphicsObject>(scene, node);
      // We disable item chaching because it seems to not make it faster,
      // but the cache sometimes mistakenly thinks the item is not on screen.
      // Probably an issue with the splitter the view is placed in.
      // After updateing QtNodes the next time with Qt6 this will not be needed anyways.
      spGo->setCacheMode(QGraphicsObject::CacheMode::NoCache);
      return spGo;
    })
{
}

CNodeEditorGraphicsObjectProvider::~CNodeEditorGraphicsObjectProvider() = default;
