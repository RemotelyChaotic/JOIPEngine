#ifndef JSONINSTRUCTIONNODE_H
#define JSONINSTRUCTIONNODE_H

#include "JsonInstructionBase.h"
#include <memory>
#include <vector>

class CJsonInstructionNode : public std::enable_shared_from_this<CJsonInstructionNode>
{
public:
  CJsonInstructionNode() = default;
  CJsonInstructionNode(CJsonInstructionNode& other);

  void ReparentChildren();

  // build data
  QString                                            m_sName;
  bool                                               m_bIgnoreChildren = false;
  tInstructionMapType*                               m_argsDefinition = nullptr;
  // for arrays
  SInstructionArgumentType*                          m_argDefinitionArr = nullptr;

  // run data
  std::weak_ptr<IJsonInstructionBase>                m_wpCommand;
  tInstructionMapValue                               m_actualArgs;
  bool                                               m_bEnabled = true;

  // tree hierarchy
  std::vector<std::shared_ptr<CJsonInstructionNode>> m_spChildren;
  std::weak_ptr<CJsonInstructionNode>                m_wpParent;
};

Q_DECLARE_METATYPE(std::shared_ptr<CJsonInstructionNode>)

#endif // JSONINSTRUCTIONNODE_H
