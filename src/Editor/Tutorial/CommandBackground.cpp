#include "CommandBackground.h"

CCommandBackground::CCommandBackground(QPointer<CEditorTutorialOverlay> pTutorialOverlay) :
  IJsonInstructionBase(),
  m_argTypes({{"showBg", QVariant::Bool}, {"onCliCkAdvance", QVariant::Bool}}),
  m_pTutorialOverlay(pTutorialOverlay)
{
}

CCommandBackground::~CCommandBackground()
{

}

//----------------------------------------------------------------------------------------
//
const std::map<QString, QVariant::Type>& CCommandBackground::ArgList() const
{
  return m_argTypes;
}

//----------------------------------------------------------------------------------------
//
void CCommandBackground::Call(const QVariantMap& args)
{
  if (nullptr != m_pTutorialOverlay)
  {
    auto itShowBg = args.find("showBg");
    auto itClickToAdvance = args.find("onCliCkAdvance");
    bool bIsVisible = m_pTutorialOverlay->isVisible();
    bool bShowRequested = false;
    if (args.end() != itShowBg)
    {
      bShowRequested = itShowBg.value().toBool();
      if (bShowRequested && !bIsVisible)
      {
        m_pTutorialOverlay->Show();
      }
      else if (!bShowRequested && bIsVisible)
      {
        m_pTutorialOverlay->Hide();
      }
    }
    if (args.end() != itClickToAdvance)
    {
      m_pTutorialOverlay->SetClickToAdvanceEnabled(itClickToAdvance.value().toBool());
    }

    if ((bShowRequested && bIsVisible) || (!bShowRequested && !bIsVisible))
    {
      m_pTutorialOverlay->SlotTriggerNextInstruction();
    }
  }
}
