#include "ScriptNotification.h"
#include "Application.h"

#include "Systems/DatabaseManager.h"
#include "Systems/EOS/EosHelpers.h"
#include "Systems/JSON/JsonInstructionBase.h"
#include "Systems/JSON/JsonInstructionSetParser.h"
#include "Systems/Project.h"
#include "Systems/Resource.h"

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

//----------------------------------------------------------------------------------------
//
CScriptNotification::CScriptNotification(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                                         QPointer<QJSEngine> pEngine) :
  CJsScriptObjectBase(pEmitter, pEngine),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>())
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
void CScriptNotification::setPortrait(QJSValue resource)
{
  if (!CheckIfScriptCanRun()) { return; }

  QString sResource = GetResource(resource, "setPortrait()");
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
void CScriptNotification::show(QString sId, QString sTitle, QString sButtonText)
{
  if (!CheckIfScriptCanRun()) { return; }
  Show(sId, sTitle, sButtonText, -1, QString(), QString());
}

//----------------------------------------------------------------------------------------
//
void CScriptNotification::show(QString sId, QString sTitle, QString sButtonText, QJSValue sOnButton)
{
  if (!CheckIfScriptCanRun()) { return; }
  QString sOnButtonResolved = GetResource(sOnButton, "show()");
  Show(sId, sTitle, sButtonText, -1, QString(), QString());
}

//----------------------------------------------------------------------------------------
//
void CScriptNotification::show(QString sId, QString sTitle, double dTimeS)
{
  if (!CheckIfScriptCanRun()) { return; }
  Show(sId, sTitle, QString(), dTimeS, QString(), QString());
}

//----------------------------------------------------------------------------------------
//
void CScriptNotification::show(QString sId, QString sTitle, double dTimeS, QJSValue sOnTimeout)
{
  if (!CheckIfScriptCanRun()) { return; }
  QString sOnTimeoutResolved = GetResource(sOnTimeout, "show()");
  Show(sId, sTitle, QString(), dTimeS, QString(), sOnTimeoutResolved);
}

//----------------------------------------------------------------------------------------
//
void CScriptNotification::show(QString sId, QString sTitle, QString sButtonText, double dTimeS)
{
  if (!CheckIfScriptCanRun()) { return; }
  Show(sId, sTitle, sButtonText, dTimeS, QString(), QString());
}

//----------------------------------------------------------------------------------------
//
void CScriptNotification::show(QString sId, QString sTitle, QString sButtonText, double dTimeS, QJSValue sOnButton)
{
  if (!CheckIfScriptCanRun()) { return; }
  QString sOnButtonResolved = GetResource(sOnButton, "show()");
  Show(sId, sTitle, sButtonText, dTimeS, sOnButtonResolved, QString());
}

//----------------------------------------------------------------------------------------
//
void CScriptNotification::show(QString sId, QString sTitle, QString sButtonText, double dTimeS, QJSValue sOnButton, QJSValue sOnTimeout)
{
  if (!CheckIfScriptCanRun()) { return; }
  QString sOnButtonResolved = GetResource(sOnButton, "show()");
  QString sOnTimeoutResolved = GetResource(sOnTimeout, "show()");
  Show(sId, sTitle, sButtonText, dTimeS, sOnButtonResolved, sOnTimeoutResolved);
}

//----------------------------------------------------------------------------------------
//
void CScriptNotification::setTextBackgroundColor(QJSValue color)
{
  if (!CheckIfScriptCanRun()) { return; }

  QColor colorConverted = GetColor(color, "setTextBackgroundColor()");
  emit SignalEmitter<CNotificationSignalEmiter>()->textBackgroundColorChanged(colorConverted);
}

//----------------------------------------------------------------------------------------
//
void CScriptNotification::setTextColor(QJSValue color)
{
  if (!CheckIfScriptCanRun()) { return; }

  QColor colorConverted = GetColor(color, "setTextColor()");
  emit SignalEmitter<CNotificationSignalEmiter>()->textColorChanged(colorConverted);
}

//----------------------------------------------------------------------------------------
//
void CScriptNotification::setWidgetBackgroundColor(QJSValue color)
{
  if (!CheckIfScriptCanRun()) { return; }

  QColor colorConverted = GetColor(color, "setWidgetBackgroundColor()");
  emit SignalEmitter<CNotificationSignalEmiter>()->widgetBackgroundColorChanged(colorConverted);
}

//----------------------------------------------------------------------------------------
//
void CScriptNotification::setWidgetColor(QJSValue color)
{
  if (!CheckIfScriptCanRun()) { return; }

  QColor colorConverted = GetColor(color, "setWidgetColor()");
  emit SignalEmitter<CNotificationSignalEmiter>()->widgetColorChanged(colorConverted);
}

//----------------------------------------------------------------------------------------
//
QColor CScriptNotification::GetColor(const QJSValue& color, const QString& sSource)
{
  QColor colorRet;
  if (color.isString())
  {
    colorRet = QColor(color.toString());
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
      QString sError = tr("Argument error in %1. Array of three or four numbers or string was expected.");
      emit m_pSignalEmitter->showError(sError.arg(sSource), QtMsgType::QtWarningMsg);
    }
    else
    {
      if (viColorComponents.size() == 4)
      {
        colorRet =
              QColor(viColorComponents[0], viColorComponents[1], viColorComponents[2], viColorComponents[3]);
      }
      else
      {
        colorRet =
              QColor(viColorComponents[0], viColorComponents[1], viColorComponents[2]);
      }
    }
  }
  return colorRet;
}

