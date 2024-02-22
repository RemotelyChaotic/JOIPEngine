#ifndef CSCRIPTEVENTSENDER_H
#define CSCRIPTEVENTSENDER_H

#include "ScriptObjectBase.h"
#include "ScriptRunnerSignalEmiter.h"
#include <QVariant>
#include <memory>

class CEventSenderSignalEmitter : public CScriptRunnerSignalEmiter
{
  Q_OBJECT
public:
  CEventSenderSignalEmitter();
  ~CEventSenderSignalEmitter();

  std::shared_ptr<CScriptObjectBase> CreateNewScriptObject(QPointer<QJSEngine> pEngine) override;
  std::shared_ptr<CScriptObjectBase> CreateNewScriptObject(QtLua::State* pState) override;

signals:
  void sendEvent(const QString& event, const QString& dataJson);
  void sendReturnValue(QJSValue value, QString sRequestEvtRet);
};
Q_DECLARE_METATYPE(CEventSenderSignalEmitter)

//----------------------------------------------------------------------------------------
//
class CScriptEventSenderBase : public CJsScriptObjectBase
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptEventSenderBase)

public:
  CScriptEventSenderBase(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                         QPointer<QJSEngine> pEngine);
  CScriptEventSenderBase(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                         QtLua::State* pState);
  ~CScriptEventSenderBase();

signals:
  void SignalQuitLoop();

protected:
  void SendEventImpl(const QString& sEvent, const QString& sData);
  QVariant SendEventAndWaitImpl(const QString& sEvent, const QString& sData);
};

//----------------------------------------------------------------------------------------
//
class CScriptEventSenderJs : public CScriptEventSenderBase
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptEventSenderJs)

public:
  CScriptEventSenderJs(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                       QPointer<QJSEngine> pEngine);
  ~CScriptEventSenderJs();

public slots:
  void sendEvent(const QString& sEvent, QVariant data);
  QVariant sendEventAndWait(const QString& sEvent, QVariant data);

private:
  QString PrepareInput(const QVariant& data);
};

//----------------------------------------------------------------------------------------
//
class CScriptEventSenderLua : public CScriptEventSenderBase
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptEventSenderLua)

public:
  CScriptEventSenderLua(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                         QtLua::State* pState);
  ~CScriptEventSenderLua();

public slots:
  void sendEvent(const QString& sEvent, QVariant data);
  QVariant sendEventAndWait(const QString& sEvent, QVariant data);

private:
  QString PrepareInput(const QVariant& data);
};

#endif // CSCRIPTEVENTSENDER_H
