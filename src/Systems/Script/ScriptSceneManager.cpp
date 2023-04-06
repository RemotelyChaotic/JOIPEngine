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
std::shared_ptr<CScriptObjectBase> CSceneManagerSignalEmiter::CreateNewScriptObject(QPointer<QJSEngine> pEngine)
{
  return std::make_shared<CScriptSceneManager>(this, pEngine);
}
std::shared_ptr<CScriptObjectBase> CSceneManagerSignalEmiter::CreateNewScriptObject(QPointer<CJsonInstructionSetParser> pParser)
{
  return std::make_shared<CEosScriptSceneManager>(this, pParser);
}
std::shared_ptr<CScriptObjectBase> CSceneManagerSignalEmiter::CreateNewScriptObject(QtLua::State* pState)
{
  return std::make_shared<CScriptSceneManager>(this, pState);
}

//----------------------------------------------------------------------------------------
//
CScriptSceneManager::CScriptSceneManager(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                                         QPointer<QJSEngine> pEngine) :
  CJsScriptObjectBase(pEmitter, pEngine),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>())
{
}
CScriptSceneManager::CScriptSceneManager(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                                         QtLua::State* pState) :
  CJsScriptObjectBase(pEmitter, pState),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>())
{
}
CScriptSceneManager::~CScriptSceneManager()
{

}

//----------------------------------------------------------------------------------------
//
void CScriptSceneManager::disable(QVariant scene)
{
  if (!CheckIfScriptCanRun()) { return; }
  QString sScene = GetScene(scene, "disable");
  emit SignalEmitter<CSceneManagerSignalEmiter>()->disable(sScene);
}

//----------------------------------------------------------------------------------------
//
void CScriptSceneManager::enable(QVariant scene)
{
  if (!CheckIfScriptCanRun()) { return; }
  QString sScene = GetScene(scene, "enable");
  emit SignalEmitter<CSceneManagerSignalEmiter>()->enable(sScene);
}

//----------------------------------------------------------------------------------------
//
void CScriptSceneManager::gotoScene(QVariant scene)
{
  if (!CheckIfScriptCanRun()) { return; }
  QString sScene = GetScene(scene, "gotoScene");

  auto pSignalEmitter = SignalEmitter<CSceneManagerSignalEmiter>();
  emit pSignalEmitter->gotoScene(sScene);
}

//----------------------------------------------------------------------------------------
//
QString CScriptSceneManager::GetScene(const QVariant& scene, const QString& sSource)
{
  auto spSignalEmitter = SignalEmitter<CSceneManagerSignalEmiter>();
  if (nullptr != spSignalEmitter)
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
      emit m_pSignalEmitter->showError(sError, QtMsgType::QtWarningMsg);
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
CEosScriptSceneManager::CEosScriptSceneManager(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                                               QPointer<CJsonInstructionSetParser> pParser) :
  CEosScriptObjectBase(pEmitter, pParser),
  m_spCommandDisable(std::make_shared<CCommandEosDisableScene>(this)),
  m_spCommandEnable(std::make_shared<CCommandEosEnableScene>(this))
{
  pParser->RegisterInstruction(eos::c_sCommandDisableScreen, m_spCommandDisable);
  pParser->RegisterInstruction(eos::c_sCommandEnableScreen, m_spCommandEnable);
}
CEosScriptSceneManager::~CEosScriptSceneManager()
{

}

//----------------------------------------------------------------------------------------
//
void CEosScriptSceneManager::Disable(const QString& sScene)
{
  if (!CheckIfScriptCanRun()) { return; }
  emit SignalEmitter<CSceneManagerSignalEmiter>()->disable(sScene);
}

//----------------------------------------------------------------------------------------
//
void CEosScriptSceneManager::Enable(const QString& sScene)
{
  if (!CheckIfScriptCanRun()) { return; }
  emit SignalEmitter<CSceneManagerSignalEmiter>()->enable(sScene);
}
