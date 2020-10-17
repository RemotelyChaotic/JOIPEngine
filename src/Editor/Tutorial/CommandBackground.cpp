#include "CommandBackground.h"

CCommandBackground::CCommandBackground(QPointer<CEditorTutorialOverlay> pTutorialOverlay) :
  IJsonInstructionBase(),
  m_argTypes({{"showBg", QVariant::Bool}, {"onCliCkAdvance", QVariant::Bool}, {"mouseTransparent", QVariant::Bool}}),
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
    auto itMouseTransparent = args.find("mouseTransparent");
    bool bIsVisible = m_pTutorialOverlay->isVisible();
    bool bShowRequested = false;
    bool bClickToAdvance = false;
    bool bMouseTransparent = false;
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
      bClickToAdvance = itClickToAdvance.value().toBool();
      m_pTutorialOverlay->SetClickToAdvanceEnabled(bClickToAdvance);
    }
    if (args.end() != itMouseTransparent)
    {
      bMouseTransparent = itMouseTransparent.value().toBool();
      m_pTutorialOverlay->SetMouseTransparecny(bMouseTransparent);
    }

    if ((bShowRequested && bIsVisible) || (!bShowRequested && !bIsVisible) || !bClickToAdvance)
    {
      m_pTutorialOverlay->SlotTriggerNextInstruction();
    }
  }
}
