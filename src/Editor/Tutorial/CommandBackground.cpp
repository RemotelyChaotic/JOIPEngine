#include "CommandBackground.h"

CCommandBackground::CCommandBackground(QPointer<CEditorTutorialOverlay> pTutorialOverlay) :
  IJsonInstructionBase(),
  m_argTypes({{"showBg", SInstructionArgumentType{EArgumentType::eBool}},
              {"onCliCkAdvance", SInstructionArgumentType{EArgumentType::eBool}},
              {"mouseTransparent", SInstructionArgumentType{EArgumentType::eBool}}}),
  m_pTutorialOverlay(pTutorialOverlay)
{
}

CCommandBackground::~CCommandBackground()
{

}

//----------------------------------------------------------------------------------------
//
tInstructionMapType& CCommandBackground::ArgList()
{
  return m_argTypes;
}

//----------------------------------------------------------------------------------------
//
IJsonInstructionBase::tRetVal CCommandBackground::Call(const tInstructionMapValue& args)
{
  if (nullptr != m_pTutorialOverlay)
  {
    const auto& itShowBg = GetValue<EArgumentType::eBool>(args, "showBg");
    const auto& itClickToAdvance = GetValue<EArgumentType::eBool>(args, "onCliCkAdvance");
    const auto& itMouseTransparent = GetValue<EArgumentType::eBool>(args, "mouseTransparent");
    bool bIsVisible = m_pTutorialOverlay->isVisible();
    bool bShowRequested = false;
    bool bClickToAdvance = false;
    bool bMouseTransparent = false;
    if (HasValue(args, "showBg") && IsOk<EArgumentType::eBool>(itShowBg))
    {
      bShowRequested = std::get<bool>(itShowBg);
      if (bShowRequested && !bIsVisible)
      {
        m_pTutorialOverlay->Show();
      }
      else if (!bShowRequested && bIsVisible)
      {
        m_pTutorialOverlay->Hide();
      }
    }
    if (HasValue(args, "onCliCkAdvance") && IsOk<EArgumentType::eBool>(itClickToAdvance))
    {
      bClickToAdvance = std::get<bool>(itClickToAdvance);
      m_pTutorialOverlay->SetClickToAdvanceEnabled(bClickToAdvance);
    }
    if (HasValue(args, "mouseTransparent") && IsOk<EArgumentType::eBool>(itMouseTransparent))
    {
      bMouseTransparent = std::get<bool>(itMouseTransparent);
      m_pTutorialOverlay->SetMouseTransparecny(bMouseTransparent);
    }

    if ((bShowRequested && bIsVisible) || (!bShowRequested && !bIsVisible) || !bClickToAdvance)
    {
      m_pTutorialOverlay->SlotTriggerNextInstruction();
    }

    return std::true_type();
  }
  return std::true_type();
}
