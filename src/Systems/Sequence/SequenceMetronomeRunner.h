#ifndef CSEQUENCEMETRONOMERUNNER_H
#define CSEQUENCEMETRONOMERUNNER_H

#include "ISequenceObjectRunner.h"
#include "Systems/Script/ScriptObjectBase.h"

#include <functional>
#include <map>

class CSequenceMetronomeRunner : public CScriptObjectBase,
                                 public ISequenceObjectRunner
{
public:
  CSequenceMetronomeRunner(QPointer<CScriptRunnerSignalEmiter> pEmitter);
  ~CSequenceMetronomeRunner() override;

  void RunSequenceInstruction(const std::shared_ptr<SSequenceInstruction>& spInstr) override;

private:
  void RunSingleBeat(const std::shared_ptr<SSequenceInstruction>& spInstr);
  void RunStartPattern(const std::shared_ptr<SSequenceInstruction>& spInstr);
  void RunStopPattern(const std::shared_ptr<SSequenceInstruction>& spInstr);

  std::map<QString, std::function<void(const std::shared_ptr<SSequenceInstruction>&)>>
      m_functionMap;
};

#endif // CSEQUENCEMETRONOMERUNNER_H
