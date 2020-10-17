#include "CommandClickFilter.h"

CCommandClickFilter::CCommandClickFilter(QPointer<CEditorTutorialOverlay> pTutorialOverlay) :
  IJsonInstructionBase(),
  m_argTypes({{"items", QVariant::StringList}}),
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
    if (args.end() != itClickableItems)
    {
      m_pTutorialOverlay->SetClickFilterWidgets(itClickableItems.value().toStringList());
      m_pTutorialOverlay->SlotTriggerNextInstruction();
    }
  }
}
