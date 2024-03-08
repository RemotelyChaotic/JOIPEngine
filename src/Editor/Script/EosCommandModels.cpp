#include "EosCommandModels.h"


CCommandEosAudioModel::CCommandEosAudioModel() :
  CCommandEosAudioBase(), IEosCommandModel()
{
}

CCommandEosAudioModel::~CCommandEosAudioModel()
{
}

//----------------------------------------------------------------------------------------
//
tInstructionMapValue CCommandEosAudioModel::DefaultArgs()
{
  return ValuesFromTypes(ArgList());
}

//----------------------------------------------------------------------------------------
//
QString CCommandEosAudioModel::DisplayName(const tInstructionMapValue& args)
{
  const auto& itLocator = GetValue<EArgumentType::eString>(args, "locator");
  const bool bHasLocator = HasValue(args, "locator") && IsOk<EArgumentType::eString>(itLocator);

  if (bHasLocator)
  {
    return QString("<b>%1</b>: %2").arg(eos::c_sCommandAudioPlay).arg(std::get<QString>(itLocator));
  }
  else
  {
    return QString("<b>%1</b>").arg(eos::c_sCommandAudioPlay);
  }
}

//----------------------------------------------------------------------------------------
//
CCommandEosChoiceModel::CCommandEosChoiceModel()  :
  CCommandEosChoiceBase(), IEosCommandModel()
{
}

CCommandEosChoiceModel::~CCommandEosChoiceModel()
{
}

//----------------------------------------------------------------------------------------
//
tInstructionMapValue CCommandEosChoiceModel::DefaultArgs()
{
  return ValuesFromTypes(ArgList());
}

//----------------------------------------------------------------------------------------
//
QString CCommandEosChoiceModel::DisplayName(const tInstructionMapValue& args)
{
  QStringList vsOptions;
  const auto& itOptions = GetValue<EArgumentType::eArray>(args, "options");
  if (HasValue(args, "options") && IsOk<EArgumentType::eArray>(itOptions))
  {
    const tInstructionArrayValue& arrOptions = std::get<tInstructionArrayValue>(itOptions);
    for (size_t i = 0; arrOptions.size() > i; ++i)
    {
      const auto& itOption = GetValue<EArgumentType::eMap>(arrOptions, i);
      if (IsOk<EArgumentType::eMap>(itOption))
      {
        const tInstructionMapValue& optionsArg = std::get<tInstructionMapValue>(itOption);
        const auto& itLabel = GetValue<EArgumentType::eString>(optionsArg, "label");
        if (HasValue(optionsArg, "label") && IsOk<EArgumentType::eString>(itLabel))
        {
          vsOptions << (QString("&quot;") + std::get<QString>(itLabel) + "&quot;");
        }
      }
    }
  }

  if (!vsOptions.isEmpty())
  {
    return QString("<b>%1</b>: %2")
        .arg(eos::c_sCommandChoice)
        .arg(vsOptions.join(", "));
  }
  else
  {
    return QString("<b>%1</b>").arg(eos::c_sCommandChoice);
  }
}

//----------------------------------------------------------------------------------------
//
void CCommandEosChoiceModel::InsertedChildAt(tInstructionMapValue* pArgs, qint32 iIndex,
                                             const QString& sIntoGroup,
                                             const QString& sInsertedChild)
{
  tChildNodeGroups groups = ChildNodeGroups(*pArgs);
  auto it = std::find_if(groups.begin(), groups.end(),
                      [&sIntoGroup](const std::tuple<QString, QString, qint32>& tuple) {
    return std::get<0>(tuple) == sIntoGroup;
  });

  if (groups.end() != it)
  {
    tInstructionArrayValue arrOptions;
    tInstructionArrayValue arrCommands;

    qint32 iIndexGroup = it - groups.begin();
    const auto& itOptions = GetValue<EArgumentType::eArray>(*pArgs, "options");
    if (HasValue(*pArgs, "options") && IsOk<EArgumentType::eArray>(itOptions))
    {
      arrOptions = std::get<tInstructionArrayValue>(itOptions);
      if (std::holds_alternative<tInstructionMapValue>(arrOptions[iIndexGroup].m_value))
      {
        tInstructionMapValue& values = std::get<tInstructionMapValue>(arrOptions[iIndexGroup].m_value);
        const auto& itCommands = GetValue<EArgumentType::eArray>(values, "commands");
        if (HasValue(*pArgs, "commands") && IsOk<EArgumentType::eArray>(itCommands))
        {
          arrCommands = std::get<tInstructionArrayValue>(itCommands);
          arrCommands.insert(arrCommands.begin()+iIndex, { EArgumentType::eObject, sInsertedChild});
        }
        values.insert_or_assign("commands",
                                SInstructionArgumentValue{ EArgumentType::eArray, arrCommands });
      }
    }
    pArgs->insert_or_assign("options",
                            SInstructionArgumentValue{ EArgumentType::eArray, arrOptions });
  }
}

