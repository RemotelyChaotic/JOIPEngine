#include "ScriptBackground.h"
#include "Application.h"
#include "CommonScriptHelpers.h"
#include "Systems/DatabaseManager.h"
#include "Systems/Project.h"
#include "Systems/Resource.h"

#include <QtLua/Value>
#include <QtLua/State>

CBackgroundSignalEmitter::CBackgroundSignalEmitter() :
  CScriptRunnerSignalEmiter()
{

}
CBackgroundSignalEmitter::~CBackgroundSignalEmitter()
{

}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<CScriptObjectBase> CBackgroundSignalEmitter::CreateNewScriptObject(QPointer<QJSEngine> pEngine)
{
  return std::make_shared<CScriptBackground>(this, pEngine);
}

std::shared_ptr<CScriptObjectBase> CBackgroundSignalEmitter::CreateNewScriptObject(QPointer<CJsonInstructionSetParser> pParser)
{
  Q_UNUSED(pParser)
  return nullptr;
}

std::shared_ptr<CScriptObjectBase> CBackgroundSignalEmitter::CreateNewScriptObject(QtLua::State* pState)
{
  return std::make_shared<CScriptBackground>(this, pState);
}

//----------------------------------------------------------------------------------------
//
CScriptBackground::CScriptBackground(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                                     QPointer<QJSEngine> pEngine) :
  CJsScriptObjectBase(pEmitter, pEngine),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>())
{
}
CScriptBackground::CScriptBackground(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                                     QtLua::State* pState) :
  CJsScriptObjectBase(pEmitter, pState),
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

  auto spSignalEmitter = SignalEmitter<CBackgroundSignalEmitter>();
  if (nullptr != spSignalEmitter)
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
      emit m_pSignalEmitter->showError(sError, QtMsgType::QtWarningMsg);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptBackground::setBackgroundTexture(QVariant resource)
{
  if (!CheckIfScriptCanRun()) { return; }

  auto spSignalEmitter = SignalEmitter<CBackgroundSignalEmitter>();
  if (nullptr != spSignalEmitter)
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
      emit m_pSignalEmitter->showError(sError, QtMsgType::QtWarningMsg);
    }
  }
}
