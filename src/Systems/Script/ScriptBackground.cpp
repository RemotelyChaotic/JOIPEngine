#include "ScriptBackground.h"
#include "Application.h"
#include "Systems/DatabaseManager.h"
#include "Systems/Project.h"
#include "Systems/Resource.h"

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
  return nullptr;
}

//----------------------------------------------------------------------------------------
//
CScriptBackground::CScriptBackground(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                                     QPointer<QJSEngine> pEngine) :
  CScriptObjectBase(pEmitter, pEngine),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>())
{
}

CScriptBackground::~CScriptBackground()
{
}

//----------------------------------------------------------------------------------------
//
void CScriptBackground::setBackgroundColor(QJSValue color)
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
  if (color.isString())
  {
    QColor col(color.toString());
    col.setAlpha(col.alpha()*dAlphaMultiplierFromOldVersion);
    emit spSignalEmitter->backgroundColorChanged(col);
  }
  else if (color.isArray())
  {
    std::vector<qint32> viColorComponents;
    const qint32 iLength = color.property("length").toInt();
    for (qint32 iIndex = 0; iLength > iIndex; iIndex++)
    {
      viColorComponents.push_back(color.property(static_cast<quint32>(iIndex)).toInt());
    }

    if (viColorComponents.size() != 4 && viColorComponents.size() != 3)
    {
      QString sError = tr("Argument error in setBackgroundColor(). Array of three or four numbers or string was expected.");
      emit m_pSignalEmitter->showError(sError, QtMsgType::QtWarningMsg);
    }
    else
    {
      if (viColorComponents.size() == 4)
      {
        QColor col(viColorComponents[0], viColorComponents[1], viColorComponents[2], viColorComponents[3]);
        col.setAlpha(col.alpha()*dAlphaMultiplierFromOldVersion);
        emit spSignalEmitter->backgroundColorChanged(col);
      }
      else
      {
        // if no alpha is given, make it half transparent
        emit spSignalEmitter->backgroundColorChanged(
              QColor(viColorComponents[0], viColorComponents[1], viColorComponents[2], 128));
      }
    }
  }
  else
  {
    QString sError = tr("Wrong argument-type to setBackgroundColor(). Array of three or four numbers or string was expected.");
    emit m_pSignalEmitter->showError(sError, QtMsgType::QtWarningMsg);
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptBackground::setBackgroundTexture(QJSValue resource)
{
  if (!CheckIfScriptCanRun()) { return; }

  auto spSignalEmitter = SignalEmitter<CBackgroundSignalEmitter>();
  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    if (resource.isString())
    {
      QString sResource = resource.toString();
      if (sResource.isNull() || sResource.isEmpty())
      {
        emit spSignalEmitter->backgroundTextureChanged(QString());
      }
      else
      {
        tspResource spResource = spDbManager->FindResourceInProject(m_spProject, sResource);
        if (nullptr != spResource)
        {
          emit spSignalEmitter->backgroundTextureChanged(sResource);
        }
        else
        {
          QString sError = tr("Resource %1 not found.");
          emit m_pSignalEmitter->showError(sError.arg(resource.toString()),
                                                  QtMsgType::QtWarningMsg);
        }
      }
    }
    else if (resource.isNull())
    {
      emit spSignalEmitter->backgroundTextureChanged(QString());
    }
    else if (resource.isQObject())
    {
      CResourceScriptWrapper* pResource = dynamic_cast<CResourceScriptWrapper*>(resource.toQObject());
      if (nullptr != pResource)
      {
        tspResource spResource = pResource->Data();
        if (nullptr != spResource)
        {
          QString sName = pResource->getName();
          emit spSignalEmitter->backgroundTextureChanged(sName);
        }
        else
        {
          QString sError = tr("Resource in setBackgroundTexture() holds no data.");
          emit m_pSignalEmitter->showError(sError, QtMsgType::QtWarningMsg);
        }
      }
      else
      {
        QString sError = tr("Wrong argument-type to setBackgroundTexture(). String, resource or null was expected.");
        emit m_pSignalEmitter->showError(sError, QtMsgType::QtWarningMsg);
      }
    }
    else
    {
      QString sError = tr("Wrong argument-type to setBackgroundTexture(). String, resource or null was expected.");
      emit m_pSignalEmitter->showError(sError, QtMsgType::QtWarningMsg);
    }
  }
}
