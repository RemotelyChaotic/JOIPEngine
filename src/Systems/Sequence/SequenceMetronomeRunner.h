#ifndef CSEQUENCEMETRONOMERUNNER_H
#define CSEQUENCEMETRONOMERUNNER_H

#include "ISequenceObjectRunner.h"
#include "Systems/Script/ScriptObjectBase.h"

#include <functional>
#include <map>

class CMetronomeManager;
class CSequenceMetronomeRunner : public CScriptObjectBase,
                                 public ISequenceObjectRunner
{
public:
  CSequenceMetronomeRunner(QPointer<CScriptRunnerSignalEmiter> pEmitter);
  ~CSequenceMetronomeRunner() override;

  void RunSequenceInstruction(const QString& sName,
                              const std::shared_ptr<SSequenceInstruction>& spInstr) override;

private:
  void RunSingleBeat(const QString& sName, const std::shared_ptr<SSequenceInstruction>& spInstr);
  void RunStartPattern(const QString& sName, const std::shared_ptr<SSequenceInstruction>& spInstr);
  void RunStopPattern(const QString& sName, const std::shared_ptr<SSequenceInstruction>& spInstr);

  std::map<QString, std::function<void(const QString&,const std::shared_ptr<SSequenceInstruction>&)>>
      m_functionMap;
  std::weak_ptr<CMetronomeManager> m_wpMetronomeManager;
};

#endif // CSEQUENCEMETRONOMERUNNER_H
