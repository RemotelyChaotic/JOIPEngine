#include "ScriptNotification.h"
#include "Application.h"
#include "CommonScriptHelpers.h"
#include "ScriptDbWrappers.h"

#include "Systems/DatabaseManager.h"

#include "Systems/EOS/CommandEosNotificationCloseBase.h"
#include "Systems/EOS/CommandEosNotificationCreateBase.h"
#include "Systems/EOS/EosCommands.h"
#include "Systems/EOS/EosHelpers.h"

#include "Systems/JSON/JsonInstructionBase.h"
#include "Systems/JSON/JsonInstructionSetParser.h"

#include "Systems/Project.h"
#include "Systems/Resource.h"

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
std::shared_ptr<CScriptObjectBase> CNotificationSignalEmiter::CreateNewScriptObject(QPointer<QJSEngine> pEngine)
{
  return std::make_shared<CScriptNotification>(this, pEngine);
}
std::shared_ptr<CScriptObjectBase> CNotificationSignalEmiter::CreateNewScriptObject(QPointer<CJsonInstructionSetParser> pParser)
{
  return std::make_shared<CEosScriptNotification>(this, pParser);
}
std::shared_ptr<CScriptObjectBase> CNotificationSignalEmiter::CreateNewScriptObject(QtLua::State* pState)
{
  return std::make_shared<CScriptNotification>(this, pState);
}

//----------------------------------------------------------------------------------------
//
CScriptNotification::CScriptNotification(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                                         QPointer<QJSEngine> pEngine) :
  CJsScriptObjectBase(pEmitter, pEngine),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>())
{
  Initialize();
}
CScriptNotification::CScriptNotification(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                                         QtLua::State* pState) :
  CJsScriptObjectBase(pEmitter, pState),
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
  emit SignalEmitter<CNotificationSignalEmiter>()->clearNotifications();
  emit SignalOverlayCleared();
}

//----------------------------------------------------------------------------------------
//
void CScriptNotification::hide(QString sId)
{
  if (!CheckIfScriptCanRun()) { return; }
  emit SignalEmitter<CNotificationSignalEmiter>()->hideNotification(sId);
  emit SignalOverlayClosed(sId);
}

//----------------------------------------------------------------------------------------
//
void CScriptNotification::setIconAlignment(qint32 alignment)
{
  if (!CheckIfScriptCanRun()) { return; }
  emit SignalEmitter<CNotificationSignalEmiter>()->iconAlignmentChanged(alignment);
}

//----------------------------------------------------------------------------------------
//
void CScriptNotification::setPortrait(QVariant resource)
{
  if (!CheckIfScriptCanRun()) { return; }

  QString sResource = GetResource(resource, "setPortrait");
  emit SignalEmitter<CNotificationSignalEmiter>()->portraitChanged(sResource);
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
void CScriptNotification::setTextBackgroundColor(QVariant color)
{
  if (!CheckIfScriptCanRun()) { return; }

  QColor colorConverted = GetColor(color, "setTextBackgroundColor");
  emit SignalEmitter<CNotificationSignalEmiter>()->textBackgroundColorChanged(colorConverted);
}

//----------------------------------------------------------------------------------------
//
void CScriptNotification::setTextColor(QVariant color)
{
  if (!CheckIfScriptCanRun()) { return; }

  QColor colorConverted = GetColor(color, "setTextColor");
  emit SignalEmitter<CNotificationSignalEmiter>()->textColorChanged(colorConverted);
}

//----------------------------------------------------------------------------------------
//
void CScriptNotification::setWidgetBackgroundColor(QVariant color)
{
  if (!CheckIfScriptCanRun()) { return; }

  QColor colorConverted = GetColor(color, "setWidgetBackgroundColor");
  emit SignalEmitter<CNotificationSignalEmiter>()->widgetBackgroundColorChanged(colorConverted);
}

//----------------------------------------------------------------------------------------
//
void CScriptNotification::setWidgetColor(QVariant color)
{
  if (!CheckIfScriptCanRun()) { return; }

  QColor colorConverted = GetColor(color, "setWidgetColor");
  emit SignalEmitter<CNotificationSignalEmiter>()->widgetColorChanged(colorConverted);
}

//----------------------------------------------------------------------------------------
//
void CScriptNotification::Initialize()
{
  auto spEmiter = SignalEmitter<CNotificationSignalEmiter>();
  connect(spEmiter, &CNotificationSignalEmiter::showNotificationClick,
          this, [this] (QString sId, QString sOnEvt){
    QString sCopy = sOnEvt;
    sCopy.detach();
    if (!sCopy.isEmpty())
    {
      emit SignalOverlayRunAsync(sId, sOnEvt);
    }
  }, Qt::QueuedConnection);
  connect(spEmiter, &CNotificationSignalEmiter::showNotificationTimeout,
          this, [this] (QString sId, QString sOnEvt){
    QString sCopy = sOnEvt;
    sCopy.detach();
    if (!sCopy.isEmpty())
    {
      emit SignalOverlayRunAsync(sId, sOnEvt);
    }
  }, Qt::QueuedConnection);
}

//----------------------------------------------------------------------------------------
//
QColor CScriptNotification::GetColor(const QVariant& color, const QString& sSource)
{
  if (nullptr != m_pSignalEmitter)
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
      emit m_pSignalEmitter->showError(sError, QtMsgType::QtWarningMsg);
    }
  }

  return QColor();
}

