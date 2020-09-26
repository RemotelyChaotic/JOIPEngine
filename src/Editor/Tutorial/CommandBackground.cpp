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
    if (args.end() != itShowBg)
    {
      if (itShowBg.value().toBool())
      {
        m_pTutorialOverlay->Show();
      }
      else
      {
        m_pTutorialOverlay->Hide();
      }
    }
    if (args.end() != itClickToAdvance)
    {
      m_pTutorialOverlay->SetClickToAdvanceEnabled(itClickToAdvance.value().toBool());
      m_pTutorialOverlay->SlotTriggerNextInstruction();
    }
  }
}
