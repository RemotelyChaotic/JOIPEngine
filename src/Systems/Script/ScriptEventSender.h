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

signals:
  void sendEvent(const QString& event, const QString& dataJson);
  void sendReturnValue(QJSValue value, QString sRequestEvtRet);

protected:
  std::shared_ptr<CScriptCommunicator>
  CreateCommunicatorImpl(std::shared_ptr<CScriptRunnerSignalEmiterAccessor> spAccessor) override;
};

//----------------------------------------------------------------------------------------
//
class CEventScriptCommunicator : public CScriptCommunicator
{
  public:
  CEventScriptCommunicator(const std::weak_ptr<CScriptRunnerSignalEmiterAccessor>& spEmitter);
  ~CEventScriptCommunicator() override;

  CScriptObjectBase* CreateNewScriptObject(QPointer<QJSEngine> pEngine) override;
  CScriptObjectBase* CreateNewScriptObject(QPointer<CJsonInstructionSetParser> pParser) override;
  CScriptObjectBase* CreateNewScriptObject(QtLua::State* pState) override;
  CScriptObjectBase* CreateNewSequenceObject() override;
};

//----------------------------------------------------------------------------------------
//
class CScriptEventSenderBase : public CJsScriptObjectBase
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptEventSenderBase)

public:
  CScriptEventSenderBase(std::weak_ptr<CScriptCommunicator> pCommunicator,
                         QPointer<QJSEngine> pEngine);
  CScriptEventSenderBase(std::weak_ptr<CScriptCommunicator> pCommunicator,
                         QtLua::State* pState);
  ~CScriptEventSenderBase();

signals:
  void SignalQuitLoop();

protected:
  void SendEventImpl(const QString& sEvent, const QString& sData);
  QVariant SendEventAndWaitImpl(const QString& sEvent, const QString& sData);

private:
  std::shared_ptr<std::function<void()>> m_spStop;
};

//----------------------------------------------------------------------------------------
//
class CScriptEventSenderJs : public CScriptEventSenderBase
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptEventSenderJs)

public:
  CScriptEventSenderJs(std::weak_ptr<CScriptCommunicator> pCommunicator,
                       QPointer<QJSEngine> pEngine);
  ~CScriptEventSenderJs();

public slots:
  void sendEvent(const QString& sEvent);
  void sendEvent(const QString& sEvent, QVariant data);
  QVariant sendEventAndWait(const QString& sEvent);
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
  CScriptEventSenderLua(std::weak_ptr<CScriptCommunicator> pCommunicator,
                         QtLua::State* pState);
  ~CScriptEventSenderLua();

public slots:
  void sendEvent(const QString& sEvent);
  void sendEvent(const QString& sEvent, QVariant data);
  QVariant sendEventAndWait(const QString& sEvent);
  QVariant sendEventAndWait(const QString& sEvent, QVariant data);

private:
  QString PrepareInput(const QVariant& data);
};

#endif // CSCRIPTEVENTSENDER_H
