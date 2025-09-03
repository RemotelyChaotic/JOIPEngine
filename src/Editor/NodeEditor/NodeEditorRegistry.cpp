#include "NodeEditorRegistry.h"

#include "Editor/NodeEditor/UndoPathSplitterModel.h"
#include "Editor/NodeEditor/UndoSceneNodeModel.h"

#include "Systems/Nodes/EndNodeModel.h"
#include "Systems/Nodes/PathMergerModel.h"
#include "Systems/Nodes/SceneTranstitionData.h"
#include "Systems/Nodes/StartNodeModel.h"

#include <nodes/DataModelRegistry>

using QtNodes::DataModelRegistry;

//----------------------------------------------------------------------------------------
//
std::shared_ptr<DataModelRegistry> CNodeEditorRegistry::RegisterDataModels()
{
  auto ret = std::make_shared<DataModelRegistry>();
  ret->registerModel<CStartNodeModel>(CNodeEditorRegistryBase::c_sModelCategoryControl);
  ret->registerModel<CUndoSceneNodeModelWithWidget>(CNodeEditorRegistryBase::c_sModelCategoryScene);
  ret->registerModel<CEndNodeModel>(CNodeEditorRegistryBase::c_sModelCategoryControl);
  ret->registerModel<CPathMergerModel>(CNodeEditorRegistryBase::c_sModelCategoryPath);
  ret->registerModel<CUndoPathSplitterModelWithWidget>(CNodeEditorRegistryBase::c_sModelCategoryPath);
  return ret;
}