//----------------------------------------------------------------------------------------
//
void CCommandEosChoiceModel::RemoveChildAt(tInstructionMapValue* pArgs, qint32 iIndex,
                                           const QString& sIntoGroup)
{
  tChildNodeGroups groups = ChildNodeGroups(*pArgs);
  auto it = std::find_if(groups.begin(), groups.end(),
                      [&sIntoGroup](const std::tuple<QString, QString, qint32>& tuple) {
    return std::get<0>(tuple) == sIntoGroup;
  });

  if (groups.end() != it)
  {
    tInstructionArrayValue arrOptions;
    tInstructionArrayValue arrCommands;

    qint32 iIndexGroup = it - groups.begin();
    const auto& itOptions = GetValue<EArgumentType::eArray>(*pArgs, "options");
    if (HasValue(*pArgs, "options") && IsOk<EArgumentType::eArray>(itOptions))
    {
      arrOptions = std::get<tInstructionArrayValue>(itOptions);
      if (std::holds_alternative<tInstructionMapValue>(arrOptions[iIndexGroup].m_value))
      {
        tInstructionMapValue& values = std::get<tInstructionMapValue>(arrOptions[iIndexGroup].m_value);
        const auto& itCommands = GetValue<EArgumentType::eArray>(values, "commands");
        if (HasValue(*pArgs, "commands") && IsOk<EArgumentType::eArray>(itCommands))
        {
          arrCommands = std::get<tInstructionArrayValue>(itCommands);
          arrCommands.erase(arrCommands.begin()+iIndex);
        }
        values.insert_or_assign("commands",
                                SInstructionArgumentValue{ EArgumentType::eArray, arrCommands });
      }
    }
    pArgs->insert_or_assign("options",
                            SInstructionArgumentValue{ EArgumentType::eArray, arrOptions });
  }
}

//----------------------------------------------------------------------------------------
//
CCommandEosDisableSceneModel::CCommandEosDisableSceneModel()  :
  CCommandEosDisableSceneBase(), IEosCommandModel()
{
}

CCommandEosDisableSceneModel::~CCommandEosDisableSceneModel()
{
}

//----------------------------------------------------------------------------------------
//
tInstructionMapValue CCommandEosDisableSceneModel::DefaultArgs()
{
  return ValuesFromTypes(ArgList());
}

//----------------------------------------------------------------------------------------
//
QString CCommandEosDisableSceneModel::DisplayName(const tInstructionMapValue& args)
{
  const auto& itTarget = GetValue<EArgumentType::eString>(args, "target");
  const bool bHasTarget = HasValue(args, "target") && IsOk<EArgumentType::eString>(itTarget);

  if (bHasTarget)
  {
    return QString("<b>%1</b>: %2").arg(eos::c_sCommandDisableScreen).arg(std::get<QString>(itTarget));
  }
  else
  {
    return QString("<b>%1</b>").arg(eos::c_sCommandDisableScreen);
  }
}

//----------------------------------------------------------------------------------------
//
CCommandEosEnableSceneModel::CCommandEosEnableSceneModel()  :
  CCommandEosEnableSceneBase(), IEosCommandModel()
{
}

CCommandEosEnableSceneModel::~CCommandEosEnableSceneModel()
{
}

