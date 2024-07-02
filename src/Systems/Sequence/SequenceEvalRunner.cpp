#include "SequenceEvalRunner.h"

#include "Systems/Script/ScriptEval.h"

CSequenceEvalRunner::CSequenceEvalRunner(
    QPointer<CScriptRunnerSignalEmiter> pEmitter) :
  CScriptObjectBase(pEmitter),
  ISequenceObjectRunner()
{
}
CSequenceEvalRunner::~CSequenceEvalRunner()
{

}

//----------------------------------------------------------------------------------------
//
void CSequenceEvalRunner::RunSequenceInstruction(const QString&,
                                                 const std::shared_ptr<SSequenceInstruction>& spInstr,
                                                 const SProjectData&)
{
  auto pSignalEmitter = SignalEmitter<CEvalSignalEmiter>();
  if (const auto& spI = std::dynamic_pointer_cast<SEvalInstruction>(spInstr);
      nullptr != spI && nullptr != pSignalEmitter)
  {
    emit pSignalEmitter->evalQuery(spI->m_sScript);
  }
}
