#include "CommandHighlight.h"

CCommandHighlight::CCommandHighlight(QPointer<CEditorTutorialOverlay> pTutorialOverlay) :
  IJsonInstructionBase(),
  m_argTypes({{"items", SInstructionArgumentType{EArgumentType::eArray,
                                                 MakeArgArray(EArgumentType::eString)}},
              {"allwaysOnTop", SInstructionArgumentType{EArgumentType::eBool}}}),
  m_pTutorialOverlay(pTutorialOverlay)
{
}

CCommandHighlight::~CCommandHighlight()
{

}

//----------------------------------------------------------------------------------------
//
tInstructionMapType& CCommandHighlight::ArgList()
{
  return m_argTypes;
}

//----------------------------------------------------------------------------------------
//
IJsonInstructionBase::tRetVal CCommandHighlight::Call(const tInstructionMapValue& args)
{
  const auto& itAllwaysOnTop = GetValue<EArgumentType::eBool>(args, "allwaysOnTop");
  const auto& itItems = GetValue<EArgumentType::eArray>(args, "items");
  bool bAllwaysOnTop = false;
  if (HasValue(args, "allwaysOnTop") && IsOk<EArgumentType::eBool>(itAllwaysOnTop))
  {
    bAllwaysOnTop = std::get<bool>(itAllwaysOnTop);
  }
  if (HasValue(args, "items") && IsOk<EArgumentType::eArray>(itItems))
  {
    QStringList vList;
    tInstructionArrayValue items = std::get<tInstructionArrayValue>(itItems);
    for (size_t i = 0; items.size() > i; ++i)
    {
      const auto& itClickableItem = GetValue<EArgumentType::eString>(items, i);
      if (IsOk<EArgumentType::eString>(itClickableItem))
      {
        vList << std::get<QString>(itClickableItem);
      }
    }
    m_pTutorialOverlay->SetHighlightedWidgets(vList, bAllwaysOnTop);
    m_pTutorialOverlay->SlotTriggerNextInstruction();
  }
  return std::true_type();
}
