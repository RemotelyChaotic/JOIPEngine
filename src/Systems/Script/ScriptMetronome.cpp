#include "ScriptMetronome.h"
#include "Application.h"
#include "Systems/DatabaseManager.h"

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

//----------------------------------------------------------------------------------------
//
CScriptMetronome::CScriptMetronome(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                                   QPointer<QJSEngine> pEngine) :
  CJsScriptObjectBase(pEmitter, pEngine),
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
void CScriptMetronome::setBeatResource(QJSValue resource)
{
  if (!CheckIfScriptCanRun()) { return; }

  auto pSignalEmitter = SignalEmitter<CMetronomeSignalEmitter>();
  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    if (resource.isString())
    {
      QString sResource = resource.toString();
      tspResource spResource = spDbManager->FindResourceInProject(m_spProject, sResource);
      if (nullptr != spResource)
      {
        emit pSignalEmitter->setBeatResource(sResource);
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
      CResourceScriptWrapper* pResource = dynamic_cast<CResourceScriptWrapper*>(resource.toQObject());
      if (nullptr != pResource)
      {
        if (nullptr != pResource->Data())
        {
          emit pSignalEmitter->setBeatResource(pResource->getName());
        }
        else
        {
          QString sError = tr("Resource in setBeatResource() holds no data.");
          emit m_pSignalEmitter->showError(sError, QtMsgType::QtWarningMsg);
        }
      }
      else
      {
        QString sError = tr("Wrong argument-type to setBeatResource(). String, resource or null was expected.");
        emit m_pSignalEmitter->showError(sError, QtMsgType::QtWarningMsg);
      }
    }
    else if (resource.isNull())
    {
      emit pSignalEmitter->setBeatResource(QString());
    }
    else
    {
      QString sError = tr("Wrong argument-type to play(). String, resource or null was expected.");
      emit m_pSignalEmitter->showError(sError, QtMsgType::QtWarningMsg);
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
