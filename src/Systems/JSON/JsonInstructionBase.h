#ifndef JSONINSTRUCTIONBASE_H
#define JSONINSTRUCTIONBASE_H

#include "JsonInstructionSetRunner.h"
#include <QVariant>
#include <tuple>

//----------------------------------------------------------------------------------------
//
class IJsonInstructionBase
{
public:
  typedef std::vector<std::tuple<QString, QString, qint32>> tChildNodeGroups;
  typedef std::variant<SJsonException,
                       SRunRetVal<ENextCommandToCall::eChild>,
                       SRunRetVal<ENextCommandToCall::eSibling>,
                       SRunRetVal<ENextCommandToCall::eForkThis>,
                       SRunRetVal<ENextCommandToCall::eFinish>,
                       std::true_type /*Default: dfs traversal of tree*/> tRetVal;

  IJsonInstructionBase() {}
  virtual ~IJsonInstructionBase() {}

  virtual tInstructionMapType& ArgList() = 0;
  virtual IJsonInstructionBase::tRetVal Call(const tInstructionMapValue& args) = 0;
  virtual tChildNodeGroups ChildNodeGroups(const tInstructionMapValue& args) const = 0;
};

#endif // JSONINSTRUCTIONBASE_H
