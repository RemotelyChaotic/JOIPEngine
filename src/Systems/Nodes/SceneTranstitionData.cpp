#include "SceneTranstitionData.h"

CSceneTranstitionData::CSceneTranstitionData() :
  NodeData()
{
}

CSceneTranstitionData::~CSceneTranstitionData()
{
}

//----------------------------------------------------------------------------------------
//
NodeDataType CSceneTranstitionData::type() const
{
  return NodeDataType {"transition",
                       "Transition"};
}
