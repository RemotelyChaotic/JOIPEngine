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

  std::shared_ptr<CScriptObjectBase> CreateNewScriptObject(QPointer<QJSEngine> pEngine) override;
  std::shared_ptr<CScriptObjectBase> CreateNewScriptObject(QPointer<CJsonInstructionSetParser> pParser) override;
  std::shared_ptr<CScriptObjectBase> CreateNewScriptObject(QtLua::State* pState) override;

signals:
  void sendLinearCmd(quint32 iDurationMs, double dPosition);
  void sendRotateCmd(bool bClockwise, double dSpeed);
  void sendStopCmd();
  void sendVibrateCmd(double dSpeed);
};
Q_DECLARE_METATYPE(CDeviceControllerSignalEmitter)

//----------------------------------------------------------------------------------------
//
class CScriptDeviceController : public CJsScriptObjectBase
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptDeviceController)

public:
  CScriptDeviceController(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                          QPointer<QJSEngine> pEngine);
  CScriptDeviceController(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                          QtLua::State* pState);
  ~CScriptDeviceController();

public slots:
  void sendLinearCmd(quint32 iDurationMs, double dPosition);
  void sendRotateCmd(bool bClockwise, double dSpeed);
  void sendStopCmd();
  void sendVibrateCmd(double dSpeed);
};

#endif // CSCRIPTDEVICECONTROLLER_H
