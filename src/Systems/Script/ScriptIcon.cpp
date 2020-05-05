#include "ScriptIcon.h"
#include "Application.h"
#include "ScriptRunnerSignalEmiter.h"
#include "Systems/DatabaseManager.h"
#include "Systems/Project.h"

CScriptIcon::CScriptIcon(std::shared_ptr<CScriptRunnerSignalEmiter> spEmitter,
                         QPointer<QJSEngine> pEngine) :
  CScriptObjectBase(spEmitter, pEngine),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>())
{

}

CScriptIcon::~CScriptIcon()
{

}

//----------------------------------------------------------------------------------------
//
void CScriptIcon::hide()
{
  hide("");
}

//----------------------------------------------------------------------------------------
//
void CScriptIcon::hide(QJSValue resource)
{
  if (!CheckIfScriptCanRun()) { return; }

  if (resource.isString())
  {
    const QString sResource = resource.toString();
    if (!sResource.isEmpty())
    {
      if (sResource != "~all")
      {
        emit m_spSignalEmitter->SignalHideIcon(sResource);
      }
      else
      {
        emit m_spSignalEmitter->SignalHideIcon(QString());
      }
    }
    else
    {
      emit m_spSignalEmitter->SignalHideIcon(QString());
    }
  }
  else
  {
    QString sError = tr("Argument to hide() needs to be a string ('~all' or '' to hide all).");
    emit m_spSignalEmitter->SignalShowError(sError.arg(resource.toString()),
                                            QtMsgType::QtWarningMsg);
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptIcon::show(QJSValue resource)
{
  if (!CheckIfScriptCanRun()) { return; }

  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    if (resource.isString())
    {
      tspResource spResource = spDbManager->FindResource(m_spProject, resource.toString());
      if (nullptr != spResource)
      {
        emit m_spSignalEmitter->SignalShowIcon(spResource);
      }
      else
      {
        QString sError = tr("Resource %1 not found");
        emit m_spSignalEmitter->SignalShowError(sError.arg(resource.toString()),
                                                QtMsgType::QtWarningMsg);
      }
    }
    else if (resource.isQObject())
    {
      CResource* pResource = dynamic_cast<CResource*>(resource.toQObject());
      if (nullptr != pResource)
      {
        tspResource spResource = pResource->Data();
        if (nullptr != spResource)
        {
          emit m_spSignalEmitter->SignalShowIcon(spResource);
        }
        else
        {
          QString sError = tr("Resource in show() holds no data.");
          emit m_spSignalEmitter->SignalShowError(sError, QtMsgType::QtWarningMsg);
        }
      }
      else
      {
        QString sError = tr("Wrong argument-type to show(). String or resource was expected.");
        emit m_spSignalEmitter->SignalShowError(sError, QtMsgType::QtWarningMsg);
      }
    }
    else
    {
      QString sError = tr("Wrong argument-type to show(). String or resource was expected.");
      emit m_spSignalEmitter->SignalShowError(sError, QtMsgType::QtWarningMsg);
    }
  }
}
