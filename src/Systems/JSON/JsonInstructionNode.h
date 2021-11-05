#ifndef JSONINSTRUCTIONNODE_H
#define JSONINSTRUCTIONNODE_H

#include "JsonInstructionBase.h"
#include <memory>
#include <vector>

struct SJsonInstructionNode
{
  // build data
  QString                                            m_sName;
  bool                                               m_bIgnoreChildren = false;
  tInstructionMapType*                               m_argsDefinition = nullptr;
  // for arrays
  SInstructionArgumentType*                          m_argDefinitionArr = nullptr;

  // run data
  std::weak_ptr<IJsonInstructionBase>                m_wpCommand;
  tInstructionMapValue                               m_actualArgs;

  // tree hierarchy
  std::vector<std::shared_ptr<SJsonInstructionNode>> m_spChildren;
  std::weak_ptr<SJsonInstructionNode>                m_wpParent;
};

#endif // JSONINSTRUCTIONNODE_H
