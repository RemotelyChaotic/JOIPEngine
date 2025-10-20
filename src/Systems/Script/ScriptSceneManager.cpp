#include "ScriptSceneManager.h"
#include "Application.h"
#include "CommonScriptHelpers.h"
#include "ScriptDbWrappers.h"

#include "Systems/DatabaseManager.h"

#include "Systems/EOS/CommandEosDisableSceneBase.h"
#include "Systems/EOS/CommandEosEnableSceneBase.h"
#include "Systems/EOS/EosCommands.h"

#include "Systems/JSON/JsonInstructionBase.h"
#include "Systems/JSON/JsonInstructionSetParser.h"

#include "Systems/Project.h"
#include "Systems/Scene.h"

CSceneManagerSignalEmiter::CSceneManagerSignalEmiter() :
  CScriptRunnerSignalEmiter()
{
}
CSceneManagerSignalEmiter::~CSceneManagerSignalEmiter()
{
}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<CScriptCommunicator>
CSceneManagerSignalEmiter::CreateCommunicatorImpl(std::shared_ptr<CScriptRunnerSignalEmiterAccessor> spAccessor)
{
  return std::make_shared<CSceneManagerScriptCommunicator>(spAccessor);
}

//----------------------------------------------------------------------------------------
//
CSceneManagerScriptCommunicator::CSceneManagerScriptCommunicator(
  const std::weak_ptr<CScriptRunnerSignalEmiterAccessor>& spEmitter) :
  CScriptCommunicator(spEmitter)
{}
CSceneManagerScriptCommunicator::~CSceneManagerScriptCommunicator() = default;

//----------------------------------------------------------------------------------------
//
CScriptObjectBase* CSceneManagerScriptCommunicator::CreateNewScriptObject(QPointer<QJSEngine> pEngine)
{
  return new CScriptSceneManager(weak_from_this(), pEngine);
}
CScriptObjectBase* CSceneManagerScriptCommunicator::CreateNewScriptObject(QPointer<CJsonInstructionSetParser> pParser)
{
  return new CEosScriptSceneManager(weak_from_this(), pParser);
}
CScriptObjectBase* CSceneManagerScriptCommunicator::CreateNewScriptObject(QtLua::State* pState)
{
  return new CScriptSceneManager(weak_from_this(), pState);
}
CScriptObjectBase* CSceneManagerScriptCommunicator::CreateNewSequenceObject()
{
  return nullptr;
}

