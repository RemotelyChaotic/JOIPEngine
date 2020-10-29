#include "CommandClickFilter.h"

CCommandClickFilter::CCommandClickFilter(QPointer<CEditorTutorialOverlay> pTutorialOverlay) :
  IJsonInstructionBase(),
  m_argTypes({{"items", QVariant::StringList}, {"triggerNext", QVariant::Bool}}),
  m_pTutorialOverlay(pTutorialOverlay)
{

}

CCommandClickFilter::~CCommandClickFilter()
{

}

//----------------------------------------------------------------------------------------
//
const std::map<QString, QVariant::Type>& CCommandClickFilter::ArgList() const
{
  return m_argTypes;
}

//----------------------------------------------------------------------------------------
//
void CCommandClickFilter::Call(const QVariantMap& args)
{
  if (nullptr != m_pTutorialOverlay)
  {
    auto itClickableItems = args.find("items");
    auto itTriggerNext = args.find("triggerNext");
    bool bTriggerNext = true;
    if (args.end() != itTriggerNext)
    {
      bTriggerNext = itTriggerNext.value().toBool();
    }
    if (args.end() != itClickableItems)
    {
      m_pTutorialOverlay->SetClickFilterWidgets(itClickableItems.value().toStringList(), bTriggerNext);
      m_pTutorialOverlay->SlotTriggerNextInstruction();
    }
  }
}
