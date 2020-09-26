#ifndef JSONINSTRUCTIONNODE_H
#define JSONINSTRUCTIONNODE_H

#include "JsonInstructionBase.h"
#include <memory>
#include <vector>

struct SJsonInstructionNode
{
  std::weak_ptr<IJsonInstructionBase>                m_wpCommand;
  QMap<QString, QVariant>                            m_actualArgs;
  std::vector<std::shared_ptr<SJsonInstructionNode>> m_spChildren;
  std::weak_ptr<SJsonInstructionNode>                m_wpParent;
};

#endif // JSONINSTRUCTIONNODE_H
