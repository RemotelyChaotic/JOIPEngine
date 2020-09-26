#ifndef CCOMMANDCLICKTRANSPARENCY_H
#define CCOMMANDCLICKTRANSPARENCY_H

#include "Systems/JSON/JsonInstructionBase.h"
#include "EditorTutorialOverlay.h"
#include <QPointer>

class CCommandClickTransparency : public IJsonInstructionBase
{
public:
  CCommandClickTransparency(QPointer<CEditorTutorialOverlay> pTutorialOverlay);
  ~CCommandClickTransparency() override;

  const std::map<QString, QVariant::Type>& ArgList() const override;
  void Call(const QVariantMap& instruction) override;

private:
  const std::map<QString, QVariant::Type> m_argTypes;
  QPointer<CEditorTutorialOverlay>        m_pTutorialOverlay;
};

#endif // CCOMMANDCLICKTRANSPARENCY_H
