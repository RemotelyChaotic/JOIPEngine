#include "SequenceDeviceControllerRunner.h"

#include "Systems/Script/ScriptDeviceController.h"

using namespace std::placeholders;

CSequenceDeviceControllerRunner::CSequenceDeviceControllerRunner(
    std::weak_ptr<CScriptCommunicator> pCommunicator) :
  CScriptObjectBase(pCommunicator),
  ISequenceObjectRunner(),
  m_functionMap({{sequence::c_sInstructionIdVibrate, std::bind(&CSequenceDeviceControllerRunner::RunVibrate, this, _1)},
                 {sequence::c_sInstructionIdLinearToy, std::bind(&CSequenceDeviceControllerRunner::RunLinear, this, _1)},
                 {sequence::c_sInstructionIdRotateToy, std::bind(&CSequenceDeviceControllerRunner::RunRotate, this, _1)},
                 {sequence::c_sInstructionIdStopVibrations, std::bind(&CSequenceDeviceControllerRunner::RunStop, this, _1)}})
{
}
CSequenceDeviceControllerRunner::~CSequenceDeviceControllerRunner()
{

}

//----------------------------------------------------------------------------------------
//
void CSequenceDeviceControllerRunner::RunSequenceInstruction(const QString&,
                                                             const std::shared_ptr<SSequenceInstruction>& spInstr,
                                                             const SProjectData&)
{
  auto it = m_functionMap.find(spInstr->m_sInstructionType);
  if (m_functionMap.end() != it)
  {
    it->second(spInstr);
  }
}

//----------------------------------------------------------------------------------------
//
void CSequenceDeviceControllerRunner::RunVibrate(const std::shared_ptr<SSequenceInstruction>& spInstr)
{
  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CDeviceControllerSignalEmitter>())
    {
      if (const auto& spI = std::dynamic_pointer_cast<SVibrateInstruction>(spInstr);
          nullptr != spI)
      {
        emit spSignalEmitter->sendVibrateCmd(spI->m_dSpeed);
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CSequenceDeviceControllerRunner::RunLinear(const std::shared_ptr<SSequenceInstruction>& spInstr)
{
  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CDeviceControllerSignalEmitter>())
    {
      if (const auto& spI = std::dynamic_pointer_cast<SLinearToyInstruction>(spInstr);
          nullptr != spI)
      {
        emit spSignalEmitter->sendLinearCmd(spI->m_dDurationS, spI->m_dPosition);
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CSequenceDeviceControllerRunner::RunRotate(const std::shared_ptr<SSequenceInstruction>& spInstr)
{
  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CDeviceControllerSignalEmitter>())
    {
      if (const auto& spI = std::dynamic_pointer_cast<SRotateToyInstruction>(spInstr);
          nullptr != spI)
      {
        emit spSignalEmitter->sendRotateCmd(spI->m_bClockwise, spI->m_dSpeed);
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CSequenceDeviceControllerRunner::RunStop(const std::shared_ptr<SSequenceInstruction>& spInstr)
{
  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CDeviceControllerSignalEmitter>())
    {
      if (const auto& spI = std::dynamic_pointer_cast<SStopVibrationsInstruction>(spInstr);
          nullptr != spI)
      {
        emit spSignalEmitter->sendStopCmd();
      }
    }
  }
}
