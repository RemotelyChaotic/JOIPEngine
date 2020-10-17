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

  const std::map<QString, QVariant::Type>& ArgList() const override;
  void Call(const QVariantMap& args) override;

private:
  const std::map<QString, QVariant::Type> m_argTypes;
  QPointer<CEditorTutorialOverlay>        m_pTutorialOverlay;
};

#endif // CCOMMANDCLICKTRANSPARENCY_H