//----------------------------------------------------------------------------------------
//
tInstructionMapValue CCommandEosEnableSceneModel::DefaultArgs()
{
  return ValuesFromTypes(ArgList());
}

//----------------------------------------------------------------------------------------
//
QString CCommandEosEnableSceneModel::DisplayName(const tInstructionMapValue& args)
{
  const auto& itTarget = GetValue<EArgumentType::eString>(args, "target");
  const bool bHasTarget = HasValue(args, "target") && IsOk<EArgumentType::eString>(itTarget);

  if (bHasTarget)
  {
    return QString("<b>%1</b>: %2").arg(eos::c_sCommandEnableScreen).arg(std::get<QString>(itTarget));
  }
  else
  {
    return QString("<b>%1</b>").arg(eos::c_sCommandEnableScreen);
  }
}

//----------------------------------------------------------------------------------------
//
CCommandEosEndModel::CCommandEosEndModel()  :
  CCommandEosEndBase(), IEosCommandModel()
{
}

CCommandEosEndModel::~CCommandEosEndModel()
{
}

//----------------------------------------------------------------------------------------
//
tInstructionMapValue CCommandEosEndModel::DefaultArgs()
{
  return ValuesFromTypes(ArgList());
}

//----------------------------------------------------------------------------------------
//
QString CCommandEosEndModel::DisplayName(const tInstructionMapValue& args)
{
  return QString("<b>%1</b>").arg(eos::c_sCommandEnd);
}

//----------------------------------------------------------------------------------------
//
CCommandEosEvalModel::CCommandEosEvalModel()  :
  CCommandEosEvalBase(), IEosCommandModel()
{
}

CCommandEosEvalModel::~CCommandEosEvalModel()
{
}

//----------------------------------------------------------------------------------------
//
tInstructionMapValue CCommandEosEvalModel::DefaultArgs()
{
  return ValuesFromTypes(ArgList());
}

//----------------------------------------------------------------------------------------
//
QString CCommandEosEvalModel::DisplayName(const tInstructionMapValue& args)
{
  return QString("<b>%1</b>: ...").arg(eos::c_sCommandEval);
}

//----------------------------------------------------------------------------------------
//
CCommandEosGotoModel::CCommandEosGotoModel()  :
  CCommandEosGotoBase(), IEosCommandModel()
{
}

CCommandEosGotoModel::~CCommandEosGotoModel()
{
}

//----------------------------------------------------------------------------------------
//
tInstructionMapValue CCommandEosGotoModel::DefaultArgs()
{
  return ValuesFromTypes(ArgList());
}

//----------------------------------------------------------------------------------------
//
QString CCommandEosGotoModel::DisplayName(const tInstructionMapValue& args)
{
  const auto& itTarget = GetValue<EArgumentType::eString>(args, "target");
  const bool bHasTarget = HasValue(args, "target") && IsOk<EArgumentType::eString>(itTarget);

  if (bHasTarget)
  {
    return QString("<b>%1</b>: %2").arg(eos::c_sCommandGoto).arg(std::get<QString>(itTarget));
  }
  else
  {
    return QString("<b>%1</b>").arg(eos::c_sCommandGoto);
  }
}

//----------------------------------------------------------------------------------------
//
CCommandEosIfModel::CCommandEosIfModel()  :
  CCommandEosIfBase(), IEosCommandModel()
{
}

CCommandEosIfModel::~CCommandEosIfModel()
{
}

//----------------------------------------------------------------------------------------
//
tInstructionMapValue CCommandEosIfModel::DefaultArgs()
{
  return ValuesFromTypes(ArgList());
}

//----------------------------------------------------------------------------------------
//
QString CCommandEosIfModel::DisplayName(const tInstructionMapValue& args)
{
  const auto& itCondition = GetValue<EArgumentType::eString>(args, "condition");
  const bool bHasCondition = HasValue(args, "condition") && IsOk<EArgumentType::eString>(itCondition);

  if (bHasCondition)
  {
    return QString("<b>%1</b> (%2)").arg(eos::c_sCommandIf).arg(std::get<QString>(itCondition));
  }
  else
  {
    return QString("<b>%1</b>").arg(eos::c_sCommandIf);
  }
}

