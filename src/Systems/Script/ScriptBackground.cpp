#include "ScriptBackground.h"
#include "Application.h"
#include "ScriptRunnerSignalEmiter.h"
#include "Systems/DatabaseManager.h"
#include "Systems/Project.h"
#include "Systems/Resource.h"


CScriptBackground::CScriptBackground(std::shared_ptr<CScriptRunnerSignalEmiter> spEmitter,
                                     QPointer<QJSEngine> pEngine) :
  CScriptObjectBase(spEmitter, pEngine),
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

  if (color.isString())
  {
    emit m_spSignalEmitter->SignalBackgroundColorChanged(
          QColor(color.toString()));
  }
  else if (color.isArray())
  {
    std::vector<qint32> viColorComponents;
    const qint32 iLength = color.property("length").toInt();
    for (qint32 iIndex = 0; iLength > iIndex; iIndex++)
    {
      viColorComponents.push_back(color.property(static_cast<quint32>(iIndex)).toInt());
    }

    if (viColorComponents.size() != 4)
    {
      QString sError = tr("Argument error in setBackgroundColor(). Array of four numbers or string was expected.");
      emit m_spSignalEmitter->SignalShowError(sError, QtMsgType::QtWarningMsg);
    }
    else
    {
      emit m_spSignalEmitter->SignalBackgroundColorChanged(
            QColor(viColorComponents[0], viColorComponents[1], viColorComponents[2], viColorComponents[3]));
    }
  }
  else
  {
    QString sError = tr("Wrong argument-type to setBackgroundColor(). Array of three numbers or string was expected.");
    emit m_spSignalEmitter->SignalShowError(sError, QtMsgType::QtWarningMsg);
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptBackground::setBackgroundTexture(QJSValue resource)
{
  if (!CheckIfScriptCanRun()) { return; }

  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    if (resource.isString())
    {
      QString sResource = resource.toString();
      if (sResource.isNull() || sResource.isEmpty())
      {
        emit m_spSignalEmitter->SignalBackgroundTextureChanged(nullptr);
      }
      else
      {
        tspResource spResource = spDbManager->FindResource(m_spProject, sResource);
        if (nullptr != spResource)
        {
          emit m_spSignalEmitter->SignalBackgroundTextureChanged(spResource);
        }
        else
        {
          QString sError = tr("Resource %1 not found");
          emit m_spSignalEmitter->SignalShowError(sError.arg(resource.toString()),
                                                  QtMsgType::QtWarningMsg);
        }
      }
    }
    else if (resource.isNull())
    {
      emit m_spSignalEmitter->SignalBackgroundTextureChanged(nullptr);
    }
    else if (resource.isQObject())
    {
      CResource* pResource = dynamic_cast<CResource*>(resource.toQObject());
      if (nullptr != pResource)
      {
        tspResource spResource = pResource->Data();
        if (nullptr != spResource)
        {
          emit m_spSignalEmitter->SignalBackgroundTextureChanged(spResource);
        }
        else
        {
          QString sError = tr("Resource in setBackgroundTexture() holds no data.");
          emit m_spSignalEmitter->SignalShowError(sError, QtMsgType::QtWarningMsg);
        }
      }
      else
      {
        QString sError = tr("Wrong argument-type to setBackgroundTexture(). String, resource or null was expected.");
        emit m_spSignalEmitter->SignalShowError(sError, QtMsgType::QtWarningMsg);
      }
    }
    else
    {
      QString sError = tr("Wrong argument-type to setBackgroundTexture(). String, resource or null was expected.");
      emit m_spSignalEmitter->SignalShowError(sError, QtMsgType::QtWarningMsg);
    }
  }
}
