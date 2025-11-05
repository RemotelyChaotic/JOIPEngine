#include "ScriptIcon.h"
#include "Application.h"
#include "CommonScriptHelpers.h"
#include "ScriptDbWrappers.h"
#include "Systems/DatabaseManager.h"
#include "Systems/Database/Project.h"


CIconSignalEmitter::CIconSignalEmitter() :
  CScriptRunnerSignalEmiter()
{

}
CIconSignalEmitter::~CIconSignalEmitter()
{

}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<CScriptCommunicator>
CIconSignalEmitter::CreateCommunicatorImpl(std::shared_ptr<CScriptRunnerSignalEmiterAccessor> spAccessor)
{
  return std::make_shared<CIconScriptCommunicator>(spAccessor);
}

//----------------------------------------------------------------------------------------
//
CIconScriptCommunicator::CIconScriptCommunicator(
  const std::weak_ptr<CScriptRunnerSignalEmiterAccessor>& spEmitter) :
  CScriptCommunicator(spEmitter)
{}
CIconScriptCommunicator::~CIconScriptCommunicator() = default;

//----------------------------------------------------------------------------------------
//
CScriptObjectBase* CIconScriptCommunicator::CreateNewScriptObject(QPointer<QJSEngine> pEngine)
{
  return new CScriptIcon(weak_from_this(), pEngine);
}
CScriptObjectBase* CIconScriptCommunicator::CreateNewScriptObject(QPointer<CJsonInstructionSetParser> pParser)
{
  Q_UNUSED(pParser)
  return nullptr;
}
CScriptObjectBase* CIconScriptCommunicator::CreateNewScriptObject(QtLua::State* pState)
{
  return new CScriptIcon(weak_from_this(), pState);
}
CScriptObjectBase* CIconScriptCommunicator::CreateNewSequenceObject()
{
  return nullptr;
}

//----------------------------------------------------------------------------------------
//
CScriptIcon::CScriptIcon(std::weak_ptr<CScriptCommunicator> pCommunicator,
                         QPointer<QJSEngine> pEngine) :
  CJsScriptObjectBase(pCommunicator, pEngine),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>())
{
}
CScriptIcon::CScriptIcon(std::weak_ptr<CScriptCommunicator> pCommunicator,
                         QtLua::State* pState) :
  CJsScriptObjectBase(pCommunicator, pState),
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

  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CIconSignalEmitter>())
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
        emit spSignalEmitter->showError(sError, QtMsgType::QtWarningMsg);
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptIcon::show(QVariant resource)
{
  if (!CheckIfScriptCanRun()) { return; }

  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CIconSignalEmitter>())
    {
      auto spDbManager = m_wpDbManager.lock();
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
        emit spSignalEmitter->showError(sError, QtMsgType::QtWarningMsg);
      }
    }
  }
}
