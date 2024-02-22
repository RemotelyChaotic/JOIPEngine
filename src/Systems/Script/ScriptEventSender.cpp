#include "ScriptEventSender.h"
#include "CommonScriptHelpers.h"
#include "ScriptRunnerSignalEmiter.h"

#include <QDebug>
#include <QEventLoop>
#include <QJsValue>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QTimer>

CEventSenderSignalEmitter::CEventSenderSignalEmitter() :
  CScriptRunnerSignalEmiter()
{

}
CEventSenderSignalEmitter::~CEventSenderSignalEmitter() = default;

//----------------------------------------------------------------------------------------
//
std::shared_ptr<CScriptObjectBase> CEventSenderSignalEmitter::CreateNewScriptObject(QPointer<QJSEngine> pEngine)
{
  return std::make_shared<CScriptEventSenderJs>(this, pEngine);
}
std::shared_ptr<CScriptObjectBase> CEventSenderSignalEmitter::CreateNewScriptObject(QtLua::State* pState)
{
  return std::make_shared<CScriptEventSenderLua>(this, pState);
}

//----------------------------------------------------------------------------------------
//
CScriptEventSenderBase::CScriptEventSenderBase(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                                               QPointer<QJSEngine> pEngine) :
  CJsScriptObjectBase(pEmitter, pEngine)
{
}
CScriptEventSenderBase::CScriptEventSenderBase(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                                               QtLua::State* pState) :
  CJsScriptObjectBase(pEmitter, pState)
{
}

CScriptEventSenderBase::~CScriptEventSenderBase() = default;

//----------------------------------------------------------------------------------------
//
void CScriptEventSenderBase::SendEventImpl(const QString& sEvent, const QString& sData)
{
  auto pSignalEmitter = SignalEmitter<CEventSenderSignalEmitter>();
  emit pSignalEmitter->sendEvent(sEvent, sData);
}

//----------------------------------------------------------------------------------------
//
QVariant CScriptEventSenderBase::SendEventAndWaitImpl(const QString& sEvent, const QString& sData)
{
  auto pSignalEmitter = SignalEmitter<CEventSenderSignalEmitter>();
  QTimer::singleShot(0, this, [&pSignalEmitter,sEvent,sData]() {
    emit pSignalEmitter->sendEvent(sEvent, sData);
  });

  // local loop to wait for answer
  QVariant varRetVal = QString();
  QEventLoop loop;
  QMetaObject::Connection quitLoop =
    connect(this, &CScriptEventSenderBase::SignalQuitLoop, &loop, &QEventLoop::quit);
  QMetaObject::Connection interruptLoop =
    connect(pSignalEmitter, &CEventSenderSignalEmitter::interrupt,
            &loop, &QEventLoop::quit, Qt::QueuedConnection);
  QMetaObject::Connection interruptThisLoop =
    connect(this, &CScriptObjectBase::SignalInterruptExecution,
            &loop, &QEventLoop::quit, Qt::QueuedConnection);
  QMetaObject::Connection showRetValLoop =
    connect(pSignalEmitter, &CEventSenderSignalEmitter::sendReturnValue,
            this, [this, &varRetVal, sEvent](QJSValue var, QString sRequestEvtRet)
  {
    if (sEvent == sRequestEvtRet)
    {
      varRetVal = var.toVariant();
      varRetVal.detach(); // fixes some crashes with QJSEngine
      emit this->SignalQuitLoop();
    }
    // direct connection to fix cross thread issues with QString content being deleted
  }, Qt::DirectConnection);
  loop.exec();
  loop.disconnect();

  disconnect(quitLoop);
  disconnect(interruptLoop);
  disconnect(interruptThisLoop);
  disconnect(showRetValLoop);

  return varRetVal;
}

//----------------------------------------------------------------------------------------
//
CScriptEventSenderJs::CScriptEventSenderJs(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                                           QPointer<QJSEngine> pEngine):
  CScriptEventSenderBase(pEmitter, pEngine)
{
}
CScriptEventSenderJs::~CScriptEventSenderJs() = default;

//----------------------------------------------------------------------------------------
//
void CScriptEventSenderJs::sendEvent(const QString& sEvent, QVariant data)
{
  if (!CheckIfScriptCanRun()) { return; }
  SendEventImpl(sEvent, PrepareInput(data));
}

//----------------------------------------------------------------------------------------
//
QVariant CScriptEventSenderJs::sendEventAndWait(const QString& sEvent, QVariant data)
{
  if (!CheckIfScriptCanRun()) { return QVariant(); }
  return SendEventAndWaitImpl(sEvent, PrepareInput(data));
}

//----------------------------------------------------------------------------------------
//
QString CScriptEventSenderJs::PrepareInput(const QVariant& data)
{
  QJSValue valFromJS = data.value<QJSValue>();
  if (data.isNull())
  {
    return QString("{}");
  }
  else if (valFromJS.isObject() || valFromJS.isArray())
  {
    QVariant varData = valFromJS.toVariant();
    QJsonDocument doc;
    doc.setObject(QJsonValue::fromVariant(varData).toObject());
    return doc.toJson(QJsonDocument::Compact);
  }
  else if (data.type() == QVariant::String || data.canConvert(QVariant::String))
  {
    return QString("{\"data\": \"%1\"}").arg(data.toString());
  }

  emit m_pSignalEmitter->showError(tr("Invalid data for event data."), QtMsgType::QtWarningMsg);
  return QString();
}

//----------------------------------------------------------------------------------------
//
CScriptEventSenderLua::CScriptEventSenderLua(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                                             QtLua::State* pState) :
  CScriptEventSenderBase(pEmitter, pState)
{
}
CScriptEventSenderLua::~CScriptEventSenderLua() = default;

//----------------------------------------------------------------------------------------
//
void CScriptEventSenderLua::sendEvent(const QString& sEvent, QVariant data)
{
  if (!CheckIfScriptCanRun()) { return; }
  SendEventImpl(sEvent, PrepareInput(data));
}

//----------------------------------------------------------------------------------------
//
QVariant CScriptEventSenderLua::sendEventAndWait(const QString& sEvent, QVariant data)
{
  if (!CheckIfScriptCanRun()) { return QVariant(); }
  return SendEventAndWaitImpl(sEvent, PrepareInput(data));
}

//----------------------------------------------------------------------------------------
//
QString CScriptEventSenderLua::PrepareInput(const QVariant& data)
{
  if (data.isNull())
  {
    return QString("{}");
  }
  else if (data.type() == QVariant::Map || data.type() == QVariant::List)
  {
    QVariant varData = script::ConvertLuaVariant(data);
    QJsonDocument doc;
    doc.setObject(QJsonValue::fromVariant(varData).toObject());
    return doc.toJson(QJsonDocument::Compact);
  }
  else if (data.type() == QVariant::String || data.canConvert(QVariant::String))
  {
    return QString("{\"data\": \"%1\"}").arg(data.toString());
  }

  emit m_pSignalEmitter->showError(tr("Invalid data for event data."), QtMsgType::QtWarningMsg);
  return QString();
}
