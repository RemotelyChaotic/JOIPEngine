#ifndef CCOMMANDCLICKTRANSPARENCY_H
#define CCOMMANDCLICKTRANSPARENCY_H

#include "Systems/JSON/JsonInstructionBase.h"
#include "EditorTutorialOverlay.h"
#include <QPointer>

class CCommandClickFilter : public IJsonInstructionBase
{
public:
  CCommandClickFilter(QPointer<CEditorTutorialOverlay> pTutorialOverlay);
  ~CCommandClickFilter() override;

  tInstructionMapType& ArgList() override;
  IJsonInstructionBase::tRetVal Call(const tInstructionMapValue& args) override;

private:
  tInstructionMapType                     m_argTypes;
  QPointer<CEditorTutorialOverlay>        m_pTutorialOverlay;
};

#endif // CCOMMANDCLICKTRANSPARENCY_H
