#ifndef CSEQUENCEEVALRUNNER_H
#define CSEQUENCEEVALRUNNER_H

#include "ISequenceObjectRunner.h"
#include "Systems/Script/ScriptObjectBase.h"

class CSequenceEvalRunner : public CScriptObjectBase,
                            public ISequenceObjectRunner
{
public:
  CSequenceEvalRunner(QPointer<CScriptRunnerSignalEmiter> pEmitter);
  ~CSequenceEvalRunner() override;

  void RunSequenceInstruction(const std::shared_ptr<SSequenceInstruction>& spInstr) override;
};

#endif // CSEQUENCEEVALRUNNER_H
