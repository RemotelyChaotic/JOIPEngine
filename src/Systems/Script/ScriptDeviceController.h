#ifndef CSCRIPTDEVICECONTROLLER_H
#define CSCRIPTDEVICECONTROLLER_H

#include "ScriptObjectBase.h"
#include "ScriptRunnerSignalEmiter.h"

class CDeviceControllerSignalEmitter : public CScriptRunnerSignalEmiter
{
  Q_OBJECT

public:
  CDeviceControllerSignalEmitter();
  ~CDeviceControllerSignalEmitter();

signals:
  void sendLinearCmd(double dDurationS, double dPosition);
  void sendRotateCmd(bool bClockwise, double dSpeed);
  void sendStopCmd();
  void sendVibrateCmd(double dSpeed);

protected:
  std::shared_ptr<CScriptCommunicator>
  CreateCommunicatorImpl(std::shared_ptr<CScriptRunnerSignalEmiterAccessor> spAccessor) override;
};

//----------------------------------------------------------------------------------------
//
class CDeviceControllerScriptCommunicator : public CScriptCommunicator
{
  public:
  CDeviceControllerScriptCommunicator(const std::weak_ptr<CScriptRunnerSignalEmiterAccessor>& spEmitter);
  ~CDeviceControllerScriptCommunicator() override;

  CScriptObjectBase* CreateNewScriptObject(QPointer<QJSEngine> pEngine) override;
  CScriptObjectBase* CreateNewScriptObject(QPointer<CJsonInstructionSetParser> pParser) override;
  CScriptObjectBase* CreateNewScriptObject(QtLua::State* pState) override;
  CScriptObjectBase* CreateNewSequenceObject() override;
};

//----------------------------------------------------------------------------------------
//
class CScriptDeviceController : public CJsScriptObjectBase
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptDeviceController)

public:
  CScriptDeviceController(std::weak_ptr<CScriptCommunicator> pCommunicator,
                          QPointer<QJSEngine> pEngine);
  CScriptDeviceController(std::weak_ptr<CScriptCommunicator> pCommunicator,
                          QtLua::State* pState);
  ~CScriptDeviceController();

public slots:
  void sendLinearCmd(double dDurationS, double dPosition);
  void sendRotateCmd(bool bClockwise, double dSpeed);
  void sendStopCmd();
  void sendVibrateCmd(double dSpeed);
};

#endif // CSCRIPTDEVICECONTROLLER_H
