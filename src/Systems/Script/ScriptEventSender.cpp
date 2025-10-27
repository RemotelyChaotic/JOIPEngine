#include "ScriptEventSender.h"
#include "CommonScriptHelpers.h"
#include "ScriptRunnerSignalEmiter.h"

#include <QDebug>
#include <QEventLoop>
#include <QJSValue>
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
std::shared_ptr<CScriptCommunicator>
CEventSenderSignalEmitter::CreateCommunicatorImpl(std::shared_ptr<CScriptRunnerSignalEmiterAccessor> spAccessor)
{
  return std::make_shared<CEventScriptCommunicator>(spAccessor);
}

//----------------------------------------------------------------------------------------
//
CEventScriptCommunicator::CEventScriptCommunicator(
  const std::weak_ptr<CScriptRunnerSignalEmiterAccessor>& spEmitter) :
  CScriptCommunicator(spEmitter)
{}
CEventScriptCommunicator::~CEventScriptCommunicator() = default;

//----------------------------------------------------------------------------------------
//
CScriptObjectBase* CEventScriptCommunicator::CreateNewScriptObject(QPointer<QJSEngine> pEngine)
{
  return new CScriptEventSenderJs(weak_from_this(), pEngine);
}
CScriptObjectBase* CEventScriptCommunicator::CreateNewScriptObject(QPointer<CJsonInstructionSetParser> pParser)
{
  Q_UNUSED(pParser)
  return nullptr;
}
CScriptObjectBase* CEventScriptCommunicator::CreateNewScriptObject(QtLua::State* pState)
{
  return new CScriptEventSenderLua(weak_from_this(), pState);
}
CScriptObjectBase* CEventScriptCommunicator::CreateNewSequenceObject()
{
  return nullptr;
}

//----------------------------------------------------------------------------------------
//
CScriptEventSenderBase::CScriptEventSenderBase(std::weak_ptr<CScriptCommunicator> pCommunicator,
                                               QPointer<QJSEngine> pEngine) :
  CJsScriptObjectBase(pCommunicator, pEngine)
{
  m_spStop = std::make_shared<std::function<void()>>([this]() {
    emit SignalQuitLoop();
  });
  if (auto spComm = pCommunicator.lock())
  {
    spComm->RegisterStopCallback(m_spStop);
  }
}
CScriptEventSenderBase::CScriptEventSenderBase(std::weak_ptr<CScriptCommunicator> pCommunicator,
                                               QtLua::State* pState) :
  CJsScriptObjectBase(pCommunicator, pState)
{
  m_spStop = std::make_shared<std::function<void()>>([this]() {
    emit SignalQuitLoop();
  });
  if (auto spComm = pCommunicator.lock())
  {
    spComm->RegisterStopCallback(m_spStop);
  }
}

CScriptEventSenderBase::~CScriptEventSenderBase()
{
  if (auto spComm = m_wpCommunicator.lock())
  {
    spComm->RemoveStopCallback(m_spStop);
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptEventSenderBase::SendEventImpl(const QString& sEvent, const QString& sData)
{
  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CEventSenderSignalEmitter>())
    {
      emit spSignalEmitter->sendEvent(sEvent, sData);
    }
  }
}

