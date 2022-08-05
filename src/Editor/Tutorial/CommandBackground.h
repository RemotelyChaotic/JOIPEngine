#ifndef CTUTORIALCOMMANDBACKGROUND_H
#define CTUTORIALCOMMANDBACKGROUND_H

#include "Systems/JSON/JsonInstructionBase.h"
#include "EditorTutorialOverlay.h"
#include <QPointer>

class CCommandBackground : public IJsonInstructionBase
{
public:
  CCommandBackground(QPointer<CEditorTutorialOverlay> pTutorialOverlay);
  ~CCommandBackground() override;

  tInstructionMapType& ArgList() override;
  IJsonInstructionBase::tRetVal Call(const tInstructionMapValue& args) override;
  tChildNodeGroups ChildNodeGroups(const tInstructionMapValue&) const override;

private:
  tInstructionMapType               m_argTypes;
  QPointer<CEditorTutorialOverlay>  m_pTutorialOverlay;
};

#endif // CTUTORIALCOMMANDBACKGROUND_H
