#include "ScriptBackground.h"
#include "Application.h"
#include "CommonScriptHelpers.h"
#include "Systems/DatabaseManager.h"
#include "Systems/Database/Project.h"
#include "Systems/Database/Resource.h"

#include <QtLua/Value>
#include <QtLua/State>

#include <QDebug>

CBackgroundSignalEmitter::CBackgroundSignalEmitter() :
  CScriptRunnerSignalEmiter()
{

}
CBackgroundSignalEmitter::~CBackgroundSignalEmitter()
{

}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<CScriptCommunicator>
CBackgroundSignalEmitter::CreateCommunicatorImpl(std::shared_ptr<CScriptRunnerSignalEmiterAccessor> spAccessor)
{
  return std::make_shared<CBackgroundScriptCommunicator>(spAccessor);
}

//----------------------------------------------------------------------------------------
//
CBackgroundScriptCommunicator::CBackgroundScriptCommunicator(
  const std::weak_ptr<CScriptRunnerSignalEmiterAccessor>& spEmitter) :
  CScriptCommunicator(spEmitter)
{}
CBackgroundScriptCommunicator::~CBackgroundScriptCommunicator() = default;

//----------------------------------------------------------------------------------------
//
CScriptObjectBase* CBackgroundScriptCommunicator::CreateNewScriptObject(QPointer<QJSEngine> pEngine)
{
  return new CScriptBackground(weak_from_this(), pEngine);
}

CScriptObjectBase* CBackgroundScriptCommunicator::CreateNewScriptObject(QPointer<CJsonInstructionSetParser> pParser)
{
  Q_UNUSED(pParser)
  return nullptr;
}

CScriptObjectBase* CBackgroundScriptCommunicator::CreateNewScriptObject(QtLua::State* pState)
{
  return new CScriptBackground(weak_from_this(), pState);
}
CScriptObjectBase* CBackgroundScriptCommunicator::CreateNewSequenceObject()
{
  return nullptr;
}

//----------------------------------------------------------------------------------------
//
CScriptBackground::CScriptBackground(std::weak_ptr<CScriptCommunicator> pCommunicator,
                                     QPointer<QJSEngine> pEngine) :
  CJsScriptObjectBase(pCommunicator, pEngine),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>())
{
}
CScriptBackground::CScriptBackground(std::weak_ptr<CScriptCommunicator> pCommunicator,
                                     QtLua::State* pState) :
  CJsScriptObjectBase(pCommunicator, pState),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>())
{
}
CScriptBackground::~CScriptBackground()
{
}

//----------------------------------------------------------------------------------------
//
void CScriptBackground::setBackgroundColor(QVariant color)
{
  if (!CheckIfScriptCanRun()) { return; }

  // old version compatibility
  double dAlphaMultiplierFromOldVersion = 1.0;
  if (nullptr != m_spProject)
  {
    QReadLocker locker(&m_spProject->m_rwLock);
    if (m_spProject->m_iTargetVersion < SVersion(1,1,0))
    {
      dAlphaMultiplierFromOldVersion = 0.5;
    }
  }

  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CBackgroundSignalEmitter>())
    {
      QString sError;
      std::optional<QColor> optCol =
          script::ParseColorFromScriptVariant(color, 128, "setBackgroundColor", &sError);
      if (optCol.has_value())
      {
        QColor colorRet = optCol.value();
        colorRet.setAlpha(colorRet.alpha()*dAlphaMultiplierFromOldVersion);
        emit spSignalEmitter->backgroundColorChanged(colorRet);
      }
      else
      {
        emit spSignalEmitter->showError(sError, QtMsgType::QtWarningMsg);
      }

      return;
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptBackground::setBackgroundTexture(QVariant resource)
{
  if (!CheckIfScriptCanRun()) { return; }

  if (auto spComm = m_wpCommunicator.lock())
  {
      if (auto spSignalEmitter = spComm->LockedEmitter<CBackgroundSignalEmitter>())
      {
      QString sError;
      std::optional<QString> optRes =
          script::ParseResourceFromScriptVariant(resource, m_wpDbManager.lock(),
                                                 m_spProject,
                                                 "setBackgroundTexture", &sError);
      if (optRes.has_value())
      {
        QString resRet = optRes.value();
        emit spSignalEmitter->backgroundTextureChanged(resRet);
      }
      else
      {
        emit spSignalEmitter->showError(sError, QtMsgType::QtWarningMsg);
      }
    }
  }
}
