#ifndef CCOMMANDHIGHLIGHT_H
#define CCOMMANDHIGHLIGHT_H

#include "Systems/JSON/JsonInstructionBase.h"
#include "EditorTutorialOverlay.h"
#include <QPointer>

class CCommandHighlight : public IJsonInstructionBase
{
public:
  CCommandHighlight(QPointer<CEditorTutorialOverlay> pTutorialOverlay);
  ~CCommandHighlight() override;

  tInstructionMapType& ArgList() override;
  IJsonInstructionBase::tRetVal Call(const tInstructionMapValue& args) override;

private:
  tInstructionMapType                     m_argTypes;
  QPointer<CEditorTutorialOverlay>        m_pTutorialOverlay;
};

#endif // CCOMMANDHIGHLIGHT_H
