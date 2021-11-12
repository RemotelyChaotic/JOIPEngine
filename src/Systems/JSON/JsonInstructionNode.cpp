#include "JsonInstructionNode.h"

CJsonInstructionNode::CJsonInstructionNode(CJsonInstructionNode& other) :
  m_sName(other.m_sName),
  m_bIgnoreChildren(other.m_bIgnoreChildren),
  // not really an issue as these are only build time information
  m_argsDefinition(other.m_argsDefinition),
  m_argDefinitionArr(other.m_argDefinitionArr),
  // actually useful things
  m_wpCommand(other.m_wpCommand),
  m_actualArgs(other.m_actualArgs),
  m_spChildren(),
  m_wpParent(other.m_wpParent)
{
  // deep copy children
  for (auto& spChild : other.m_spChildren)
  {
    m_spChildren.push_back(std::make_shared<CJsonInstructionNode>(*spChild));
    m_spChildren.back()->m_wpParent =  shared_from_this();
  }
}
