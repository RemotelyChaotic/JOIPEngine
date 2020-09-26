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

  const std::map<QString, QVariant::Type>& ArgList() const override;
  void Call(const QVariantMap& args) override;

private:
  const std::map<QString, QVariant::Type> m_argTypes;
  QPointer<CEditorTutorialOverlay>        m_pTutorialOverlay;
};

#endif // CTUTORIALCOMMANDBACKGROUND_H
