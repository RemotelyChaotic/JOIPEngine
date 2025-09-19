#include "DebugInterface.h"

#include <QDebug>
#include <QQmlEngine>

CDebugInterface::CDebugInterface(QObject* pParent) :
    QObject{pParent},
    m_spEngine(new QJSEngine(this))
{
}

CDebugInterface::~CDebugInterface()
{
  DeleteEngine();
}

//----------------------------------------------------------------------------------------
//
void CDebugInterface::ResetEngine()
{
  DeleteEngine();
  m_spEngine = new QJSEngine(this);
  for (const auto& [objectName, pObj] : m_registeredObjects)
  {
    QJSValue wrapped = m_spEngine->newQObject(pObj);
    m_spEngine->globalObject().setProperty(objectName, wrapped);
  }
}

//----------------------------------------------------------------------------------------
//
void CDebugInterface::Register(const QString& objectName, QObject* pObj)
{
  m_registeredObjects.insert({objectName, pObj});
  if (nullptr != m_spEngine)
  {
    QJSValue wrapped = m_spEngine->newQObject(pObj);
    QQmlEngine::setObjectOwnership(pObj, QQmlEngine::CppOwnership);
    m_spEngine->globalObject().setProperty(objectName, wrapped);
  }
}

//----------------------------------------------------------------------------------------
//
QString CDebugInterface::TryEval(const QString& sCode)
{
  if (nullptr != m_spEngine)
  {
    QJSValue val = m_spEngine->evaluate(sCode);
    if (val.isError())
    {
      QString sException = val.property("name").toString();
      qint32 iLineNr = val.property("lineNumber").toInt() - 1;
      QString sStack = val.property("stack").toString();
      Q_UNUSED(sStack)
      QString sError = sException +
                       " at line " + QString::number(iLineNr) +
                       ": " + val.toString();
      return sError;
    }

    return val.toString();
  }

  qWarning() << tr("Debug engine instance was null.");
  return QString("undefined");
}

//----------------------------------------------------------------------------------------
//
void CDebugInterface::UnRegister(const QString& objectName)
{
  auto it = m_registeredObjects.find(objectName);
  if (m_registeredObjects.end() != it)
  {
    m_registeredObjects.erase(it);
    if (nullptr != m_spEngine)
    {
      m_spEngine->globalObject().setProperty(objectName, QJSValue(QJSValue::UndefinedValue));
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CDebugInterface::DeleteEngine()
{
  if (nullptr != m_spEngine)
  {
    m_spEngine->collectGarbage();

    // reset objects
    for (const auto& [objectName, _] : m_registeredObjects)
    {
      m_spEngine->globalObject().setProperty(objectName, QJSValue(QJSValue::UndefinedValue));
    }

    m_spEngine->setInterrupted(true);
    delete m_spEngine;
  }
}
