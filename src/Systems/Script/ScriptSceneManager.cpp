#include "ScriptSceneManager.h"
#include "Application.h"

#include "Systems/DatabaseManager.h"
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

//----------------------------------------------------------------------------------------
//
CScriptSceneManager::CScriptSceneManager(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                                         QPointer<QJSEngine> pEngine) :
  CJsScriptObjectBase(pEmitter, pEngine),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>())
{

}
CScriptSceneManager::~CScriptSceneManager()
{

}

//----------------------------------------------------------------------------------------
//
void CScriptSceneManager::disable(QJSValue scene)
{
  if (!CheckIfScriptCanRun()) { return; }
  QString sScene = GetScene(scene, "disable()");
  emit SignalEmitter<CSceneManagerSignalEmiter>()->disable(sScene);
}

//----------------------------------------------------------------------------------------
//
void CScriptSceneManager::enable(QJSValue scene)
{
  if (!CheckIfScriptCanRun()) { return; }
  QString sScene = GetScene(scene, "enable()");
  emit SignalEmitter<CSceneManagerSignalEmiter>()->enable(sScene);
}

//----------------------------------------------------------------------------------------
//
void CScriptSceneManager::gotoScene(QJSValue scene)
{
  if (!CheckIfScriptCanRun()) { return; }
  QString sScene = GetScene(scene, "gotoScene()");

  auto pSignalEmitter = SignalEmitter<CSceneManagerSignalEmiter>();
  emit pSignalEmitter->gotoScene(sScene);
}

//----------------------------------------------------------------------------------------
//
QString CScriptSceneManager::GetScene(const QJSValue& scene, const QString& sSource)
{
  QString sRet;
  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    if (scene.isString())
    {
      QString sSceneName = scene.toString();
      tspScene spScene = spDbManager->FindScene(m_spProject, sSceneName);
      if (nullptr != spScene)
      {
        sRet = sSceneName;
      }
      else
      {
        QString sError = tr("Resource %1 not found");
        emit m_pSignalEmitter->showError(sError.arg(scene.toString()),
                                                QtMsgType::QtWarningMsg);
      }
    }
    else if (scene.isQObject())
    {
      CSceneScriptWrapper* pScene = dynamic_cast<CSceneScriptWrapper*>(scene.toQObject());
      if (nullptr != pScene)
      {
        tspScene spScene = pScene->Data();
        if (nullptr != spScene)
        {
          sRet = pScene->getName();
        }
        else
        {
          QString sError = tr("Resource in %1 holds no data.");
          emit m_pSignalEmitter->showError(sError.arg(sSource), QtMsgType::QtWarningMsg);
        }
      }
      else
      {
        QString sError = tr("Wrong argument-type to %1. String or scene was expected.");
        emit m_pSignalEmitter->showError(sError.arg(sSource), QtMsgType::QtWarningMsg);
      }
    }
    else
    {
      QString sError = tr("Wrong argument-type to %1. String or scene was expected.");
      emit m_pSignalEmitter->showError(sError.arg(sSource), QtMsgType::QtWarningMsg);
    }
  }
  return sRet;
}

//----------------------------------------------------------------------------------------
//
class CCommandEosDisableScene : public IJsonInstructionBase
{
public:
  CCommandEosDisableScene(CEosScriptSceneManager* pParent) :
    m_pParent(pParent),
    m_argTypes({
      {"target", SInstructionArgumentType{EArgumentType::eString}}
    }) {}
  ~CCommandEosDisableScene() override {}

  tInstructionMapType& ArgList() override
  {
    return m_argTypes;
  }

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
      return SJsonException{"Do id for notification.", "", "notification.remove", 0, 0};
    }
    return SJsonException{"Internal error.", "", "notification.remove", 0, 0};
  }

private:
  CEosScriptSceneManager*       m_pParent;
  tInstructionMapType           m_argTypes;
};

//----------------------------------------------------------------------------------------
//
class CCommandEosEnableScene : public IJsonInstructionBase
{
public:
  CCommandEosEnableScene(CEosScriptSceneManager* pParent) :
    m_pParent(pParent),
    m_argTypes({
      {"target", SInstructionArgumentType{EArgumentType::eString}}
    }) {}
  ~CCommandEosEnableScene() override {}

  tInstructionMapType& ArgList() override
  {
    return m_argTypes;
  }

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
      return SJsonException{"Do id for notification.", "", "notification.remove", 0, 0};
    }
    return SJsonException{"Internal error.", "", "notification.remove", 0, 0};
  }

private:
  CEosScriptSceneManager*       m_pParent;
  tInstructionMapType           m_argTypes;
};

//----------------------------------------------------------------------------------------
//
CEosScriptSceneManager::CEosScriptSceneManager(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                                               QPointer<CJsonInstructionSetParser> pParser) :
  CEosScriptObjectBase(pEmitter, pParser),
  m_spCommandDisable(std::make_shared<CCommandEosDisableScene>(this)),
  m_spCommandEnable(std::make_shared<CCommandEosEnableScene>(this))
{
  pParser->RegisterInstruction("disable", m_spCommandDisable);
  pParser->RegisterInstruction("enable", m_spCommandEnable);
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
