#include "DebugInterface.h"

#include <QDebug>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>

CDebugInterface::CDebugInterface(QObject* pParent) :
    QObject{pParent},
    m_spOwnEngine(new QJSEngine(this))
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
  m_spOwnEngine = new QJSEngine(this);
  for (const auto& [objectName, pair] : m_registeredObjects)
  {
    if (pair.first == m_spOwnEngine)
    {
      QJSValue wrapped = m_spOwnEngine->newQObject(pair.second);
      QQmlEngine::setObjectOwnership(pair.first, QQmlEngine::CppOwnership);
      m_spOwnEngine->globalObject().setProperty(objectName, wrapped);
    }
    else if (nullptr != pair.first)
    {
      QJSValue wrapped = pair.first->newQObject(pair.second);
      QQmlEngine::setObjectOwnership(pair.second, QQmlEngine::CppOwnership);
      pair.first->globalObject().setProperty(objectName, wrapped);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CDebugInterface::Register(const QString& objectName, QObject* pObj)
{
  QPointer<QJSEngine> e = m_spOwnEngine;
  if (auto pForeignItem = dynamic_cast<QQuickItem*>(pObj))
  {
    e = QQmlEngine::contextForObject(pForeignItem)->engine();
    m_registeredObjects.insert({objectName, {e, pObj}});
  }
  else
  {
    m_registeredObjects.insert({objectName, {e, pObj}});
  }
  if (nullptr != m_spOwnEngine && e == m_spOwnEngine)
  {
    QJSValue wrapped = m_spOwnEngine->newQObject(pObj);
    QQmlEngine::setObjectOwnership(pObj, QQmlEngine::CppOwnership);
    m_spOwnEngine->globalObject().setProperty(objectName, wrapped);
  }
  else if (nullptr != e)
  {
    QJSValue wrapped = e->newQObject(pObj);
    QQmlEngine::setObjectOwnership(pObj, QQmlEngine::CppOwnership);
    e->globalObject().setProperty(objectName, wrapped);
  }
}

//----------------------------------------------------------------------------------------
//
bool TrySingleEngine(QPointer<QJSEngine> pEngineToTry,
                     const QString& sCode,
                     QString* psOutput,
                     std::vector<QPointer<QJSEngine>>* pvpTriedEngines)
{
  auto it = std::find(pvpTriedEngines->begin(), pvpTriedEngines->end(), pEngineToTry);
  if (pvpTriedEngines->end() == it)
  {
    pvpTriedEngines->push_back(pEngineToTry);

    QJSValue val = pEngineToTry->evaluate(sCode);
    if (val.isError())
    {
      QString sException = val.property("name").toString();
      qint32 iLineNr = val.property("lineNumber").toInt() - 1;
      QString sStack = val.property("stack").toString();
      Q_UNUSED(sStack)
      QString sError = sException +
                       " at line " + QString::number(iLineNr) +
                       ": " + val.toString();
      *psOutput = sError;
      return false;
    }
    *psOutput = val.toString();
    return true;
  }
  return false;
}

//----------------------------------------------------------------------------------------
//
QString CDebugInterface::TryEval(const QString& sCode)
{
  std::vector<QPointer<QJSEngine>> vpTriedEngines;
  QString sOut;
  if (nullptr != m_spOwnEngine)
  {
    bool bOk = TrySingleEngine(m_spOwnEngine, sCode, &sOut, &vpTriedEngines);
    if (!bOk)
    {
      for (const auto& [objectName, pair] : m_registeredObjects)
      {
        if (pair.first != m_spOwnEngine)
        {
          bOk = TrySingleEngine(pair.first, sCode, &sOut, &vpTriedEngines);
          if (bOk) { break; }
        }
      }
    }
    return sOut;
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
    QJSEngine* pEngine = it->second.first;
    m_registeredObjects.erase(it);
    if (nullptr != m_spOwnEngine && m_spOwnEngine == pEngine)
    {
      m_spOwnEngine->globalObject().setProperty(objectName, QJSValue(QJSValue::UndefinedValue));
    }
    else if (nullptr != pEngine)
    {
      pEngine->globalObject().setProperty(objectName, QJSValue(QJSValue::UndefinedValue));
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CDebugInterface::DeleteEngine()
{
  if (nullptr != m_spOwnEngine)
  {
    m_spOwnEngine->collectGarbage();

    // reset objects
    for (const auto& [objectName, pair] : m_registeredObjects)
    {
      if (m_spOwnEngine == pair.first)
      {
        m_spOwnEngine->globalObject().setProperty(objectName, QJSValue(QJSValue::UndefinedValue));
      }
      else if (nullptr != pair.first)
      {
        pair.first->globalObject().setProperty(objectName, QJSValue(QJSValue::UndefinedValue));
      }
    }

    m_spOwnEngine->setInterrupted(true);
    delete m_spOwnEngine;
  }
}
