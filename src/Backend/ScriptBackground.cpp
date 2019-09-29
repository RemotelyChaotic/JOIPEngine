#include "ScriptBackground.h"
#include "Application.h"
#include "DatabaseManager.h"
#include "ScriptRunnerSignalEmiter.h"

CScriptBackground::CScriptBackground(std::shared_ptr<CScriptRunnerSignalEmiter> spEmitter,
                                     QJSEngine* pEngine) :
  QObject(),
  m_spSignalEmitter(spEmitter),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>()),
  m_pEngine(pEngine)
{
}

CScriptBackground::~CScriptBackground()
{
}

//----------------------------------------------------------------------------------------
//
void CScriptBackground::SetCurrentProject(tspProject spProject)
{
  m_spProject = spProject;
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
      tspResource spResource = spDbManager->FindResource(m_spProject, resource.toString());
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
        QString sError = tr("Wrong argument-type to setBackgroundTexture(). String or resource was expected.");
        emit m_spSignalEmitter->SignalShowError(sError, QtMsgType::QtWarningMsg);
      }
    }
    else
    {
      QString sError = tr("Wrong argument-type to setBackgroundTexture(). String or resource was expected.");
      emit m_spSignalEmitter->SignalShowError(sError, QtMsgType::QtWarningMsg);
    }
  }
}

//----------------------------------------------------------------------------------------
//
bool CScriptBackground::CheckIfScriptCanRun()
{
  if (m_spSignalEmitter->ScriptExecutionStatus()._to_integral() == EScriptExecutionStatus::eStopped)
  {
    QJSValue val = m_pEngine->evaluate("f();"); //undefined function -> create error
    Q_UNUSED(val);
    return false;
  }
  else
  {
    return true;
  }
}

