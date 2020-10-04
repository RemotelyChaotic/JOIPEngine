#include "CommandText.h"
#include "Enums.h"

CCommandText::CCommandText(QPointer<CEditorTutorialOverlay> pTutorialOverlay) :
  IJsonInstructionBase(),
  m_argTypes({{"posX", QVariant::Double}, {"posY", QVariant::Double},
              {"text", QVariant::String}, {"anchor", QVariant::String}}),
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
  double dPosX = 0.0;
  double dPosY = 0.0;
  QString sText;
  EAnchors anchor = EAnchors::eCenter;
  for (auto it = instruction.begin(); instruction.end() != it; ++it)
  {
    if (it.key() == "posX")
    {
      dPosX = it.value().toDouble();
    }
    else if (it.key() == "posY")
    {
      dPosY = it.value().toDouble();
    }
    else if (it.key() == "text")
    {
      sText = it.value().toString();
    }
    else if (it.key() == "anchor")
    {
      anchor = EAnchors::_from_string(it.value().toString().toStdString().data());
    }
  }

  if (nullptr != m_pTutorialOverlay)
  {
    m_pTutorialOverlay->ShowTutorialText(anchor, dPosX, dPosY, sText);
  }
}
