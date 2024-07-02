#ifndef CSEQUENCEDEVICECONTROLLERRUNNER_H
#define CSEQUENCEDEVICECONTROLLERRUNNER_H

#include "ISequenceObjectRunner.h"
#include "Systems/Script/ScriptObjectBase.h"

#include <functional>
#include <map>

class CSequenceDeviceControllerRunner : public CScriptObjectBase,
                                        public ISequenceObjectRunner
{
public:
  CSequenceDeviceControllerRunner(QPointer<CScriptRunnerSignalEmiter> pEmitter);
  ~CSequenceDeviceControllerRunner() override;

  void RunSequenceInstruction(const QString& sName,
                              const std::shared_ptr<SSequenceInstruction>& spInstr,
                              const SProjectData& proj) override;

private:
  void RunVibrate(const std::shared_ptr<SSequenceInstruction>& spInstr);
  void RunLinear(const std::shared_ptr<SSequenceInstruction>& spInstr);
  void RunRotate(const std::shared_ptr<SSequenceInstruction>& spInstr);
  void RunStop(const std::shared_ptr<SSequenceInstruction>& spInstr);

  std::map<QString, std::function<void(const std::shared_ptr<SSequenceInstruction>&)>>
      m_functionMap;
};

#endif // CSEQUENCEDEVICECONTROLLERRUNNER_H
