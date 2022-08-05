#include "CommandText.h"
#include "Enums.h"

CCommandText::CCommandText(QPointer<CEditorTutorialOverlay> pTutorialOverlay) :
  IJsonInstructionBase(),
  m_argTypes({{"posX", SInstructionArgumentType{EArgumentType::eDouble}},
              {"posY", SInstructionArgumentType{EArgumentType::eDouble}},
              {QString("text"), SInstructionArgumentType{EArgumentType::eString}},
              {"anchor", SInstructionArgumentType{EArgumentType::eString}},
              {"hideButtons", SInstructionArgumentType{EArgumentType::eBool}}}),
  m_pTutorialOverlay(pTutorialOverlay)
{
}
CCommandText::~CCommandText()
{

}

//----------------------------------------------------------------------------------------
//
tInstructionMapType& CCommandText::ArgList()
{
  return m_argTypes;
}

//----------------------------------------------------------------------------------------
//
IJsonInstructionBase::tRetVal CCommandText::Call(const tInstructionMapValue& args)
{
  double dPosX = 0.0;
  double dPosY = 0.0;
  QString sText;
  EAnchors anchor = EAnchors::eCenter;
  bool bHideButtons = false;

  const auto& itPosX = GetValue<EArgumentType::eDouble>(args, "posX");
  const auto& itPosY = GetValue<EArgumentType::eDouble>(args, "posY");
  const auto& itText = GetValue<EArgumentType::eString>(args, "text");
  const auto& itanchor = GetValue<EArgumentType::eString>(args, "anchor");
  const auto& itHideButtons = GetValue<EArgumentType::eBool>(args, "hideButtons");

  if (HasValue(args, "posX") && IsOk<EArgumentType::eDouble>(itPosX))
  {
    dPosX = std::get<double>(itPosX);
  }
  if (HasValue(args, "posY") && IsOk<EArgumentType::eDouble>(itPosY))
  {
    dPosY = std::get<double>(itPosY);
  }
  if (HasValue(args, "text") && IsOk<EArgumentType::eString>(itText))
  {
    sText = std::get<QString>(itText);
  }
  if (HasValue(args, "anchor") && IsOk<EArgumentType::eString>(itanchor))
  {
    anchor = EAnchors::_from_string(std::get<QString>(itanchor).toStdString().data());
  }
  if (HasValue(args, "hideButtons") && IsOk<EArgumentType::eBool>(itHideButtons))
  {
    bHideButtons = std::get<bool>(itHideButtons);
  }

  if (nullptr != m_pTutorialOverlay)
  {
    bool bOk = QMetaObject::invokeMethod(m_pTutorialOverlay, "ShowTutorialText", Qt::QueuedConnection,
                                         Q_ARG(int, anchor._to_integral()),
                                         Q_ARG(double, dPosX), Q_ARG(double, dPosY),
                                         Q_ARG(bool, bHideButtons), Q_ARG(QString, sText));
    assert(bOk); Q_UNUSED(bOk);
    return std::true_type();
  }
  return std::true_type();
}

//----------------------------------------------------------------------------------------
//
CCommandText::tChildNodeGroups CCommandText::ChildNodeGroups(const tInstructionMapValue&) const
{
  return {};
}