//----------------------------------------------------------------------------------------
//
QString CScriptNotification::GetResource(const QVariant& resource, const QString& sSource)
{
  if (nullptr != m_pSignalEmitter)
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
      emit m_pSignalEmitter->showError(sError, QtMsgType::QtWarningMsg);
    }
  }
  return QString();
}

//----------------------------------------------------------------------------------------
//
void CScriptNotification::Show(QString sId, QString sTitle, QString sButtonText,
                               double dTimeS, QString sOnButton, QString sOnTimeout)
{
  emit SignalEmitter<CNotificationSignalEmiter>()->showNotification(sId, sTitle, sButtonText,
                                                                    dTimeS, sOnButton, sOnTimeout);
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
        if (HasValue(args, "buttonCommands") && IsOk<EArgumentType::eArray>(itButtonCommands))
        {
          tInstructionMapValue argsCopyButtons;
          tInstructionArrayValue items = std::get<tInstructionArrayValue>(itButtonCommands);
          if (items.size() > 0)
          {
            sIdButton = sId + "_Button";
            argsCopyButtons.insert({"onButtonCommand_Impl", SInstructionArgumentValue{EArgumentType::eArray, true}});
            argsCopyButtons.insert({"buttonCommands", SInstructionArgumentValue{EArgumentType::eArray, items}});
            vForks.push_back({argsCopyButtons, sIdButton, false});
          }
        }
        if (HasValue(args, "timerCommands") && IsOk<EArgumentType::eArray>(itTimerCommands))
        {
          tInstructionMapValue argsCopyTimer;
          tInstructionArrayValue items = std::get<tInstructionArrayValue>(itTimerCommands);
          if (items.size() > 0)
          {
            sIdTimer = sId + "_Timer";
            argsCopyTimer.insert({"onButtonCommand_Impl", SInstructionArgumentValue{EArgumentType::eArray, true}});
            argsCopyTimer.insert({"timerCommands", SInstructionArgumentValue{EArgumentType::eArray, items}});
            vForks.push_back({argsCopyTimer, sIdTimer, false});
          }
        }

        m_pParent->Show(sId, sTitle, sButtonLabel, iTimerDurationMs, sIdButton, sIdTimer);

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
CEosScriptNotification::CEosScriptNotification(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                                               QPointer<CJsonInstructionSetParser> pParser) :
  CEosScriptObjectBase(pEmitter, pParser),
  m_spCommandCreate(std::make_shared<CCommandEosNotificationCreate>(this)),
  m_spCommandClose(std::make_shared<CCommandEosNotificationClose>(this))
{
  pParser->RegisterInstruction(eos::c_sCommandNotificationCreate, m_spCommandCreate);
  pParser->RegisterInstruction(eos::c_sCommandNotificationClose, m_spCommandClose);

  auto spEmiter = SignalEmitter<CNotificationSignalEmiter>();
  connect(spEmiter, &CNotificationSignalEmiter::showNotificationClick,
          this, [this] (QString, QString sOnEvt){
    emit SignalOverlayRunAsync(sOnEvt);
  }, Qt::QueuedConnection);
  connect(spEmiter, &CNotificationSignalEmiter::showNotificationTimeout,
          this, [this] (QString, QString sOnEvt){
    emit SignalOverlayRunAsync(sOnEvt);
  }, Qt::QueuedConnection);
}
CEosScriptNotification::~CEosScriptNotification()
{
}

//----------------------------------------------------------------------------------------
//
void CEosScriptNotification::Hide(QString sId)
{
  if (!CheckIfScriptCanRun()) { return; }
  emit SignalEmitter<CNotificationSignalEmiter>()->hideNotification(sId);
  emit SignalOverlayClosed(sId);
}

//----------------------------------------------------------------------------------------
//
void CEosScriptNotification::Show(QString sId, const QString& sTitle, const QString& sButtonText,
                                  double dTimeS, const QString&  sOnButton, const QString& sOnTimeout)
{
  if (!CheckIfScriptCanRun()) { return; }
  emit SignalEmitter<CNotificationSignalEmiter>()->showNotification(sId, sTitle, sButtonText,
                                                                    dTimeS, sOnButton, sOnTimeout);
}
