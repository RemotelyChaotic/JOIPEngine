#ifndef CNODEEDITORREGISTRY_BASE_H
#define CNODEEDITORREGISTRY_BASE_H

#include <memory>

namespace QtNodes {
  class DataModelRegistry;
}

//----------------------------------------------------------------------------------------
//
class CNodeEditorRegistryBase
{
public:
  static const char* c_sModelCategoryControl;
  static const char* c_sModelCategoryScene;
  static const char* c_sModelCategoryPath;

  static std::shared_ptr<QtNodes::DataModelRegistry> RegisterDataModelsWithoutUi();
};

#endif // CNODEEDITORREGISTRY_BASE_H
