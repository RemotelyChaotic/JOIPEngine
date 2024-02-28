#ifndef CSEQUENCETEXTBOXRUNNER_H
#define CSEQUENCETEXTBOXRUNNER_H

#include "ISequenceObjectRunner.h"
#include "Systems/Script/ScriptObjectBase.h"

class CSequenceTextBoxRunner : public CScriptObjectBase,
                               public ISequenceObjectRunner
{
public:
  CSequenceTextBoxRunner(QPointer<CScriptRunnerSignalEmiter> pEmitter);
  ~CSequenceTextBoxRunner() override;

  void RunSequenceInstruction(const QString& sName,
                              const std::shared_ptr<SSequenceInstruction>& spInstr) override;
};

#endif // CSEQUENCETEXTBOXRUNNER_H
