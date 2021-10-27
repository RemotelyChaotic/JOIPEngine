#include "NodeEditorRegistry.h"

#include "Editor/NodeEditor/EndNodeModel.h"
#include "Editor/NodeEditor/PathMergerModel.h"
#include "Editor/NodeEditor/PathSplitterModel.h"
#include "Editor/NodeEditor/SceneNodeModel.h"
#include "Editor/NodeEditor/SceneTranstitionData.h"
#include "Editor/NodeEditor/StartNodeModel.h"

#include <nodes/DataModelRegistry>

using QtNodes::DataModelRegistry;

const char* CNodeEditorRegistry::c_sModelCategoryControl = "Control";
const char* CNodeEditorRegistry::c_sModelCategoryScene = "Scene";
const char* CNodeEditorRegistry::c_sModelCategoryPath = "Path";

//----------------------------------------------------------------------------------------
//
std::shared_ptr<DataModelRegistry> CNodeEditorRegistry::RegisterDataModels()
{
  auto ret = std::make_shared<DataModelRegistry>();
  ret->registerModel<CStartNodeModel>(CNodeEditorRegistry::c_sModelCategoryControl);
  ret->registerModel<CSceneNodeModelWithWidget>(CNodeEditorRegistry::c_sModelCategoryScene);
  ret->registerModel<CEndNodeModel>(CNodeEditorRegistry::c_sModelCategoryControl);
  ret->registerModel<CPathMergerModel>(CNodeEditorRegistry::c_sModelCategoryPath);
  ret->registerModel<CPathSplitterModelWithWidget>(CNodeEditorRegistry::c_sModelCategoryPath);
  return ret;
}

std::shared_ptr<QtNodes::DataModelRegistry> CNodeEditorRegistry::RegisterDataModelsWithoutUi()
{
  auto ret = std::make_shared<DataModelRegistry>();
  ret->registerModel<CStartNodeModel>(CNodeEditorRegistry::c_sModelCategoryControl);
  ret->registerModel<CSceneNodeModel>(CNodeEditorRegistry::c_sModelCategoryScene);
  ret->registerModel<CEndNodeModel>(CNodeEditorRegistry::c_sModelCategoryControl);
  ret->registerModel<CPathMergerModel>(CNodeEditorRegistry::c_sModelCategoryPath);
  ret->registerModel<CPathSplitterModel>(CNodeEditorRegistry::c_sModelCategoryPath);
  return ret;
}
