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

  const std::map<QString, QVariant::Type>& ArgList() const override;
  void Call(const QVariantMap& instruction) override;

private:
  const std::map<QString, QVariant::Type> m_argTypes;
  QPointer<CEditorTutorialOverlay>        m_pTutorialOverlay;
};

#endif // CCOMMANDTEXT_H
