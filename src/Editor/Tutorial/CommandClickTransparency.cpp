#include "CommandClickTransparency.h"

CCommandClickTransparency::CCommandClickTransparency(QPointer<CEditorTutorialOverlay> pTutorialOverlay) :
  IJsonInstructionBase(),
  m_argTypes({{"x", QVariant::Double}, {"y", QVariant::Double},
              {"x2", QVariant::Double}, {"y2", QVariant::Double}}),
  m_pTutorialOverlay(pTutorialOverlay)
{

}

CCommandClickTransparency::~CCommandClickTransparency()
{

}

//----------------------------------------------------------------------------------------
//
const std::map<QString, QVariant::Type>& CCommandClickTransparency::ArgList() const
{
  return m_argTypes;
}

//----------------------------------------------------------------------------------------
//
void CCommandClickTransparency::Call(const QVariantMap& instruction)
{

}
