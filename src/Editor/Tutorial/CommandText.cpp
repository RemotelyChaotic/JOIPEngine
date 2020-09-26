#include "CommandText.h"

CCommandText::CCommandText(QPointer<CEditorTutorialOverlay> pTutorialOverlay) :
  IJsonInstructionBase(),
  m_argTypes({{"centerX", QVariant::Double}, {"centerY", QVariant::Double},
              {"text", QVariant::String}}),
  m_pTutorialOverlay(pTutorialOverlay)
{

}
CCommandText::~CCommandText()
{

}

//----------------------------------------------------------------------------------------
//
const std::map<QString, QVariant::Type>& CCommandText::ArgList() const
{
  return m_argTypes;
}

//----------------------------------------------------------------------------------------
//
void CCommandText::Call(const QVariantMap& instruction)
{
  double dCenterX = 0.0;
  double dCenterY = 0.0;
  QString sText;
  for (auto it = instruction.begin(); instruction.end() != it; ++it)
  {
    if (it.key() == "centerX")
    {
      dCenterX = it.value().toDouble();
    }
    else if (it.key() == "centerY")
    {
      dCenterY = it.value().toDouble();
    }
    else if (it.key() == "text")
    {
      sText = it.value().toString();
    }
  }

  if (nullptr != m_pTutorialOverlay)
  {
    m_pTutorialOverlay->ShowTutorialText(dCenterX, dCenterY, sText);
  }
}