//----------------------------------------------------------------------------------------
//
QString CScriptNotification::GetResource(const QJSValue& resource, const QString& sSource)
{
  QString sRet;
  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    if (resource.isString())
    {
      QString sResourceName = resource.toString();
      tspResource spResource = spDbManager->FindResourceInProject(m_spProject, sResourceName);
      if (nullptr != spResource)
      {
        sRet = sResourceName;
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
        tspResource spResource = pResource->Data();
        if (nullptr != spResource)
        {
          sRet = pResource->getName();
        }
        else
        {
          QString sError = tr("Resource in %1 holds no data.");
          emit m_pSignalEmitter->showError(sError.arg(sSource), QtMsgType::QtWarningMsg);
        }
      }
      else
      {
        QString sError = tr("Wrong argument-type to %1. String or resource was expected.");
        emit m_pSignalEmitter->showError(sError.arg(sSource), QtMsgType::QtWarningMsg);
      }
    }
    else if (resource.isNull() || resource.isUndefined())
    {
      sRet = QString();
    }
    else
    {
      QString sError = tr("Wrong argument-type to %1. String or resource was expected.");
      emit m_pSignalEmitter->showError(sError.arg(sSource), QtMsgType::QtWarningMsg);
    }
  }
  return sRet;
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
class CCommandEosNotificationCreate : public IJsonInstructionBase
{
public:
  CCommandEosNotificationCreate(CEosScriptNotification* pParent) :
    m_pParent(pParent),
    m_argTypes({
      {"id", SInstructionArgumentType{EArgumentType::eString}},
      {"title", SInstructionArgumentType{EArgumentType::eString}},
      {"buttonLabel", SInstructionArgumentType{EArgumentType::eString}},
      {"timerDuration", SInstructionArgumentType{EArgumentType::eString}},
      {"onButtonCommand_Impl", SInstructionArgumentType{EArgumentType::eBool}},
      {"onTimerCommand_Impl", SInstructionArgumentType{EArgumentType::eBool}},
      {"timerDuration", SInstructionArgumentType{EArgumentType::eString}},
      {"buttonCommands", SInstructionArgumentType{EArgumentType::eArray,
               MakeArgArray(EArgumentType::eObject)}},
      {"timerCommands", SInstructionArgumentType{EArgumentType::eArray,
               MakeArgArray(EArgumentType::eObject)}},
    }) {}
  ~CCommandEosNotificationCreate() override {}

  tInstructionMapType& ArgList() override
  {
    return m_argTypes;
  }

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

      if (HasValue(args, "id") && IsOk<EArgumentType::eString>(itId))
      {
        const QString sId = std::get<QString>(itId);

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
      else if (HasValue(args, "onButtonCommand_Impl") && IsOk<EArgumentType::eBool>(itButtonImpl) &&
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
      return SJsonException{"Do valid arguments for notification.", "", "notification.create", 0, 0};
    }
    return SJsonException{"Internal error.", "", "notification.create", 0, 0};
  }

private:
  CEosScriptNotification*       m_pParent;
  tInstructionMapType           m_argTypes;
};

//----------------------------------------------------------------------------------------
//
class CCommandEosNotificationClose : public IJsonInstructionBase
{
public:
  CCommandEosNotificationClose(CEosScriptNotification* pParent) :
    m_pParent(pParent),
    m_argTypes({
      {"id", SInstructionArgumentType{EArgumentType::eString}}
    }) {}
  ~CCommandEosNotificationClose() override {}

  tInstructionMapType& ArgList() override
  {
    return m_argTypes;
  }

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
      return SJsonException{"Do id for notification.", "", "notification.remove", 0, 0};
    }
    return SJsonException{"Internal error.", "", "notification.remove", 0, 0};
  }

private:
  CEosScriptNotification*       m_pParent;
  tInstructionMapType           m_argTypes;
};

//----------------------------------------------------------------------------------------
//
CEosScriptNotification::CEosScriptNotification(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                                               QPointer<CJsonInstructionSetParser> pParser) :
  CEosScriptObjectBase(pEmitter, pParser),
  m_spCommandCreate(std::make_shared<CCommandEosNotificationCreate>(this)),
  m_spCommandClose(std::make_shared<CCommandEosNotificationClose>(this))
{
  pParser->RegisterInstruction("notification.create", m_spCommandCreate);
  pParser->RegisterInstruction("notification.remove", m_spCommandClose);

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
