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
        bool bOk = QMetaObject::invokeMethod(m_pTutorialOverlay, "Show", Qt::QueuedConnection);
        assert(bOk); Q_UNUSED(bOk);
      }
      else if (!bShowRequested && bIsVisible)
      {
        bool bOk = QMetaObject::invokeMethod(m_pTutorialOverlay, "Hide", Qt::QueuedConnection);
        assert(bOk); Q_UNUSED(bOk);
      }
    }
    if (HasValue(args, "onCliCkAdvance") && IsOk<EArgumentType::eBool>(itClickToAdvance))
    {
      bClickToAdvance = std::get<bool>(itClickToAdvance);
      bool bOk = QMetaObject::invokeMethod(m_pTutorialOverlay, "SetClickToAdvanceEnabled", Qt::QueuedConnection,
                                           Q_ARG(bool, bClickToAdvance));
      assert(bOk); Q_UNUSED(bOk);
    }
    if (HasValue(args, "mouseTransparent") && IsOk<EArgumentType::eBool>(itMouseTransparent))
    {
      bMouseTransparent = std::get<bool>(itMouseTransparent);
      bool bOk = QMetaObject::invokeMethod(m_pTutorialOverlay, "SetMouseTransparecny", Qt::QueuedConnection,
                                           Q_ARG(bool, bMouseTransparent));
      assert(bOk); Q_UNUSED(bOk);
    }

    if ((bShowRequested && bIsVisible) || (!bShowRequested && !bIsVisible) || !bClickToAdvance)
    {
      bool bOk = QMetaObject::invokeMethod(m_pTutorialOverlay, "SlotTriggerNextInstruction", Qt::QueuedConnection);
      assert(bOk); Q_UNUSED(bOk);
    }

    return std::true_type();
  }
  return std::true_type();
}
