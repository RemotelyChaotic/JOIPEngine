#include "NodeEditorRegistryBase.h"

#include "Systems/Nodes/EndNodeModel.h"
#include "Systems/Nodes/PathMergerModel.h"
#include "Systems/Nodes/PathSplitterModel.h"
#include "Systems/Nodes/SceneNodeModel.h"
#include "Systems/Nodes/SceneTranstitionData.h"
#include "Systems/Nodes/StartNodeModel.h"

#include <nodes/DataModelRegistry>

using QtNodes::DataModelRegistry;

const char* CNodeEditorRegistryBase::c_sModelCategoryControl = "Control";
const char* CNodeEditorRegistryBase::c_sModelCategoryScene = "Scene";
const char* CNodeEditorRegistryBase::c_sModelCategoryPath = "Path";

std::shared_ptr<QtNodes::DataModelRegistry> CNodeEditorRegistryBase::RegisterDataModelsWithoutUi()
{
  auto ret = std::make_shared<DataModelRegistry>();
  ret->registerModel<CStartNodeModel>(CNodeEditorRegistryBase::c_sModelCategoryControl);
  ret->registerModel<CSceneNodeModel>(CNodeEditorRegistryBase::c_sModelCategoryScene);
  ret->registerModel<CEndNodeModel>(CNodeEditorRegistryBase::c_sModelCategoryControl);
  ret->registerModel<CPathMergerModel>(CNodeEditorRegistryBase::c_sModelCategoryPath);
  ret->registerModel<CPathSplitterModel>(CNodeEditorRegistryBase::c_sModelCategoryPath);
  return ret;
}
