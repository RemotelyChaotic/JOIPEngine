#include "ScriptIcon.h"
#include "Application.h"
#include "CommonScriptHelpers.h"
#include "ScriptDbWrappers.h"
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
std::shared_ptr<CScriptObjectBase> CIconSignalEmitter::CreateNewScriptObject(QtLua::State* pState)
{
  return std::make_shared<CScriptIcon>(this, pState);
}

//----------------------------------------------------------------------------------------
//
CScriptIcon::CScriptIcon(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                         QPointer<QJSEngine> pEngine) :
  CJsScriptObjectBase(pEmitter, pEngine),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>())
{
}
CScriptIcon::CScriptIcon(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                         QtLua::State* pState) :
  CJsScriptObjectBase(pEmitter, pState),
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
void CScriptIcon::hide(QVariant resource)
{
  if (!CheckIfScriptCanRun()) { return; }

  auto spSignalEmitter = SignalEmitter<CIconSignalEmitter>();
  if (nullptr != spSignalEmitter)
  {
    QString sError;
    std::optional<QString> optRes =
        script::ParseResourceFromScriptVariant(resource, m_wpDbManager.lock(),
                                               m_spProject,
                                               "hide", &sError);
    if (optRes.has_value())
    {
      QString resRet = optRes.value();
      if (!resRet.isEmpty() && resRet != "~all")
      {
        emit spSignalEmitter->hideIcon(resRet);
      }
      else
      {
        emit spSignalEmitter->hideIcon(QString());
      }
    }
    else
    {
      if (resource.type() == QVariant::String ||
          resource.type() == QVariant::ByteArray)
      {
        QString resRet = resource.toString();
        if ("~all" == resRet)
        {
          emit spSignalEmitter->hideIcon(QString());
          return;
        }
      }
      emit m_pSignalEmitter->showError(sError, QtMsgType::QtWarningMsg);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptIcon::show(QVariant resource)
{
  if (!CheckIfScriptCanRun()) { return; }

  auto spSignalEmitter = SignalEmitter<CIconSignalEmitter>();
  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spSignalEmitter)
  {
    QString sError;
    std::optional<QString> optRes =
        script::ParseResourceFromScriptVariant(resource, m_wpDbManager.lock(),
                                               m_spProject,
                                               "show", &sError);
    if (optRes.has_value())
    {
      QString resRet = optRes.value();
      emit spSignalEmitter->showIcon(resRet);
    }
    else
    {
      emit m_pSignalEmitter->showError(sError, QtMsgType::QtWarningMsg);
    }
  }
}
