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
std::shared_ptr<CScriptCommunicator>
CMetronomeSignalEmitter::CreateCommunicatorImpl(std::shared_ptr<CScriptRunnerSignalEmiterAccessor> spAccessor)
{
  return std::make_shared<CMetronomeScriptCommunicator>(spAccessor);
}

//----------------------------------------------------------------------------------------
//
CMetronomeScriptCommunicator::CMetronomeScriptCommunicator(
  const std::weak_ptr<CScriptRunnerSignalEmiterAccessor>& spEmitter) :
  CScriptCommunicator(spEmitter)
{}
CMetronomeScriptCommunicator::~CMetronomeScriptCommunicator() = default;

//----------------------------------------------------------------------------------------
//
CScriptObjectBase* CMetronomeScriptCommunicator::CreateNewScriptObject(QPointer<QJSEngine> pEngine)
{
  return new CScriptMetronome(weak_from_this(), pEngine);
}
CScriptObjectBase* CMetronomeScriptCommunicator::CreateNewScriptObject(QPointer<CJsonInstructionSetParser> pParser)
{
  Q_UNUSED(pParser)
  return nullptr;
}
CScriptObjectBase* CMetronomeScriptCommunicator::CreateNewScriptObject(QtLua::State* pState)
{
  return new CScriptMetronome(weak_from_this(), pState);
}
CScriptObjectBase* CMetronomeScriptCommunicator::CreateNewSequenceObject()
{
  return new CSequenceMetronomeRunner(weak_from_this());
}

//----------------------------------------------------------------------------------------
//
CScriptMetronome::CScriptMetronome(std::weak_ptr<CScriptCommunicator> pCommunicator,
                                   QPointer<QJSEngine> pEngine) :
  CJsScriptObjectBase(pCommunicator, pEngine),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>())
{
}
CScriptMetronome::CScriptMetronome(std::weak_ptr<CScriptCommunicator> pCommunicator,
                                   QtLua::State* pState) :
  CJsScriptObjectBase(pCommunicator, pState),
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

  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CMetronomeSignalEmitter>())
    {
      emit spSignalEmitter->setBpm(iBpm);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptMetronome::setBeatResource(QVariant resource)
{
  if (!CheckIfScriptCanRun()) { return; }

  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CMetronomeSignalEmitter>())
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
          emit spSignalEmitter->setBeatResource(vsResRet);
        }
        else
        {
          emit spSignalEmitter->showError(sError, QtMsgType::QtWarningMsg);
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
          emit spSignalEmitter->setBeatResource(vsResRet);
        }
        else
        {
          emit spSignalEmitter->showError(sError, QtMsgType::QtWarningMsg);
        }
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptMetronome::setMuted(bool bMuted)
{
  if (!CheckIfScriptCanRun()) { return; }

  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CMetronomeSignalEmitter>())
    {
      emit spSignalEmitter->setMuted(bMuted);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptMetronome::setPattern(const QList<double>& vdPattern)
{
  if (!CheckIfScriptCanRun()) { return; }

  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CMetronomeSignalEmitter>())
    {
      spSignalEmitter->setPattern(vdPattern);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptMetronome::setVolume(double dVolume)
{
  if (!CheckIfScriptCanRun()) { return; }

  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CMetronomeSignalEmitter>())
    {
      spSignalEmitter->setVolume(dVolume);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptMetronome::start()
{
  if (!CheckIfScriptCanRun()) { return; }

  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CMetronomeSignalEmitter>())
    {
      spSignalEmitter->start();
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptMetronome::stop()
{
  if (!CheckIfScriptCanRun()) { return; }

  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CMetronomeSignalEmitter>())
    {
      emit spSignalEmitter->stop();
    }
  }
}
