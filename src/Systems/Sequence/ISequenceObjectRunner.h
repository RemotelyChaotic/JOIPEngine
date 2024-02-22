#ifndef ISEQUENCEOBJECTRUNNER_H
#define ISEQUENCEOBJECTRUNNER_H

#include "Sequence.h"

class ISequenceObjectRunner
{
public:
  virtual ~ISequenceObjectRunner(){}

  virtual void RunSequenceInstruction(const std::shared_ptr<SSequenceInstruction>& spInstr) = 0;
};

#endif // ISEQUENCEOBJECTRUNNER_H
