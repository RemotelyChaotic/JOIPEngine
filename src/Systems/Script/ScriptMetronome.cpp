#include "ScriptMetronome.h"
#include "Application.h"
#include "CommonScriptHelpers.h"
#include "ScriptDbWrappers.h"

#include "Systems/DatabaseManager.h"

#include "Systems/Sequence/SequenceMetronomeRunner.h"

CMetronomeSignalEmitter::CMetronomeSignalEmitter() :
  CScriptRunnerSignalEmiter()
{

}
CMetronomeSignalEmitter::~CMetronomeSignalEmitter()
{

}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<CScriptObjectBase> CMetronomeSignalEmitter::CreateNewScriptObject(QPointer<QJSEngine> pEngine)
{
  return std::make_shared<CScriptMetronome>(this, pEngine);
}
std::shared_ptr<CScriptObjectBase> CMetronomeSignalEmitter::CreateNewScriptObject(QtLua::State* pState)
{
  return std::make_shared<CScriptMetronome>(this, pState);
}
std::shared_ptr<CScriptObjectBase> CMetronomeSignalEmitter::CreateNewSequenceObject()
{
  return std::make_shared<CSequenceMetronomeRunner>(this);
}

//----------------------------------------------------------------------------------------
//
CScriptMetronome::CScriptMetronome(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                                   QPointer<QJSEngine> pEngine) :
  CJsScriptObjectBase(pEmitter, pEngine),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>())
{
}
CScriptMetronome::CScriptMetronome(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                                   QtLua::State* pState) :
  CJsScriptObjectBase(pEmitter, pState),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>())
{
}

CScriptMetronome::~CScriptMetronome()
{
}

//----------------------------------------------------------------------------------------
//
void CScriptMetronome::setBpm(qint32 iBpm)
{
  if (!CheckIfScriptCanRun()) { return; }
  emit SignalEmitter<CMetronomeSignalEmitter>()->setBpm(iBpm);
}

//----------------------------------------------------------------------------------------
//
void CScriptMetronome::setBeatResource(QVariant resource)
{
  if (!CheckIfScriptCanRun()) { return; }

  auto pSignalEmitter = SignalEmitter<CMetronomeSignalEmitter>();
  if (nullptr != pSignalEmitter)
  {
    // the input can either be a string, a bytearray, null, a resource...
    CResourceScriptWrapper* pItemWrapper = dynamic_cast<CResourceScriptWrapper*>(resource.value<QObject*>());
    if (resource.type() == QVariant::String || resource.type() == QVariant::ByteArray ||
        resource.isNull() || nullptr != pItemWrapper)
    {
      QString sError;
      std::optional<QString> optRes =
          script::ParseResourceFromScriptVariant(resource, m_wpDbManager.lock(),
                                                 m_spProject,
                                                 "setBeatResource", &sError);
      if (optRes.has_value())
      {
        QStringList vsResRet = QStringList() << optRes.value();
        emit pSignalEmitter->setBeatResource(vsResRet);
      }
      else
      {
        emit m_pSignalEmitter->showError(sError, QtMsgType::QtWarningMsg);
      }
    }
    // ...or an array of the above.
    else
    {
      QString sError;
      std::optional<QStringList> optRes =
          script::ParseResourceListFromScriptVariant(resource, m_wpDbManager.lock(),
                                                 m_spProject,
                                                 "setBeatResource", &sError);
      if (optRes.has_value())
      {
        QStringList vsResRet = optRes.value();
        emit pSignalEmitter->setBeatResource(vsResRet);
      }
      else
      {
        emit m_pSignalEmitter->showError(sError, QtMsgType::QtWarningMsg);
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptMetronome::setMuted(bool bMuted)
{
  if (!CheckIfScriptCanRun()) { return; }
  emit SignalEmitter<CMetronomeSignalEmitter>()->setMuted(bMuted);
}

//----------------------------------------------------------------------------------------
//
void CScriptMetronome::setPattern(const QList<double>& vdPattern)
{
  if (!CheckIfScriptCanRun()) { return; }
  emit SignalEmitter<CMetronomeSignalEmitter>()->setPattern(vdPattern);
}

//----------------------------------------------------------------------------------------
//
void CScriptMetronome::setVolume(double dVolume)
{
  if (!CheckIfScriptCanRun()) { return; }
  emit SignalEmitter<CMetronomeSignalEmitter>()->setVolume(dVolume);
}

//----------------------------------------------------------------------------------------
//
void CScriptMetronome::start()
{
  if (!CheckIfScriptCanRun()) { return; }
  emit SignalEmitter<CMetronomeSignalEmitter>()->start();
}

//----------------------------------------------------------------------------------------
//
void CScriptMetronome::stop()
{
  if (!CheckIfScriptCanRun()) { return; }
  emit SignalEmitter<CMetronomeSignalEmitter>()->stop();
}
