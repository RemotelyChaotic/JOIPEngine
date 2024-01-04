#include "ScriptDeviceController.h"

CDeviceControllerSignalEmitter::CDeviceControllerSignalEmitter() :  CScriptRunnerSignalEmiter()
{
}
CDeviceControllerSignalEmitter::~CDeviceControllerSignalEmitter()
{
}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<CScriptObjectBase> CDeviceControllerSignalEmitter::CreateNewScriptObject(QPointer<QJSEngine> pEngine)
{
  return std::make_shared<CScriptDeviceController>(this, pEngine);
}

std::shared_ptr<CScriptObjectBase> CDeviceControllerSignalEmitter::CreateNewScriptObject(QPointer<CJsonInstructionSetParser> pParser)
{
  Q_UNUSED(pParser)
  return nullptr;
}

std::shared_ptr<CScriptObjectBase> CDeviceControllerSignalEmitter::CreateNewScriptObject(QtLua::State* pState)
{
  return std::make_shared<CScriptDeviceController>(this, pState);
}

//----------------------------------------------------------------------------------------
//
CScriptDeviceController::CScriptDeviceController(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                                                 QPointer<QJSEngine> pEngine):
  CJsScriptObjectBase(pEmitter, pEngine)
{
}
CScriptDeviceController::CScriptDeviceController(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                                                 QtLua::State* pState):
  CJsScriptObjectBase(pEmitter, pState)
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
  auto spSignalEmitter = SignalEmitter<CDeviceControllerSignalEmitter>();
  if (nullptr != spSignalEmitter)
  {
    emit spSignalEmitter->sendLinearCmd(dDurationS, dPosition);
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptDeviceController::sendRotateCmd(bool bClockwise, double dSpeed)
{
  if (!CheckIfScriptCanRun()) { return; }
  auto spSignalEmitter = SignalEmitter<CDeviceControllerSignalEmitter>();
  if (nullptr != spSignalEmitter)
  {
    emit spSignalEmitter->sendRotateCmd(bClockwise, dSpeed);
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptDeviceController::sendStopCmd()
{
  if (!CheckIfScriptCanRun()) { return; }
  auto spSignalEmitter = SignalEmitter<CDeviceControllerSignalEmitter>();
  if (nullptr != spSignalEmitter)
  {
    emit spSignalEmitter->sendStopCmd();
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptDeviceController::sendVibrateCmd(double dSpeed)
{
  if (!CheckIfScriptCanRun()) { return; }
  auto spSignalEmitter = SignalEmitter<CDeviceControllerSignalEmitter>();
  if (nullptr != spSignalEmitter)
  {
    emit spSignalEmitter->sendVibrateCmd(dSpeed);
  }
}
