#ifndef CCOMMANDCHANGEEMITTERCOUNT_H
#define CCOMMANDCHANGEEMITTERCOUNT_H

#include "Systems/Project.h"
#include <QPointer>
#include <QSpinBox>
#include <QUndoCommand>

class CCommandChangeEmitterCount : public QUndoCommand
{
public:
  CCommandChangeEmitterCount(QPointer<QSpinBox> pEmitterCount,
                             QUndoCommand* pParent = nullptr);
  ~CCommandChangeEmitterCount();

  void undo() override;
  void redo() override;

  int id() const override;
  bool mergeWith(const QUndoCommand* pOther) override;

protected:
  QPointer<QSpinBox> m_pEmitterCount;
  qint32             m_iOriginalValue;
  qint32             m_iNewValue;
};

#endif // CCOMMANDCHANGEEMITTERCOUNT_H
