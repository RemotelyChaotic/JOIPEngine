#include "ScriptNotification.h"
#include "Application.h"
#include "CommonScriptHelpers.h"
#include "IScriptRunner.h"
#include "ScriptDbWrappers.h"

#include "Systems/DatabaseManager.h"

#include "Systems/EOS/CommandEosNotificationCloseBase.h"
#include "Systems/EOS/CommandEosNotificationCreateBase.h"
#include "Systems/EOS/EosCommands.h"
#include "Systems/EOS/EosHelpers.h"

#include "Systems/JSON/JsonInstructionBase.h"
#include "Systems/JSON/JsonInstructionSetParser.h"

#include "Systems/Database/Project.h"
#include "Systems/Database/Resource.h"

//----------------------------------------------------------------------------------------
//
CNotificationSignalEmiter::CNotificationSignalEmiter() :
  CScriptRunnerSignalEmiter()
{

}
CNotificationSignalEmiter::~CNotificationSignalEmiter()
{

}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<CScriptCommunicator>
CNotificationSignalEmiter::CreateCommunicatorImpl(std::shared_ptr<CScriptRunnerSignalEmiterAccessor> spAccessor)
{
  return std::make_shared<CNotificationScriptCommunicator>(spAccessor);
}

//----------------------------------------------------------------------------------------
//
CNotificationScriptCommunicator::CNotificationScriptCommunicator(
  const std::weak_ptr<CScriptRunnerSignalEmiterAccessor>& spEmitter) :
  CScriptCommunicator(spEmitter)
{}
CNotificationScriptCommunicator::~CNotificationScriptCommunicator() = default;

//----------------------------------------------------------------------------------------
//
CScriptObjectBase* CNotificationScriptCommunicator::CreateNewScriptObject(QPointer<QJSEngine> pEngine)
{
  return new CScriptNotification(weak_from_this(), pEngine);
}
CScriptObjectBase* CNotificationScriptCommunicator::CreateNewScriptObject(QPointer<CJsonInstructionSetParser> pParser)
{
  return new CEosScriptNotification(weak_from_this(), pParser);
}
CScriptObjectBase* CNotificationScriptCommunicator::CreateNewScriptObject(QtLua::State* pState)
{
  return new CScriptNotification(weak_from_this(), pState);
}
CScriptObjectBase* CNotificationScriptCommunicator::CreateNewSequenceObject()
{
  return nullptr;
}

//----------------------------------------------------------------------------------------
//
CScriptNotification::CScriptNotification(std::weak_ptr<CScriptCommunicator> pCommunicator,
                                         QPointer<QJSEngine> pEngine) :
  CJsScriptObjectBase(pCommunicator, pEngine),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>())
{
  Initialize();
}
CScriptNotification::CScriptNotification(std::weak_ptr<CScriptCommunicator> pCommunicator,
                                         QtLua::State* pState) :
  CJsScriptObjectBase(pCommunicator, pState),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>())
{
  Initialize();
}
CScriptNotification::~CScriptNotification()
{
}

//----------------------------------------------------------------------------------------
//
void CScriptNotification::clear()
{
  if (!CheckIfScriptCanRun()) { return; }

  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CNotificationSignalEmiter>())
    {
      emit spSignalEmitter->clearNotifications();
      emit SignalOverlayCleared();
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptNotification::hide(QString sId)
{
  if (!CheckIfScriptCanRun()) { return; }

  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CNotificationSignalEmiter>())
    {
      if (IScriptRunnerFactory::c_sMainRunner == sId)
      {
        emit spSignalEmitter->showError(
              tr("Can not hide %1.").arg(IScriptRunnerFactory::c_sMainRunner),
              QtMsgType::QtWarningMsg);
        return;
      }

      emit spSignalEmitter->hideNotification(sId);
    }
  }

  emit SignalOverlayClosed(sId);
}

