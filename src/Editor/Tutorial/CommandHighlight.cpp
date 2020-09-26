#include "CommandHighlight.h"

CCommandHighlight::CCommandHighlight(QPointer<CEditorTutorialOverlay> pTutorialOverlay) :
  IJsonInstructionBase(),
  m_argTypes({{"items", QVariant::StringList}}),
  m_pTutorialOverlay(pTutorialOverlay)
{
}

CCommandHighlight::~CCommandHighlight()
{

}

//----------------------------------------------------------------------------------------
//
const std::map<QString, QVariant::Type>& CCommandHighlight::ArgList() const
{
  return m_argTypes;
}

//----------------------------------------------------------------------------------------
//
void CCommandHighlight::Call(const QVariantMap& instruction)
{
  auto it = instruction.find("items");
  if (instruction.end() != it)
  {
    m_pTutorialOverlay->SetHighlightedWidgets(it.value().toStringList());
    m_pTutorialOverlay->SlotTriggerNextInstruction();
  }
}
