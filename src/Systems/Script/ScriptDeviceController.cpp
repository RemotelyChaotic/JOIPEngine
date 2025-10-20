#include "ScriptDeviceController.h"

#include "Systems/Sequence/SequenceDeviceControllerRunner.h"

CDeviceControllerSignalEmitter::CDeviceControllerSignalEmitter() :  CScriptRunnerSignalEmiter()
{
}
CDeviceControllerSignalEmitter::~CDeviceControllerSignalEmitter()
{
}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<CScriptCommunicator>
CDeviceControllerSignalEmitter::CreateCommunicatorImpl(std::shared_ptr<CScriptRunnerSignalEmiterAccessor> spAccessor)
{
  return std::make_shared<CDeviceControllerScriptCommunicator>(spAccessor);
}

//----------------------------------------------------------------------------------------
//
CDeviceControllerScriptCommunicator::CDeviceControllerScriptCommunicator(
  const std::weak_ptr<CScriptRunnerSignalEmiterAccessor>& spEmitter) :
  CScriptCommunicator(spEmitter)
{}
CDeviceControllerScriptCommunicator::~CDeviceControllerScriptCommunicator() = default;

//----------------------------------------------------------------------------------------
//
CScriptObjectBase* CDeviceControllerScriptCommunicator::CreateNewScriptObject(QPointer<QJSEngine> pEngine)
{
  return new CScriptDeviceController(weak_from_this(), pEngine);
}
CScriptObjectBase* CDeviceControllerScriptCommunicator::CreateNewScriptObject(QPointer<CJsonInstructionSetParser> pParser)
{
  Q_UNUSED(pParser)
  return nullptr;
}
CScriptObjectBase* CDeviceControllerScriptCommunicator::CreateNewScriptObject(QtLua::State* pState)
{
  return new CScriptDeviceController(weak_from_this(), pState);
}
CScriptObjectBase* CDeviceControllerScriptCommunicator::CreateNewSequenceObject()
{
  return new CSequenceDeviceControllerRunner(weak_from_this());
}

//----------------------------------------------------------------------------------------
//
CScriptDeviceController::CScriptDeviceController(std::weak_ptr<CScriptCommunicator> pCommunicator,
                                                 QPointer<QJSEngine> pEngine):
  CJsScriptObjectBase(pCommunicator, pEngine)
{
}
CScriptDeviceController::CScriptDeviceController(std::weak_ptr<CScriptCommunicator> pCommunicator,
                                                 QtLua::State* pState):
  CJsScriptObjectBase(pCommunicator, pState)
{
}
CScriptDeviceController::~CScriptDeviceController()
{
}

//----------------------------------------------------------------------------------------
//
void CScriptDeviceController::sendLinearCmd(double dDurationS, double dPosition)
{
  if (!CheckIfScriptCanRun()) { return; }

  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CDeviceControllerSignalEmitter>())
    {
      emit spSignalEmitter->sendLinearCmd(dDurationS, dPosition);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptDeviceController::sendRotateCmd(bool bClockwise, double dSpeed)
{
  if (!CheckIfScriptCanRun()) { return; }
  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CDeviceControllerSignalEmitter>())
    {
      emit spSignalEmitter->sendRotateCmd(bClockwise, dSpeed);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptDeviceController::sendStopCmd()
{
  if (!CheckIfScriptCanRun()) { return; }
  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CDeviceControllerSignalEmitter>())
    {
      emit spSignalEmitter->sendStopCmd();
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptDeviceController::sendVibrateCmd(double dSpeed)
{
  if (!CheckIfScriptCanRun()) { return; }
  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CDeviceControllerSignalEmitter>())
    {
      emit spSignalEmitter->sendVibrateCmd(dSpeed);
    }
  }
}
