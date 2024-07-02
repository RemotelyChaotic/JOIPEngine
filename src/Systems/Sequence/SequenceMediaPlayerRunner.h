#ifndef CSEQUENCEMEDIAPLAYERRUNNER_H
#define CSEQUENCEMEDIAPLAYERRUNNER_H

#include "ISequenceObjectRunner.h"
#include "Systems/Script/ScriptObjectBase.h"

#include <functional>
#include <map>

class CSequenceMediaPlayerRunner : public CScriptObjectBase,
                                   public ISequenceObjectRunner
{
public:
  CSequenceMediaPlayerRunner(QPointer<CScriptRunnerSignalEmiter> pEmitter);
  ~CSequenceMediaPlayerRunner() override;

  void RunSequenceInstruction(const QString& sName,
                              const std::shared_ptr<SSequenceInstruction>& spInstr,
                              const SProjectData& proj) override;

private:
  void RunShow(const std::shared_ptr<SSequenceInstruction>& spInstr);
  void RunPlayVideo(const std::shared_ptr<SSequenceInstruction>& spInstr);
  void RunPauseVideo(const std::shared_ptr<SSequenceInstruction>& spInstr);
  void RunStopVideo(const std::shared_ptr<SSequenceInstruction>& spInstr);
  void RunPlayAudio(const std::shared_ptr<SSequenceInstruction>& spInstr);
  void RunPauseAudio(const std::shared_ptr<SSequenceInstruction>& spInstr);
  void RunStopAudio(const std::shared_ptr<SSequenceInstruction>& spInstr);

  std::map<QString, std::function<void(const std::shared_ptr<SSequenceInstruction>&)>>
      m_functionMap;
};

#endif // CSEQUENCEMEDIAPLAYERRUNNER_H
