#ifndef CCOMMANDTEXT_H
#define CCOMMANDTEXT_H

#include "Systems/JSON/JsonInstructionBase.h"
#include "EditorTutorialOverlay.h"
#include <QPointer>

class CCommandText : public IJsonInstructionBase
{
public:
  CCommandText(QPointer<CEditorTutorialOverlay> pTutorialOverlay);
  ~CCommandText() override;

  tInstructionMapType& ArgList() override;
  IJsonInstructionBase::tRetVal Call(const tInstructionMapValue& instruction) override;

private:
  tInstructionMapType                     m_argTypes;
  QPointer<CEditorTutorialOverlay>        m_pTutorialOverlay;
};

#endif // CCOMMANDTEXT_H