//----------------------------------------------------------------------------------------
//
void CCommandEosIfModel::InsertedChildAt(tInstructionMapValue* pArgs, qint32 iIndex,
                                         const QString& sIntoGroup,
                                         const QString& sInsertedChild)
{
  tChildNodeGroups groups = ChildNodeGroups(*pArgs);
  auto it = std::find_if(groups.begin(), groups.end(),
                      [&sIntoGroup](const std::tuple<QString, QString, qint32>& tuple) {
    return std::get<0>(tuple) == sIntoGroup;
  });

  if (groups.end() != it)
  {
    QString sAttr = std::get<1>(*it);
    tInstructionArrayValue arrCommands;

    const auto& itCommands = GetValue<EArgumentType::eArray>(*pArgs, sAttr);
    if (HasValue(*pArgs, sAttr) && IsOk<EArgumentType::eArray>(itCommands))
    {
      arrCommands = std::get<tInstructionArrayValue>(itCommands);
      arrCommands.insert(arrCommands.begin()+iIndex, { EArgumentType::eObject, sInsertedChild});
    }
    pArgs->insert_or_assign(sAttr,
                            SInstructionArgumentValue{ EArgumentType::eArray, arrCommands });
  }
}

//----------------------------------------------------------------------------------------
//
void CCommandEosIfModel::RemoveChildAt(tInstructionMapValue* pArgs, qint32 iIndex,
                                       const QString& sIntoGroup)
{
  tChildNodeGroups groups = ChildNodeGroups(*pArgs);
  auto it = std::find_if(groups.begin(), groups.end(),
                      [&sIntoGroup](const std::tuple<QString, QString, qint32>& tuple) {
    return std::get<0>(tuple) == sIntoGroup;
  });

  if (groups.end() != it)
  {
    QString sAttr = std::get<1>(*it);
    tInstructionArrayValue arrCommands;

    const auto& itCommands = GetValue<EArgumentType::eArray>(*pArgs, sAttr);
    if (HasValue(*pArgs, sAttr) && IsOk<EArgumentType::eArray>(itCommands))
    {
      arrCommands = std::get<tInstructionArrayValue>(itCommands);
      arrCommands.erase(arrCommands.begin()+iIndex);
    }
    pArgs->insert_or_assign(sAttr,
                            SInstructionArgumentValue{ EArgumentType::eArray, arrCommands });
  }
}

//----------------------------------------------------------------------------------------
//
CCommandEosImageModel::CCommandEosImageModel() :
  CCommandEosImageBase(), IEosCommandModel()
{
}

CCommandEosImageModel::~CCommandEosImageModel()
{
}

//----------------------------------------------------------------------------------------
//
tInstructionMapValue CCommandEosImageModel::DefaultArgs()
{
  return ValuesFromTypes(ArgList());
}

//----------------------------------------------------------------------------------------
//
QString CCommandEosImageModel::DisplayName(const tInstructionMapValue& args)
{
  const auto& itLocator = GetValue<EArgumentType::eString>(args, "locator");
  const bool bHasLocator = HasValue(args, "locator") && IsOk<EArgumentType::eString>(itLocator);

  if (bHasLocator)
  {
    return QString("<b>%1</b>: %2").arg(eos::c_sCommandImage).arg(std::get<QString>(itLocator));
  }
  else
  {
    return QString("<b>%1</b>").arg(eos::c_sCommandImage);
  }
}

//----------------------------------------------------------------------------------------
//
CCommandEosNoopModel::CCommandEosNoopModel() :
  CCommandEosNoopBase(), IEosCommandModel()
{
}

CCommandEosNoopModel::~CCommandEosNoopModel()
{
}

//----------------------------------------------------------------------------------------
//
tInstructionMapValue CCommandEosNoopModel::DefaultArgs()
{
  return ValuesFromTypes(ArgList());
}

//----------------------------------------------------------------------------------------
//
QString CCommandEosNoopModel::DisplayName(const tInstructionMapValue& args)
{
  return QString("<b>%1</b>").arg(eos::c_sCommandNoop);
}