//----------------------------------------------------------------------------------------
//
QVariant CScriptEventSenderBase::SendEventAndWaitImpl(const QString& sEvent, const QString& sData)
{
  QTimer::singleShot(0, this, [this, sEvent, sData]() {
    if (auto spComm = m_wpCommunicator.lock())
    {
      if (auto spSignalEmitter = spComm->LockedEmitter<CEventSenderSignalEmitter>())
      {
        emit spSignalEmitter->sendEvent(sEvent, sData);
      }
    }
  });

  // local loop to wait for answer
  QPointer<CScriptEventSenderBase> pThis(this);
  std::shared_ptr<QVariant> spReturnValue = std::make_shared<QVariant>();
  QString sEvt = sEvent;
  sEvt.detach();

  QEventLoop loop;
  QMetaObject::Connection quitLoop =
    connect(this, &CScriptEventSenderBase::SignalQuitLoop, &loop, &QEventLoop::quit);
  QMetaObject::Connection interruptThisLoop =
    connect(this, &CScriptObjectBase::SignalInterruptExecution,
            &loop, &QEventLoop::quit, Qt::QueuedConnection);
  QMetaObject::Connection showRetValLoop;
  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CEventSenderSignalEmitter>())
    {
      showRetValLoop =
        connect(spSignalEmitter.Get(), &CEventSenderSignalEmitter::sendReturnValue,
                &loop, [&loop, spReturnValue, sEvt](QJSValue var, QString sRequestEvtRet)
      {
        if (sEvt == sRequestEvtRet)
        {
          *spReturnValue = var.toVariant();
          spReturnValue->detach(); // fixes some crashes with QJSEngine
          bool bOk = QMetaObject::invokeMethod(&loop, "quit", Qt::QueuedConnection);
          assert(bOk); Q_UNUSED(bOk)
        }
        // direct connection to fix cross thread issues with QString content being deleted
      }, Qt::DirectConnection);
    }
  }
  loop.exec();
  loop.disconnect();

  if (nullptr != pThis)
  {
    disconnect(quitLoop);
    disconnect(interruptThisLoop);
    if (showRetValLoop) disconnect(showRetValLoop);
  }

  return *spReturnValue;
}

//----------------------------------------------------------------------------------------
//
CScriptEventSenderJs::CScriptEventSenderJs(std::weak_ptr<CScriptCommunicator> pCommunicator,
                                           QPointer<QJSEngine> pEngine):
  CScriptEventSenderBase(pCommunicator, pEngine)
{
}
CScriptEventSenderJs::~CScriptEventSenderJs() = default;

//----------------------------------------------------------------------------------------
//
void CScriptEventSenderJs::sendEvent(const QString& sEvent)
{
  sendEvent(sEvent, QVariant());
}

//----------------------------------------------------------------------------------------
//
void CScriptEventSenderJs::sendEvent(const QString& sEvent, QVariant data)
{
  if (!CheckIfScriptCanRun()) { return; }
  SendEventImpl(sEvent, PrepareInput(data));
}

//----------------------------------------------------------------------------------------
//
QVariant CScriptEventSenderJs::sendEventAndWait(const QString& sEvent)
{
  return sendEventAndWait(sEvent, QVariant());
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

  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CEventSenderSignalEmitter>())
    {
      emit spSignalEmitter->showError(tr("Invalid data for event data."), QtMsgType::QtWarningMsg);
    }
  }
  return QString();
}

//----------------------------------------------------------------------------------------
//
CScriptEventSenderLua::CScriptEventSenderLua(std::weak_ptr<CScriptCommunicator> pCommunicator,
                                             QtLua::State* pState) :
  CScriptEventSenderBase(pCommunicator, pState)
{
}
CScriptEventSenderLua::~CScriptEventSenderLua() = default;

//----------------------------------------------------------------------------------------
//
void CScriptEventSenderLua::sendEvent(const QString& sEvent)
{
  sendEvent(sEvent, QVariant());
}

//----------------------------------------------------------------------------------------
//
void CScriptEventSenderLua::sendEvent(const QString& sEvent, QVariant data)
{
  if (!CheckIfScriptCanRun()) { return; }
  SendEventImpl(sEvent, PrepareInput(data));
}

//----------------------------------------------------------------------------------------
//
QVariant CScriptEventSenderLua::sendEventAndWait(const QString& sEvent)
{
  return sendEventAndWait(sEvent, QVariant());
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

  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CEventSenderSignalEmitter>())
    {
      emit spSignalEmitter->showError(tr("Invalid data for event data."), QtMsgType::QtWarningMsg);
    }
  }
  return QString();
}
