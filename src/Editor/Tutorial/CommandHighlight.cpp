#include "CommandHighlight.h"

CCommandHighlight::CCommandHighlight(QPointer<CEditorTutorialOverlay> pTutorialOverlay) :
  IJsonInstructionBase(),
  m_argTypes({{"items", QVariant::StringList}, {"allwaysOnTop", QVariant::Bool}}),
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
  auto itAllwaysOnTop = instruction.find("allwaysOnTop");
  bool bAllwaysOnTop = false;
  if (instruction.end() != itAllwaysOnTop)
  {
    bAllwaysOnTop = itAllwaysOnTop.value().toBool();
  }
  auto it = instruction.find("items");
  if (instruction.end() != it)
  {
    m_pTutorialOverlay->SetHighlightedWidgets(it.value().toStringList(), bAllwaysOnTop);
    m_pTutorialOverlay->SlotTriggerNextInstruction();
  }
}
