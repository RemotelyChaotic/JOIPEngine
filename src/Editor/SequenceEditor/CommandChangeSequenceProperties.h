#ifndef CCOMMANDCHANGESEQUENCEPROPERTIES_H
#define CCOMMANDCHANGESEQUENCEPROPERTIES_H

#include "Systems/Sequence/Sequence.h"

#include <QPointer>
#include <QUndoCommand>
#include <QTimeEdit>

#include <optional>

class CSequencePropertiesOverlay;

class CCommandChangeSequenceProperties : public QUndoCommand
{
public:
  CCommandChangeSequenceProperties(QPointer<CSequencePropertiesOverlay> pOverlay,
                                   QPointer<QTimeEdit> pTimeEdit);
  ~CCommandChangeSequenceProperties();

  void undo() override;
  void redo() override;

  int id() const override;
  bool mergeWith(const QUndoCommand* pOther) override;

private:
  void DoUndoRedo(const SSequenceFile& file);

  QPointer<CSequencePropertiesOverlay> m_pOverlay;
  QPointer<QTimeEdit>                  m_pTimeEdit;
  SSequenceFile                        m_sequenceOld;
  SSequenceFile                        m_sequenceNew;
};

#endif // CCOMMANDCHANGESEQUENCEPROPERTIES_H