//----------------------------------------------------------------------------------------
//
CCommandEosNotificationCreateModel::CCommandEosNotificationCreateModel() :
  CCommandEosNotificationCreateBase(), IEosCommandModel()
{
}

CCommandEosNotificationCreateModel::~CCommandEosNotificationCreateModel()
{
}

//----------------------------------------------------------------------------------------
//
tInstructionMapValue CCommandEosNotificationCreateModel::DefaultArgs()
{
  return ValuesFromTypes(ArgList());
}

//----------------------------------------------------------------------------------------
//
QString CCommandEosNotificationCreateModel::DisplayName(const tInstructionMapValue& args)
{
  const auto& itId = GetValue<EArgumentType::eString>(args, "id");
  const auto& itTitle = GetValue<EArgumentType::eString>(args, "title");
  const auto& itButtonLabel = GetValue<EArgumentType::eString>(args, "buttonLabel");

  const bool bHasId = HasValue(args, "id") && IsOk<EArgumentType::eString>(itId);
  const bool bHasTitle = HasValue(args, "title") && IsOk<EArgumentType::eString>(itTitle);
  const bool bHasButton = HasValue(args, "buttonLabel") && IsOk<EArgumentType::eString>(itButtonLabel);

  if (bHasId)
  {
    QString sTemplate = "<b>%1</b>: <i>id</i>:%2";
    sTemplate = sTemplate.arg(eos::c_sCommandNotificationCreate)
                         .arg(std::get<QString>(itId));
    if (bHasTitle || bHasButton) { sTemplate += " (%1)"; }
    if (bHasTitle && !bHasButton)
    {
      return sTemplate.arg(QString("<i>Title</i>:&quot;%1&quot;").arg(std::get<QString>(itTitle)));
    }
    else if (!bHasTitle && bHasButton)
    {
      return sTemplate.arg(QString("<i>Button</i>:&quot;%1&quot;").arg(std::get<QString>(itButtonLabel)));
    }
    else if (bHasTitle && bHasButton)
    {
      return sTemplate.arg(QString("<i>Title</i>:&quot;%1&quot;, <i>Button</i>:&quot;%2&quot;")
                           .arg(std::get<QString>(itTitle))
                           .arg(std::get<QString>(itButtonLabel)));
    }
    return sTemplate;
  }
  else
  {
    return QString("<b>%1</b>").arg(eos::c_sCommandNotificationCreate);
  }
}

//----------------------------------------------------------------------------------------
//
void CCommandEosNotificationCreateModel::InsertedChildAt(tInstructionMapValue* pArgs, qint32 iIndex,
                                                         const QString& sIntoGroup,
                                                         const QString& sInsertedChild)
{
  tChildNodeGroups groups = ChildNodeGroups(*pArgs);
  auto it = std::find_if(groups.begin(), groups.end(),
                      [&sIntoGroup](const std::tuple<QString, QString, qint32>& tuple) {
    return std::get<0>(tuple) == sIntoGroup;
  });

  qint32 iIndexOfGroup = it - groups.begin();
  if (groups.end() != it)
  {
    QString sAttr = std::get<1>(*it);
    tInstructionArrayValue arrCommands;

    qint32 iIndexToInsertAt = iIndex;
    for (size_t i = 0; groups.size() > static_cast<size_t>(i) && static_cast<size_t>(iIndexOfGroup) > i; ++i)
    {
      QString sAttrLoop = std::get<1>(groups[static_cast<size_t>(i)]);
      const auto& itCommandsLoop = GetValue<EArgumentType::eArray>(*pArgs, sAttrLoop);
      if (HasValue(*pArgs, sAttrLoop) && IsOk<EArgumentType::eArray>(itCommandsLoop))
      {
        iIndexToInsertAt -=
            static_cast<qint32>(std::get<tInstructionArrayValue>(itCommandsLoop).size());
      }
    }
    const auto& itCommands = GetValue<EArgumentType::eArray>(*pArgs, sAttr);
    if (HasValue(*pArgs, sAttr) && IsOk<EArgumentType::eArray>(itCommands))
    {
      arrCommands = std::get<tInstructionArrayValue>(itCommands);
      arrCommands.insert(arrCommands.begin()+iIndexToInsertAt,
                         { EArgumentType::eObject, sInsertedChild});
    }
    pArgs->insert_or_assign(sAttr,
                            SInstructionArgumentValue{ EArgumentType::eArray, arrCommands });
  }
}

