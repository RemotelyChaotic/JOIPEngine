#ifndef CNODEEDITORREGISTRY_H
#define CNODEEDITORREGISTRY_H

#include "Systems/Nodes/NodeEditorRegistryBase.h"

#include <memory>

namespace QtNodes {
  class DataModelRegistry;
}

//----------------------------------------------------------------------------------------
//
class CNodeEditorRegistry : public CNodeEditorRegistryBase
{
public:
  static std::shared_ptr<QtNodes::DataModelRegistry> RegisterDataModels();
};

#endif // CNODEEDITORREGISTRY_H
