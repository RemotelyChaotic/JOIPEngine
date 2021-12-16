#include "ProjectEventTarget.h"

#include <QDebug>

//----------------------------------------------------------------------------------------
//
CProjectEventWrapper::CProjectEventWrapper(const QString& sType) :
  QObject(nullptr),
  m_sType(sType)
{
}

//----------------------------------------------------------------------------------------
//
CProjectEventCallbackRegistry::CProjectEventCallbackRegistry(QObject* pParent) :
  QObject(pParent),
  m_callbackMap(),
  m_dispatchTargetMap()
{

}
CProjectEventCallbackRegistry::~CProjectEventCallbackRegistry()
{

}

//----------------------------------------------------------------------------------------
//
void CProjectEventCallbackRegistry::AddDispatchTarget(const QString& sType,
                                                      QPointer<CProjectEventTargetWrapper> pWrapper)
{
  auto it = m_dispatchTargetMap.find(sType);
  if (m_dispatchTargetMap.end() == it)
  {
    m_dispatchTargetMap.insert({sType, pWrapper});
  }
}

//----------------------------------------------------------------------------------------
//
void CProjectEventCallbackRegistry::AddEventListener(QString sType, QJSValue callback)
{
  auto it = m_callbackMap.find(sType);
  if (m_callbackMap.end() == it)
  {
    m_callbackMap.insert({sType, {}});
    it = m_callbackMap.find(sType);
  }

  auto itCallbackMap = std::find_if(it->second.begin(), it->second.end(),
                                    [&callback](const QJSValue& val) {
    return val.equals(callback);
  });
  if (it->second.end() == itCallbackMap)
  {
    it->second.push_back(callback);
  }
}

//----------------------------------------------------------------------------------------
//
void CProjectEventCallbackRegistry::Clear()
{
  m_callbackMap.clear();
}

//----------------------------------------------------------------------------------------
//
void CProjectEventCallbackRegistry::HandleError(QJSValue& value)
{
  QString sException = value.property("name").toString();
  qint32 iLineNr = value.property("lineNumber").toInt() - 1;
  QString sStack = value.property("stack").toString();
  QString sError = "Uncaught " + sException +
                   " at line " + QString::number(iLineNr) +
                   ": " + value.toString() + "\n" + sStack;
  qWarning() << sError;
  emit SignalError(sError, QtMsgType::QtWarningMsg);
}

//----------------------------------------------------------------------------------------
//
void CProjectEventCallbackRegistry::RemoveDispatchTarget(const QString& sType)
{
  auto it = m_dispatchTargetMap.find(sType);
  if (m_dispatchTargetMap.end() != it)
  {
    m_dispatchTargetMap.erase(it);
  }
}


//----------------------------------------------------------------------------------------
//
void CProjectEventCallbackRegistry::RemoveEventListener(QString sType, QJSValue callback)
{
  auto it = m_callbackMap.find(sType);
  if (m_callbackMap.end() != it)
  {
    auto itCallbackMap = std::find_if(it->second.begin(), it->second.end(),
                                      [&callback](const QJSValue& val) {
      return val.equals(callback);
    });
    if (it->second.end() != itCallbackMap)
    {
      it->second.erase(itCallbackMap);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CProjectEventCallbackRegistry::Dispatch(const QString& sEvent)
{
  auto it = m_callbackMap.find(sEvent);
  if (m_callbackMap.end() != it)
  {
    for (auto& callback : it->second)
    {
      if (callback.isCallable())
      {
        QJSValue ret = callback.call();
        if (ret.isError())
        {
          HandleError(ret);
        }
      }
    }
  }

  auto itDispatchTarget = m_dispatchTargetMap.find(sEvent);
  if (m_dispatchTargetMap.end() != itDispatchTarget && nullptr != itDispatchTarget->second)
  {
    itDispatchTarget->second->Dispatched(sEvent);
  }
}

//----------------------------------------------------------------------------------------
//
CProjectEventTargetWrapper::CProjectEventTargetWrapper(QObject* pParent) :
  QObject(pParent),
  m_wpRegistry()
{
}
CProjectEventTargetWrapper::~CProjectEventTargetWrapper()
{

}

//----------------------------------------------------------------------------------------
//
void CProjectEventTargetWrapper::Dispatched(const QString&)
{
  // don't do anything
}

//----------------------------------------------------------------------------------------
//
void CProjectEventTargetWrapper::InitializeEventRegistry(
    std::weak_ptr<CProjectEventCallbackRegistry> wpRegistry)
{
  m_wpRegistry = wpRegistry;
}

//----------------------------------------------------------------------------------------
//
void CProjectEventTargetWrapper::addEventListener(QString sType, QJSValue callback)
{
  if (auto spRegistry = m_wpRegistry.lock())
  {
    spRegistry->AddEventListener(sType, callback);
  }
}

//----------------------------------------------------------------------------------------
//
void CProjectEventTargetWrapper::removeEventListener(QString sType, QJSValue callback)
{
  if (auto spRegistry = m_wpRegistry.lock())
  {
    spRegistry->RemoveEventListener(sType, callback);
  }
}

//----------------------------------------------------------------------------------------
//
void CProjectEventTargetWrapper::dispatch(QJSValue event)
{
  if (auto spRegistry = m_wpRegistry.lock())
  {
    if (event.isString())
    {
      spRegistry->Dispatch(event.toString());
    }
    else if (event.isObject())
    {
      CProjectEventWrapper* pEventWrapper =
          dynamic_cast<CProjectEventWrapper*>(event.toQObject());
      if (nullptr != pEventWrapper)
      {
        spRegistry->Dispatch(pEventWrapper->m_sType);
      }
    }
  }
}
