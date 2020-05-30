#include "ScriptIcon.h"
#include "Application.h"
#include "Systems/DatabaseManager.h"
#include "Systems/Project.h"


CIconSignalEmitter::CIconSignalEmitter() :
  CScriptRunnerSignalEmiter()
{

}
CIconSignalEmitter::~CIconSignalEmitter()
{

}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<CScriptObjectBase> CIconSignalEmitter::CreateNewScriptObject(QPointer<QJSEngine> pEngine)
{
  return std::make_shared<CScriptIcon>(this, pEngine);
}

//----------------------------------------------------------------------------------------
//
CScriptIcon::CScriptIcon(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                         QPointer<QJSEngine> pEngine) :
  CScriptObjectBase(pEmitter, pEngine),
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

  auto spSignalEmitter = SignalEmitter<CIconSignalEmitter>();
  if (resource.isString())
  {
    const QString sResource = resource.toString();
    if (!sResource.isEmpty())
    {
      if (sResource != "~all")
      {
        emit spSignalEmitter->hideIcon(sResource);
      }
      else
      {
        emit spSignalEmitter->hideIcon(QString());
      }
    }
    else
    {
      emit spSignalEmitter->hideIcon(QString());
    }
  }
  else
  {
    QString sError = tr("Argument to hide() needs to be a string ('~all' or '' to hide all).");
    emit m_pSignalEmitter->showError(sError.arg(resource.toString()),
                                      QtMsgType::QtWarningMsg);
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptIcon::show(QJSValue resource)
{
  if (!CheckIfScriptCanRun()) { return; }

  auto spSignalEmitter = SignalEmitter<CIconSignalEmitter>();
  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    if (resource.isString())
    {
      QString sResourceName = resource.toString();
      tspResource spResource = spDbManager->FindResourceInProject(m_spProject, sResourceName);
      if (nullptr != spResource)
      {
        emit spSignalEmitter->showIcon(sResourceName);
      }
      else
      {
        QString sError = tr("Resource %1 not found");
        emit m_pSignalEmitter->showError(sError.arg(resource.toString()),
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
          emit spSignalEmitter->showIcon(pResource->getName());
        }
        else
        {
          QString sError = tr("Resource in show() holds no data.");
          emit m_pSignalEmitter->showError(sError, QtMsgType::QtWarningMsg);
        }
      }
      else
      {
        QString sError = tr("Wrong argument-type to show(). String or resource was expected.");
        emit m_pSignalEmitter->showError(sError, QtMsgType::QtWarningMsg);
      }
    }
    else
    {
      QString sError = tr("Wrong argument-type to show(). String or resource was expected.");
      emit m_pSignalEmitter->showError(sError, QtMsgType::QtWarningMsg);
    }
  }
}
