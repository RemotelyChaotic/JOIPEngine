#include "CommandClickFilter.h"

CCommandClickFilter::CCommandClickFilter(QPointer<CEditorTutorialOverlay> pTutorialOverlay) :
  IJsonInstructionBase(),
  m_argTypes({{"items", SInstructionArgumentType{EArgumentType::eArray,
                                                 MakeArgArray(EArgumentType::eString)}},
              {"triggerNext", SInstructionArgumentType{EArgumentType::eBool}}}),
  m_pTutorialOverlay(pTutorialOverlay)
{
}

CCommandClickFilter::~CCommandClickFilter()
{

}

//----------------------------------------------------------------------------------------
//
tInstructionMapType& CCommandClickFilter::ArgList()
{
  return m_argTypes;
}

//----------------------------------------------------------------------------------------
//
IJsonInstructionBase::tRetVal CCommandClickFilter::Call(const tInstructionMapValue& args)
{
  if (nullptr != m_pTutorialOverlay)
  {
    const auto& itClickableItems = GetValue<EArgumentType::eArray>(args, "items");
    const auto& itTriggerNext = GetValue<EArgumentType::eBool>(args, "triggerNext");
    bool bTriggerNext = true;
    if (HasValue(args, "triggerNext") && IsOk<EArgumentType::eBool>(itTriggerNext))
    {
      bTriggerNext = std::get<bool>(itTriggerNext);
    }
    if (HasValue(args, "items") && IsOk<EArgumentType::eArray>(itClickableItems))
    {
      QStringList vList;
      tInstructionArrayValue items = std::get<tInstructionArrayValue>(itClickableItems);
      for (size_t i = 0; items.size() > i; ++i)
      {
        const auto& itClickableItem = GetValue<EArgumentType::eString>(items, i);
        if (IsOk<EArgumentType::eString>(itClickableItem))
        {
          vList << std::get<QString>(itClickableItem);
        }
      }
      bool bOk = QMetaObject::invokeMethod(m_pTutorialOverlay, "SetClickFilterWidgets", Qt::QueuedConnection,
                                           Q_ARG(QStringList, vList),
                                           Q_ARG(bool, bTriggerNext));
      bOk &= QMetaObject::invokeMethod(m_pTutorialOverlay, "SlotTriggerNextInstruction", Qt::QueuedConnection);
      assert(bOk); Q_UNUSED(bOk);
    }
    return std::true_type();
  }
  return std::true_type();
}
