#ifndef ISEQUENCEOBJECTRUNNER_H
#define ISEQUENCEOBJECTRUNNER_H

#include "Sequence.h"
#include <memory>

struct SProjectData;

class ISequenceObjectRunner
{
public:
  virtual ~ISequenceObjectRunner(){}

  virtual void RunSequenceInstruction(const QString& sName,
                                      const std::shared_ptr<SSequenceInstruction>& spInstr,
                                      const SProjectData& proj) = 0;
};

#endif // ISEQUENCEOBJECTRUNNER_H