//----------------------------------------------------------------------------------------
//
void CCommandEosNotificationCreateModel::RemoveChildAt(tInstructionMapValue* pArgs, qint32 iIndex,
                                                       const QString& sIntoGroup)
{
  tChildNodeGroups groups = ChildNodeGroups(*pArgs);
  auto it = std::find_if(groups.begin(), groups.end(),
                      [&sIntoGroup](const std::tuple<QString, QString, qint32>& tuple) {
    return std::get<0>(tuple) == sIntoGroup;
  });

  if (groups.end() != it)
  {
    QString sAttr = std::get<1>(*it);
    tInstructionArrayValue arrCommands;

    const auto& itCommands = GetValue<EArgumentType::eArray>(*pArgs, sAttr);
    if (HasValue(*pArgs, sAttr) && IsOk<EArgumentType::eArray>(itCommands))
    {
      arrCommands = std::get<tInstructionArrayValue>(itCommands);
      arrCommands.erase(arrCommands.begin()+iIndex);
    }
    pArgs->insert_or_assign(sAttr,
                            SInstructionArgumentValue{ EArgumentType::eArray, arrCommands });
  }
}

//----------------------------------------------------------------------------------------
//
CCommandEosNotificationCloseModel::CCommandEosNotificationCloseModel() :
  CCommandEosNotificationCloseBase(), IEosCommandModel()
{
}

CCommandEosNotificationCloseModel::~CCommandEosNotificationCloseModel()
{
}

//----------------------------------------------------------------------------------------
//
tInstructionMapValue CCommandEosNotificationCloseModel::DefaultArgs()
{
  return ValuesFromTypes(ArgList());
}

//----------------------------------------------------------------------------------------
//
QString CCommandEosNotificationCloseModel::DisplayName(const tInstructionMapValue& args)
{
  const auto& itId = GetValue<EArgumentType::eString>(args, "id");
  const bool bHasId = HasValue(args, "id") && IsOk<EArgumentType::eString>(itId);

  if (bHasId)
  {
    return QString("<b>%1</b>: <i>id</i>:%2").arg(eos::c_sCommandNotificationClose).arg(std::get<QString>(itId));
  }
  else
  {
    return QString("<b>%1</b>").arg(eos::c_sCommandNotificationClose);
  }
}

//----------------------------------------------------------------------------------------
//
CCommandEosPromptModel::CCommandEosPromptModel() :
  CCommandEosPromptBase(), IEosCommandModel()
{
}

CCommandEosPromptModel::~CCommandEosPromptModel()
{
}

//----------------------------------------------------------------------------------------
//
tInstructionMapValue CCommandEosPromptModel::DefaultArgs()
{
  return ValuesFromTypes(ArgList());
}

//----------------------------------------------------------------------------------------
//
QString CCommandEosPromptModel::DisplayName(const tInstructionMapValue& args)
{
  const auto& itVariable = GetValue<EArgumentType::eString>(args, "variable");
  const bool bHasVariable = HasValue(args, "variable") && IsOk<EArgumentType::eString>(itVariable);

  if (bHasVariable)
  {
    return QString("<b>%1</b>: %2 = <i>?</i>").arg(eos::c_sCommandPrompt).arg(std::get<QString>(itVariable));
  }
  else
  {
    return QString("<b>%1</b>").arg(eos::c_sCommandPrompt);
  }
}

//----------------------------------------------------------------------------------------
//
CCommandEosSayModel::CCommandEosSayModel() :
  CCommandEosSayBase(), IEosCommandModel()
{
}

CCommandEosSayModel::~CCommandEosSayModel()
{
}

//----------------------------------------------------------------------------------------
//
tInstructionMapValue CCommandEosSayModel::DefaultArgs()
{
  return ValuesFromTypes(ArgList());
}

