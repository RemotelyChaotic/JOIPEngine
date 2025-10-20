#include "SequenceEvalRunner.h"

#include "Systems/Script/ScriptEval.h"

CSequenceEvalRunner::CSequenceEvalRunner(
    std::weak_ptr<CScriptCommunicator> pCommunicator) :
  CScriptObjectBase(pCommunicator),
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
  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CEvalSignalEmiter>())
    {
      if (const auto& spI = std::dynamic_pointer_cast<SEvalInstruction>(spInstr);
          nullptr != spI)
      {
        emit spSignalEmitter->evalQuery(spI->m_sScript);
      }
    }
  }
}