//----------------------------------------------------------------------------------------
//
CScriptSceneManager::CScriptSceneManager(std::weak_ptr<CScriptCommunicator> pCommunicator,
                                         QPointer<QJSEngine> pEngine) :
  CJsScriptObjectBase(pCommunicator, pEngine),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>())
{
  m_spStop = std::make_shared<std::function<void()>>([this]() {
    emit SignalQuitLoop();
  });
  if (auto spComm = pCommunicator.lock())
  {
    spComm->RegisterStopCallback(m_spStop);
  }
}
CScriptSceneManager::CScriptSceneManager(std::weak_ptr<CScriptCommunicator> pCommunicator,
                                         QtLua::State* pState) :
  CJsScriptObjectBase(pCommunicator, pState),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>())
{
  m_spStop = std::make_shared<std::function<void()>>([this]() {
    emit SignalQuitLoop();
  });
  if (auto spComm = pCommunicator.lock())
  {
    spComm->RegisterStopCallback(m_spStop);
  }
}
CScriptSceneManager::~CScriptSceneManager()
{
  if (auto spComm = m_wpCommunicator.lock())
  {
    spComm->RemoveStopCallback(m_spStop);
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptSceneManager::disable(QVariant scene)
{
  if (!CheckIfScriptCanRun()) { return; }

  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CSceneManagerSignalEmiter>())
    {
      QString sScene = GetScene(scene, "disable");
      emit spSignalEmitter->disable(sScene);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptSceneManager::enable(QVariant scene)
{
  if (!CheckIfScriptCanRun()) { return; }

  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CSceneManagerSignalEmiter>())
    {
      QString sScene = GetScene(scene, "enable");
      emit spSignalEmitter->enable(sScene);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptSceneManager::gotoScene(QVariant scene)
{
  if (!CheckIfScriptCanRun()) { return; }

  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CSceneManagerSignalEmiter>())
    {
      QPointer<CScriptSceneManager> pThis(this);
      QString sScene = GetScene(scene, "gotoScene");

      // goto needs to wait here otherwise we accidentally run commands after the call
      QEventLoop loop;
      QMetaObject::Connection quitLoop =
        connect(this, &CScriptSceneManager::SignalQuitLoop, &loop, &QEventLoop::quit,
                Qt::QueuedConnection);
      QMetaObject::Connection interruptThisLoop =
          connect(this, &CScriptObjectBase::SignalInterruptExecution,
                  &loop, &QEventLoop::quit, Qt::QueuedConnection);
      emit spSignalEmitter->gotoScene(sScene);
      loop.exec();
      loop.disconnect();

      if (nullptr != pThis)
      {
        disconnect(quitLoop);
        disconnect(interruptThisLoop);
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
QString CScriptSceneManager::GetScene(const QVariant& scene, const QString& sSource)
{
  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CSceneManagerSignalEmiter>())
    {
      QString sError;
      std::optional<QString> optRes =
          script::ParseSceneFromScriptVariant(scene, m_wpDbManager.lock(),
                                              m_spProject,
                                              sSource, &sError);
      if (optRes.has_value())
      {
        return optRes.value();
      }
      else
      {
        emit spSignalEmitter->showError(sError, QtMsgType::QtWarningMsg);
      }
    }
  }

  return QString();
}

//----------------------------------------------------------------------------------------
//
class CCommandEosDisableScene : public CCommandEosEnableSceneBase
{
public:
  CCommandEosDisableScene(CEosScriptSceneManager* pParent) :
    CCommandEosEnableSceneBase(),
    m_pParent(pParent)
  {}
  ~CCommandEosDisableScene() override {}

  IJsonInstructionBase::tRetVal Call(const tInstructionMapValue& args) override
  {
    if (nullptr != m_pParent)
    {
      const auto& itTarget = GetValue<EArgumentType::eString>(args, "target");
      if (HasValue(args, "target") && IsOk<EArgumentType::eString>(itTarget))
      {
        QString sTarget = std::get<QString>(itTarget);
        m_pParent->Disable(sTarget);
        return SRunRetVal<ENextCommandToCall::eSibling>();
      }
      return SJsonException{"Do id for notification.", "", eos::c_sCommandEnableScreen, 0, 0};
    }
    return SJsonException{"Internal error.", "", eos::c_sCommandEnableScreen, 0, 0};
  }

private:
  CEosScriptSceneManager*       m_pParent;

};

//----------------------------------------------------------------------------------------
//
class CCommandEosEnableScene : public CCommandEosDisableSceneBase
{
public:
  CCommandEosEnableScene(CEosScriptSceneManager* pParent) :
    CCommandEosDisableSceneBase(),
    m_pParent(pParent)
  {}
  ~CCommandEosEnableScene() override {}

  IJsonInstructionBase::tRetVal Call(const tInstructionMapValue& args) override
  {
    if (nullptr != m_pParent)
    {
      const auto& itTarget = GetValue<EArgumentType::eString>(args, "target");
      if (HasValue(args, "target") && IsOk<EArgumentType::eString>(itTarget))
      {
        QString sTarget = std::get<QString>(itTarget);
        m_pParent->Enable(sTarget);
        return SRunRetVal<ENextCommandToCall::eSibling>();
      }
      return SJsonException{"Do id for notification.", "", eos::c_sCommandDisableScreen, 0, 0};
    }
    return SJsonException{"Internal error.", "", eos::c_sCommandDisableScreen, 0, 0};
  }

private:
  CEosScriptSceneManager*       m_pParent;
};

//----------------------------------------------------------------------------------------
//
CEosScriptSceneManager::CEosScriptSceneManager(std::weak_ptr<CScriptCommunicator> pCommunicator,
                                               QPointer<CJsonInstructionSetParser> pParser) :
  CEosScriptObjectBase(pCommunicator, pParser),
  m_spCommandDisable(std::make_shared<CCommandEosDisableScene>(this)),
  m_spCommandEnable(std::make_shared<CCommandEosEnableScene>(this))
{
  m_spStop = std::make_shared<std::function<void()>>([this]() {
    emit SignalQuitLoop();
  });
  if (auto spComm = pCommunicator.lock())
  {
    spComm->RegisterStopCallback(m_spStop);
  }
  pParser->RegisterInstruction(eos::c_sCommandDisableScreen, m_spCommandDisable);
  pParser->RegisterInstruction(eos::c_sCommandEnableScreen, m_spCommandEnable);
}
CEosScriptSceneManager::~CEosScriptSceneManager()
{
  if (auto spComm = m_wpCommunicator.lock())
  {
    spComm->RemoveStopCallback(m_spStop);
  }
}

//----------------------------------------------------------------------------------------
//
void CEosScriptSceneManager::Disable(const QString& sScene)
{
  if (!CheckIfScriptCanRun()) { return; }

  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CSceneManagerSignalEmiter>())
    {
      emit spSignalEmitter->disable(sScene);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CEosScriptSceneManager::Enable(const QString& sScene)
{
  if (!CheckIfScriptCanRun()) { return; }

  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CSceneManagerSignalEmiter>())
    {
      emit spSignalEmitter->enable(sScene);
    }
  }
}