//----------------------------------------------------------------------------------------
//
void CScriptNotification::setIconAlignment(qint32 alignment)
{
  if (!CheckIfScriptCanRun()) { return; }

  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CNotificationSignalEmiter>())
    {
      spSignalEmitter->iconAlignmentChanged(alignment);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptNotification::setPortrait(QVariant resource)
{
  if (!CheckIfScriptCanRun()) { return; }

  QString sResource = GetResource(resource, "setPortrait");
  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CNotificationSignalEmiter>())
    {
      emit spSignalEmitter->portraitChanged(sResource);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptNotification::show(QString sId, QString sTitle)
{
  if (!CheckIfScriptCanRun()) { return; }
  Show(sId, sTitle, QString(), -1, QString(), QString());
}

//----------------------------------------------------------------------------------------
//
void CScriptNotification::show(QString sId, QString sTitle, QVariant sButtonTextOrTimeout)
{
  if (!CheckIfScriptCanRun()) { return; }
  if (sButtonTextOrTimeout.type() == QVariant::String)
  {
    QString sButtonText = sButtonTextOrTimeout.toString();
    Show(sId, sTitle, sButtonText, -1, QString(), QString());
  }
  else if (sButtonTextOrTimeout.isNull())
  {
    Show(sId, sTitle, QString(), -1, QString(), QString());
  }
  else if (sButtonTextOrTimeout.canConvert(QVariant::Double))
  {
    double dTimeS = sButtonTextOrTimeout.toDouble();
    Show(sId, sTitle, QString(), dTimeS, QString(), QString());
  }
  else
  {
    Show(sId, sTitle, QString(), -1, QString(), QString());
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptNotification::show(QString sId, QString sTitle, QVariant sButtonTextOrTimeout,
                               QVariant onButtonOnTimeoutOrTime)
{
  if (!CheckIfScriptCanRun()) { return; }

  if (sButtonTextOrTimeout.isNull())
  {
    Show(sId, sTitle, QString(), -1, QString(), QString());
  }
  else if (sButtonTextOrTimeout.type() == QVariant::String)
  {
    QString sButtonText = sButtonTextOrTimeout.toString();
    if (onButtonOnTimeoutOrTime.isNull())
    {
      Show(sId, sTitle, sButtonText, -1, QString(), QString());
    }
    else if (onButtonOnTimeoutOrTime.type() == QVariant::String)
    {
      QString sOnButtonResolved = GetResource(onButtonOnTimeoutOrTime, "show");
      Show(sId, sTitle, sButtonText, -1, sOnButtonResolved, QString());
    }
    else if (onButtonOnTimeoutOrTime.canConvert(QVariant::Double))
    {
      double dTimeS = onButtonOnTimeoutOrTime.toDouble();
      Show(sId, sTitle, sButtonText, dTimeS, QString(), QString());
    }
    else
    {
      Show(sId, sTitle, sButtonText, -1, QString(), QString());
    }
  }
  else if (sButtonTextOrTimeout.canConvert(QVariant::Double))
  {
    double dTimeS = sButtonTextOrTimeout.toDouble();
    QString sOnTimeoutResolved = GetResource(onButtonOnTimeoutOrTime, "show");
    Show(sId, sTitle, QString(), dTimeS, QString(), sOnTimeoutResolved);
  }
  else
  {
    Show(sId, sTitle, QString(), -1, QString(), QString());
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptNotification::show(QString sId, QString sTitle, QString sButtonText, double dTimeS, QVariant sOnButton)
{
  if (!CheckIfScriptCanRun()) { return; }
  QString sOnButtonResolved = GetResource(sOnButton, "show");
  Show(sId, sTitle, sButtonText, dTimeS, sOnButtonResolved, QString());
}

//----------------------------------------------------------------------------------------
//
void CScriptNotification::show(QString sId, QString sTitle, QString sButtonText, double dTimeS, QVariant sOnButton, QVariant sOnTimeout)
{
  if (!CheckIfScriptCanRun()) { return; }
  QString sOnButtonResolved = GetResource(sOnButton, "show");
  QString sOnTimeoutResolved = GetResource(sOnTimeout, "show");
  Show(sId, sTitle, sButtonText, dTimeS, sOnButtonResolved, sOnTimeoutResolved);
}

//----------------------------------------------------------------------------------------
//
void CScriptNotification::setShortcut(QString sShortcut)
{
  if (!CheckIfScriptCanRun()) { return; }

  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CNotificationSignalEmiter>())
    {
      emit spSignalEmitter->shortcutChanged(sShortcut);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptNotification::setTextBackgroundColor(QVariant color)
{
  if (!CheckIfScriptCanRun()) { return; }

  QColor colorConverted = GetColor(color, "setTextBackgroundColor");
  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CNotificationSignalEmiter>())
    {
      emit spSignalEmitter->textBackgroundColorChanged(colorConverted);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptNotification::setTextColor(QVariant color)
{
  if (!CheckIfScriptCanRun()) { return; }

  QColor colorConverted = GetColor(color, "setTextColor");
  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CNotificationSignalEmiter>())
    {
      spSignalEmitter->textColorChanged(colorConverted);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptNotification::setWidgetBackgroundColor(QVariant color)
{
  if (!CheckIfScriptCanRun()) { return; }

  QColor colorConverted = GetColor(color, "setWidgetBackgroundColor");
  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CNotificationSignalEmiter>())
    {
      emit spSignalEmitter->widgetBackgroundColorChanged(colorConverted);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptNotification::setWidgetColor(QVariant color)
{
  if (!CheckIfScriptCanRun()) { return; }

  QColor colorConverted = GetColor(color, "setWidgetColor");
  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CNotificationSignalEmiter>())
    {
      spSignalEmitter->widgetColorChanged(colorConverted);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptNotification::registerClickCallback(const QString& sId, QVariant callback)
{
  if (!CheckIfScriptCanRun()) { return; }

  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CNotificationSignalEmiter>())
    {
      if (callback.canConvert<QtLua::Value>())
      {
        QtLua::Value fn = callback.value<QtLua::Value>();
        if (fn.type() == QtLua::Value::TFunction)
        {
          m_clickCallbacks[sId] = fn;
        }
      }
      else if (callback.canConvert<QJSValue>())
      {
        QJSValue fn = callback.value<QJSValue>();
        if (fn.isCallable())
        {
          m_clickCallbacks[sId] = fn;
        }
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptNotification::registerTimeouCallback(const QString& sId, QVariant callback)
{
  if (!CheckIfScriptCanRun()) { return; }

  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CNotificationSignalEmiter>())
    {
      if (callback.canConvert<QtLua::Value>())
      {
        QtLua::Value fn = callback.value<QtLua::Value>();
        if (fn.type() == QtLua::Value::TFunction)
        {
          m_timeoutCallbacks[sId] = fn;
        }
      }
      else if (callback.canConvert<QJSValue>())
      {
        QJSValue fn = callback.value<QJSValue>();
        if (fn.isCallable())
        {
          m_timeoutCallbacks[sId] = fn;
        }
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptNotification::HandleClicked(const QString& sId)
{
  if (!CheckIfScriptCanRun()) { return; }

  HandleCallbacks(sId, &m_clickCallbacks);
}

//----------------------------------------------------------------------------------------
//
void CScriptNotification::HandleTimeout(const QString& sId)
{
  if (!CheckIfScriptCanRun()) { return; }

  HandleCallbacks(sId, &m_timeoutCallbacks);
}

//----------------------------------------------------------------------------------------
//
void CScriptNotification::Initialize()
{
  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CNotificationSignalEmiter>())
    {
      connect(spSignalEmitter.Get(), &CNotificationSignalEmiter::showNotificationClick,
              this, [this] (QString sId, QString sOnEvt){
        QString sCopy = sOnEvt;
        sCopy.detach();
        if (!sCopy.isEmpty())
        {
          if (IScriptRunnerFactory::c_sMainRunner == sId)
          {
            if (auto spComm = m_wpCommunicator.lock())
            {
              if (auto spSignalEmitter = spComm->LockedEmitter<CNotificationSignalEmiter>())
              {
                emit spSignalEmitter->showError(
                      tr("Id of overlay must not be %1").arg(IScriptRunnerFactory::c_sMainRunner),
                      QtMsgType::QtWarningMsg);
              }
            }
            return;
          }
          HandleClicked(sId);
          emit SignalOverlayRunAsync(sId, sOnEvt);
        }
      }, Qt::QueuedConnection);
      connect(spSignalEmitter.Get(), &CNotificationSignalEmiter::showNotificationTimeout,
              this, [this] (QString sId, QString sOnEvt){
        QString sCopy = sOnEvt;
        sCopy.detach();
        if (!sCopy.isEmpty())
        {
          if (IScriptRunnerFactory::c_sMainRunner == sId)
          {
            if (auto spComm = m_wpCommunicator.lock())
            {
              if (auto spSignalEmitter = spComm->LockedEmitter<CNotificationSignalEmiter>())
              {
                emit spSignalEmitter->showError(
                      tr("Id of overlay must not be %1").arg(IScriptRunnerFactory::c_sMainRunner),
                      QtMsgType::QtWarningMsg);
              }
            }
            return;
          }
          HandleTimeout(sId);
          emit SignalOverlayRunAsync(sId, sOnEvt);
        }
      }, Qt::QueuedConnection);
    }
  }
}

//----------------------------------------------------------------------------------------
//
QColor CScriptNotification::GetColor(const QVariant& color, const QString& sSource)
{
  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CNotificationSignalEmiter>())
    {
      QString sError;
      std::optional<QColor> optCol =
          script::ParseColorFromScriptVariant(color, 255, sSource, &sError);
      if (optCol.has_value())
      {
        return optCol.value();
      }
      else
      {
        emit spSignalEmitter->showError(sError, QtMsgType::QtWarningMsg);
      }
    }
  }

  return QColor();
}

//----------------------------------------------------------------------------------------
//
QString CScriptNotification::GetResource(const QVariant& resource, const QString& sSource)
{
  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CNotificationSignalEmiter>())
    {
      QString sError;
      std::optional<QString> optRes =
          script::ParseResourceFromScriptVariant(resource, m_wpDbManager.lock(),
                                                 m_spProject,
                                                 sSource, &sError);
      if (optRes.has_value())
      {
        return optRes.value();
      }
      else
      {
        emit spSignalEmitter->showError(sError, QtMsgType::QtWarningMsg);
      }
    }
  }
  return QString();
}

//----------------------------------------------------------------------------------------
//
void CScriptNotification::HandleCallbacks(const QString& sId, std::map<QString, script::tCallbackValue>* pCallbacks)
{
  QString sError;
  if (nullptr != m_pEngine)
  {
    auto it = pCallbacks->find(sId);
    if (pCallbacks->end() != it && std::holds_alternative<QJSValue>(it->second))
    {
      if (!script::CallCallback(std::get<QJSValue>(it->second), QJSValueList() << QJSValue(sId),
                                &sError))
      {
        if (auto spComm = m_wpCommunicator.lock())
        {
          if (auto spSignalEmitter = spComm->LockedEmitter<CNotificationSignalEmiter>())
          {
            emit spSignalEmitter->showError(sError, QtMsgType::QtCriticalMsg);
          }
        }
      }
    }
  }
  else if (nullptr != m_pState)
  {
    auto it = pCallbacks->find(sId);
    if (pCallbacks->end() != it && std::holds_alternative<QtLua::Value>(it->second))
    {
      if (!script::CallCallback(std::get<QtLua::Value>(it->second), QVariantList() << sId,
                                &sError))
      {
        if (auto spComm = m_wpCommunicator.lock())
        {
          if (auto spSignalEmitter = spComm->LockedEmitter<CNotificationSignalEmiter>())
          {
            emit spSignalEmitter->showError(sError, QtMsgType::QtCriticalMsg);
          }
        }
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptNotification::Show(QString sId, QString sTitle, QString sButtonText,
                               double dTimeS, QString sOnButton, QString sOnTimeout)
{
  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CNotificationSignalEmiter>())
    {
      spSignalEmitter->showNotification(sId, sTitle, sButtonText, dTimeS, sOnButton, sOnTimeout);
    }
  }
}

//----------------------------------------------------------------------------------------
//
class CCommandEosNotificationCreate : public CCommandEosNotificationCreateBase
{
public:
  CCommandEosNotificationCreate(CEosScriptNotification* pParent) :
    CCommandEosNotificationCreateBase(),
    m_pParent(pParent)
  {}
  ~CCommandEosNotificationCreate() override {}

  IJsonInstructionBase::tRetVal Call(const tInstructionMapValue& args) override
  {
    if (nullptr != m_pParent)
    {
      const auto& itId = GetValue<EArgumentType::eString>(args, "id");
      const auto& itTitle = GetValue<EArgumentType::eString>(args, "title");
      const auto& itButtonLabel = GetValue<EArgumentType::eString>(args, "buttonLabel");
      const auto& itTimerDuration = GetValue<EArgumentType::eString>(args, "timerDuration");

      const auto& itButtonImpl = GetValue<EArgumentType::eBool>(args, "onButtonCommand_Impl");
      const auto& itTimerImpl = GetValue<EArgumentType::eBool>(args, "onTimerCommand_Impl");

      const auto& itButtonCommands = GetValue<EArgumentType::eArray>(args, "buttonCommands");
      const auto& itTimerCommands = GetValue<EArgumentType::eArray>(args, "timerCommands");

      if (HasValue(args, "onButtonCommand_Impl") && IsOk<EArgumentType::eBool>(itButtonImpl) &&
          std::get<bool>(itButtonImpl) &&
          HasValue(args, "buttonCommands") && IsOk<EArgumentType::eArray>(itButtonCommands))
      {
        QStringList vList;
        tInstructionArrayValue items = std::get<tInstructionArrayValue>(itButtonCommands);
        for (size_t i = 0; items.size() > i; ++i)
        {
          const auto& itCommand = GetValue<EArgumentType::eObject>(items, i);
          if (IsOk<EArgumentType::eString>(itCommand))
          {
            vList << std::get<QString>(itCommand);
          }
        }
        if (vList.size() > 0)
        {
          return SRunRetVal<ENextCommandToCall::eChild>(0);
        }
      }
      else if (HasValue(args, "onTimerCommand_Impl") && IsOk<EArgumentType::eBool>(itTimerImpl) &&
               std::get<bool>(itTimerImpl) &&
               HasValue(args, "timerCommands") && IsOk<EArgumentType::eArray>(itTimerCommands))
      {
        QStringList vList;
        tInstructionArrayValue items = std::get<tInstructionArrayValue>(itTimerCommands);
        for (size_t i = 0; items.size() > i; ++i)
        {
          const auto& itCommand = GetValue<EArgumentType::eObject>(items, i);
          if (IsOk<EArgumentType::eString>(itCommand))
          {
            vList << std::get<QString>(itCommand);
          }
        }
        if (vList.size() > 0)
        {
          return SRunRetVal<ENextCommandToCall::eChild>(0);
        }
      }
      else
      {
        QString sId;
        if (HasValue(args, "id") && IsOk<EArgumentType::eString>(itId))
        {
          sId = std::get<QString>(itId);
        }

        QString sTitle;
        if (HasValue(args, "title") && IsOk<EArgumentType::eString>(itTitle))
        {
          sTitle = std::get<QString>(itTitle);
        }
        QString sButtonLabel;
        if (HasValue(args, "buttonLabel") && IsOk<EArgumentType::eString>(itButtonLabel))
        {
          sButtonLabel = std::get<QString>(itButtonLabel);
        }
        qint64 iTimerDurationMs = -1;
        if (HasValue(args, "timerDuration") && IsOk<EArgumentType::eString>(itTimerDuration))
        {
          iTimerDurationMs = eos::ParseEosDuration(std::get<QString>(itTimerDuration));
        }

        QString sIdButton;
        QString sIdTimer;
        tvForks vForks;
        tInstructionArrayValue buttonitems;
        if (HasValue(args, "buttonCommands") && IsOk<EArgumentType::eArray>(itButtonCommands))
        {
          tInstructionMapValue argsCopyButtons;
          buttonitems = std::get<tInstructionArrayValue>(itButtonCommands);
          if (buttonitems.size() > 0)
          {
            sIdButton = sId + "_Button";
            argsCopyButtons.insert({"onButtonCommand_Impl", SInstructionArgumentValue{EArgumentType::eArray, true}});
            argsCopyButtons.insert({"buttonCommands", SInstructionArgumentValue{EArgumentType::eArray, buttonitems}});
            vForks.push_back({argsCopyButtons, sIdButton, false, 0,
                              static_cast<qint32>(buttonitems.size())});
          }
        }
        if (HasValue(args, "timerCommands") && IsOk<EArgumentType::eArray>(itTimerCommands))
        {
          tInstructionMapValue argsCopyTimer;
          tInstructionArrayValue items = std::get<tInstructionArrayValue>(itTimerCommands);
          if (items.size() > 0)
          {
            sIdTimer = sId + "_Timer";
            argsCopyTimer.insert({"onTimerCommand_Impl", SInstructionArgumentValue{EArgumentType::eArray, true}});
            argsCopyTimer.insert({"timerCommands", SInstructionArgumentValue{EArgumentType::eArray, items}});
            vForks.push_back({argsCopyTimer, sIdTimer, false,
                              static_cast<qint32>(buttonitems.size()), -1});
          }
        }

        m_pParent->Show(sId, sTitle, sButtonLabel,
                        static_cast<double>(iTimerDurationMs) / 1000,
                        sIdButton, sIdTimer);

        if (vForks.size() > 0)
        {
          return SRunRetVal<ENextCommandToCall::eForkThis>(vForks);
        }
        else
        {
          return SRunRetVal<ENextCommandToCall::eSibling>();
        }
      }
      return SJsonException{"No valid arguments for notification.", "", eos::c_sCommandNotificationCreate, 0, 0};
    }
    return SJsonException{"Internal error.", "", eos::c_sCommandNotificationCreate, 0, 0};
  }

private:
  CEosScriptNotification*       m_pParent;
};

//----------------------------------------------------------------------------------------
//
class CCommandEosNotificationClose : public CCommandEosNotificationCloseBase
{
public:
  CCommandEosNotificationClose(CEosScriptNotification* pParent) :
    CCommandEosNotificationCloseBase(),
    m_pParent(pParent)
  {}
  ~CCommandEosNotificationClose() override {}

  IJsonInstructionBase::tRetVal Call(const tInstructionMapValue& args) override
  {
    if (nullptr != m_pParent)
    {
      const auto& itId = GetValue<EArgumentType::eString>(args, "id");
      if (HasValue(args, "id") && IsOk<EArgumentType::eString>(itId))
      {
        QString sId = std::get<QString>(itId);
        m_pParent->Hide(sId);
        return SRunRetVal<ENextCommandToCall::eSibling>();
      }
      return SJsonException{"Do id for notification.", "", eos::c_sCommandNotificationClose, 0, 0};
    }
    return SJsonException{"Internal error.", "", eos::c_sCommandNotificationClose, 0, 0};
  }

private:
  CEosScriptNotification*       m_pParent;
};

//----------------------------------------------------------------------------------------
//
CEosScriptNotification::CEosScriptNotification(std::weak_ptr<CScriptCommunicator> pCommunicator,
                                               QPointer<CJsonInstructionSetParser> pParser) :
  CEosScriptObjectBase(pCommunicator, pParser),
  m_spCommandCreate(std::make_shared<CCommandEosNotificationCreate>(this)),
  m_spCommandClose(std::make_shared<CCommandEosNotificationClose>(this))
{
  pParser->RegisterInstruction(eos::c_sCommandNotificationCreate, m_spCommandCreate);
  pParser->RegisterInstruction(eos::c_sCommandNotificationClose, m_spCommandClose);

  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CNotificationSignalEmiter>())
    {
      connect(spSignalEmitter.Get(), &CNotificationSignalEmiter::showNotificationClick,
              this, [this] (QString, QString sOnEvt){
        if (IScriptRunnerFactory::c_sMainRunner == sOnEvt)
        {
          if (auto spComm = m_wpCommunicator.lock())
          {
            if (auto spSignalEmitter = spComm->LockedEmitter<CNotificationSignalEmiter>())
            {
              emit spSignalEmitter->showError(
                    tr("Id of overlay must not be %1").arg(IScriptRunnerFactory::c_sMainRunner),
                    QtMsgType::QtWarningMsg);
            }
          }
          return;
        }
        emit SignalOverlayRunAsync(sOnEvt);
      }, Qt::QueuedConnection);
      connect(spSignalEmitter.Get(), &CNotificationSignalEmiter::showNotificationTimeout,
              this, [this] (QString, QString sOnEvt){
        if (IScriptRunnerFactory::c_sMainRunner == sOnEvt)
        {
          if (auto spComm = m_wpCommunicator.lock())
          {
            if (auto spSignalEmitter = spComm->LockedEmitter<CNotificationSignalEmiter>())
            {
              emit spSignalEmitter->showError(
                    tr("Id of overlay must not be %1").arg(IScriptRunnerFactory::c_sMainRunner),
                    QtMsgType::QtWarningMsg);
            }
          }
          return;
        }
        emit SignalOverlayRunAsync(sOnEvt);
      }, Qt::QueuedConnection);
    }
  }
}
CEosScriptNotification::~CEosScriptNotification()
{
}

//----------------------------------------------------------------------------------------
//
void CEosScriptNotification::Hide(QString sId)
{
  if (!CheckIfScriptCanRun()) { return; }

  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CNotificationSignalEmiter>())
    {
      if (IScriptRunnerFactory::c_sMainRunner == sId)
      {
        emit spSignalEmitter->showError(
              tr("Can not hide %1.").arg(IScriptRunnerFactory::c_sMainRunner),
              QtMsgType::QtWarningMsg);
        return;
      }

      emit spSignalEmitter->hideNotification(sId);
    }
  }

  emit SignalOverlayClosed(sId);
}

//----------------------------------------------------------------------------------------
//
void CEosScriptNotification::Show(QString sId, const QString& sTitle, const QString& sButtonText,
                                  double dTimeS, const QString&  sOnButton, const QString& sOnTimeout)
{
  if (!CheckIfScriptCanRun()) { return; }

  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CNotificationSignalEmiter>())
    {
      emit spSignalEmitter->showNotification(sId, sTitle, sButtonText, dTimeS, sOnButton, sOnTimeout);
    }
  }
}