//----------------------------------------------------------------------------------------
//
QString CCommandEosSayModel::DisplayName(const tInstructionMapValue& args)
{
  const auto& itLabel = GetValue<EArgumentType::eString>(args, "label");
  const bool bHasLabel = HasValue(args, "label") && IsOk<EArgumentType::eString>(itLabel);

  if (bHasLabel)
  {
    QString sLabel = std::get<QString>(itLabel);
    sLabel.replace("<p>", " ").replace("</p>", " ").replace("<br>", " ").replace("</br>", " ");
    return QString("<b>%1</b>: %2").arg(eos::c_sCommandSay)
        .arg(sLabel);
  }
  else
  {
    return QString("<b>%1</b>").arg(eos::c_sCommandSay);
  }
}

//----------------------------------------------------------------------------------------
//
CCommandEosTimerModel::CCommandEosTimerModel() :
  CCommandEosTimerBase(), IEosCommandModel()
{
}

CCommandEosTimerModel::~CCommandEosTimerModel()
{
}

//----------------------------------------------------------------------------------------
//
tInstructionMapValue CCommandEosTimerModel::DefaultArgs()
{
  return ValuesFromTypes(ArgList());
}

//----------------------------------------------------------------------------------------
//
QString CCommandEosTimerModel::DisplayName(const tInstructionMapValue& args)
{
  const auto& itDuration = GetValue<EArgumentType::eString>(args, "duration");
  const bool bHasDuration = HasValue(args, "label") && IsOk<EArgumentType::eString>(itDuration);

  if (bHasDuration)
  {
    return QString("<b>%1</b>: %2").arg(eos::c_sCommandTimer).arg(std::get<QString>(itDuration));
  }
  else
  {
    return QString("<b>%1</b>").arg(eos::c_sCommandTimer);
  }
}

//----------------------------------------------------------------------------------------
//
void CCommandEosTimerModel::InsertedChildAt(tInstructionMapValue* pArgs, qint32 iIndex,
                                            const QString& sIntoGroup,
                                            const QString& sInsertedChild)
{
  tChildNodeGroups groups = ChildNodeGroups(*pArgs);
  auto it = std::find_if(groups.begin(), groups.end(),
                      [&sIntoGroup](const std::tuple<QString, QString, qint32>& tuple) {
    return std::get<0>(tuple) == sIntoGroup;
  });

  if (groups.end() != it)
  {
    QString sAttr = std::get<1>(*it);
    tInstructionArrayValue arrCommands;

    const auto& itCommands = GetValue<EArgumentType::eArray>(*pArgs, sAttr);
    if (HasValue(*pArgs, sAttr) && IsOk<EArgumentType::eArray>(itCommands))
    {
      arrCommands = std::get<tInstructionArrayValue>(itCommands);
      arrCommands.insert(arrCommands.begin()+iIndex, { EArgumentType::eObject, sInsertedChild});
    }
    pArgs->insert_or_assign(sAttr,
                            SInstructionArgumentValue{ EArgumentType::eArray, arrCommands });
  }
}

//----------------------------------------------------------------------------------------
//
void CCommandEosTimerModel::RemoveChildAt(tInstructionMapValue* pArgs, qint32 iIndex,
                                          const QString& sIntoGroup)
{
  tChildNodeGroups groups = ChildNodeGroups(*pArgs);
  auto it = std::find_if(groups.begin(), groups.end(),
                      [&sIntoGroup](const std::tuple<QString, QString, qint32>& tuple) {
    return std::get<0>(tuple) == sIntoGroup;
  });

  if (groups.end() != it)
  {
    QString sAttr = std::get<1>(*it);
    tInstructionArrayValue arrCommands;

    const auto& itCommands = GetValue<EArgumentType::eArray>(*pArgs, sAttr);
    if (HasValue(*pArgs, sAttr) && IsOk<EArgumentType::eArray>(itCommands))
    {
      arrCommands = std::get<tInstructionArrayValue>(itCommands);
      arrCommands.erase(arrCommands.begin()+iIndex);
    }
    pArgs->insert_or_assign(sAttr,
                            SInstructionArgumentValue{ EArgumentType::eArray, arrCommands });
  }
}
