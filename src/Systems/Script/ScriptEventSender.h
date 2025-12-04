#ifndef CSCRIPTEVENTSENDER_H
#define CSCRIPTEVENTSENDER_H

#include "ScriptObjectBase.h"
#include "ScriptRunnerSignalEmiter.h"

#include <QtLua/Value>

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

  void SendReturnValuePrivate(QVariant value, const QString& sRequestEvtRet);

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
  virtual void HandleEventRet(QVariant value, const QString& sRequestRet) = 0;

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

  void registerEventHandler(const QString& sEvent, QJSValue callback);

protected:
  void HandleEventRet(QVariant value, const QString& sRequestRet) override;

private:
  QString PrepareInput(const QVariant& data);

  std::map<QString, QJSValue> m_callbacks;
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

  void registerEventHandler(const QString& sEvent, QVariant callback);

protected:
  void HandleEventRet(QVariant value, const QString& sRequestRet) override;

private:
  QString PrepareInput(const QVariant& data);

  std::map<QString, QtLua::Value> m_callbacks;
};

#endif // CSCRIPTEVENTSENDER_H
