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

  const std::map<QString, QVariant::Type>& ArgList() const override;
  void Call(const QVariantMap& instruction) override;

private:
  const std::map<QString, QVariant::Type> m_argTypes;
  QPointer<CEditorTutorialOverlay>        m_pTutorialOverlay;
};

#endif // CCOMMANDHIGHLIGHT_H
