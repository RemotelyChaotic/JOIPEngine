#include "CommandEosChoiceBase.h"
#include "EosCommands.h"

CCommandEosChoiceBase::CCommandEosChoiceBase() :
  m_argTypes({
    {"options", SInstructionArgumentType{EArgumentType::eArray,
             MakeArgArray(EArgumentType::eMap, tInstructionMapType{
                   {"label", SInstructionArgumentType{EArgumentType::eString}},
                   {"commands", SInstructionArgumentType{EArgumentType::eArray,
                          MakeArgArray(EArgumentType::eObject)}},
                   {"color", SInstructionArgumentType{EArgumentType::eString}},
                   {"visible", SInstructionArgumentType{EArgumentType::eBool}}
             })
    }},
  })
{}
CCommandEosChoiceBase::~CCommandEosChoiceBase()
{}

//----------------------------------------------------------------------------------------
//
tInstructionMapType& CCommandEosChoiceBase::ArgList()
{
  return m_argTypes;
}

//----------------------------------------------------------------------------------------
//
IJsonInstructionBase::tRetVal CCommandEosChoiceBase::Call(const tInstructionMapValue& args)
{
  Q_UNUSED(args)
  return SJsonException{"Not implemented call error.", "", eos::c_sCommandChoice, 0, 0};
}

//----------------------------------------------------------------------------------------
//
CCommandEosChoiceBase::tChildNodeGroups CCommandEosChoiceBase::ChildNodeGroups(const tInstructionMapValue& args) const
{
  tChildNodeGroups vsReturn;
  const auto& itOptions = GetValue<EArgumentType::eArray>(args, "options");
  if (HasValue(args, "options") && IsOk<EArgumentType::eArray>(itOptions))
  {
    QStringList vsOptions;
    std::map<qint32, qint32> vsOptionMapping;
    std::vector<qint32> viNumChildren;

    const tInstructionArrayValue& arrOptions = std::get<tInstructionArrayValue>(itOptions);
    for (size_t i = 0; arrOptions.size() > i; ++i)
    {
      const auto& itOption = GetValue<EArgumentType::eMap>(arrOptions, i);
      if (IsOk<EArgumentType::eMap>(itOption))
      {
        const tInstructionMapValue& optionsArg = std::get<tInstructionMapValue>(itOption);

        const auto& itLabel = GetValue<EArgumentType::eString>(optionsArg, "label");
        const auto& itCommands = GetValue<EArgumentType::eArray>(optionsArg, "commands");

        QString sLabel;
        if (HasValue(optionsArg, "label") && IsOk<EArgumentType::eString>(itLabel))
        {
          sLabel = std::get<QString>(itLabel);
        }
        if (HasValue(optionsArg, "commands") && IsOk<EArgumentType::eArray>(itCommands))
        {
          const tInstructionArrayValue& commands = std::get<tInstructionArrayValue>(itCommands);
          viNumChildren.push_back(static_cast<qint32>(commands.size()));
        }

        vsReturn.push_back({sLabel, viNumChildren.back()});
      }
    }
  }
  return vsReturn;
}
