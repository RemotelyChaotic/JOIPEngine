#include "JsonInstructionNode.h"


CJsonInstructionNodeBuildData::CJsonInstructionNodeBuildData(CJsonInstructionNodeBuildData& other) :
  m_sName(other.m_sName),
  m_bIgnoreChildren(other.m_bIgnoreChildren),
  // not really an issue as these are only build time information
  m_argsDefinition(other.m_argsDefinition),
  m_argDefinitionArr(other.m_argDefinitionArr)
{

}

CJsonInstructionNode::CJsonInstructionNode(CJsonInstructionNode& other) :
  std::enable_shared_from_this<CJsonInstructionNode>(),
  CJsonInstructionNodeBuildData(other),
  // actually useful things
  m_wpCommand(other.m_wpCommand),
  m_actualArgs(other.m_actualArgs),
  m_bEnabled(other.m_bEnabled),
  m_spChildren(),
  m_wpParent(other.m_wpParent)
{
  // deep copy children
  for (auto& spChild : other.m_spChildren)
  {
    m_spChildren.push_back(std::make_shared<CJsonInstructionNode>(*spChild));
  }
}

//----------------------------------------------------------------------------------------
//
void CJsonInstructionNode::ReparentChildren()
{
  for (auto spChild : m_spChildren)
  {
    spChild->m_wpParent = shared_from_this();
    spChild->ReparentChildren(); // only on first layer all susequent children are adopted
  }
}
