#ifndef CNODEEDITORREGISTRY_H
#define CNODEEDITORREGISTRY_H

#include <memory>

namespace QtNodes {
  class DataModelRegistry;
}

//----------------------------------------------------------------------------------------
//
class CNodeEditorRegistry
{
public:
  static const char* c_sModelCategoryControl;
  static const char* c_sModelCategoryScene;
  static const char* c_sModelCategoryPath;

  static std::shared_ptr<QtNodes::DataModelRegistry> RegisterDataModels();
  static std::shared_ptr<QtNodes::DataModelRegistry> RegisterDataModelsWithoutUi();
};

#endif // CNODEEDITORREGISTRY_H
