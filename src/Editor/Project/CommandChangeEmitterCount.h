#ifndef CCOMMANDCHANGEEMITTERCOUNT_H
#define CCOMMANDCHANGEEMITTERCOUNT_H

#include "Systems/Database/Project.h"
#include <QPointer>
#include <QSpinBox>
#include <QUndoCommand>
#include <functional>

class CCommandChangeEmitterCount : public QUndoCommand
{
public:
  CCommandChangeEmitterCount(QPointer<QSpinBox> pEmitterCount,
                             const std::function<void(void)>& fnOnUndoRedo,
                             QUndoCommand* pParent = nullptr);
  ~CCommandChangeEmitterCount();

  void undo() override;
  void redo() override;

  int id() const override;
  bool mergeWith(const QUndoCommand* pOther) override;

protected:
  QPointer<QSpinBox> m_pEmitterCount;
  std::function<void(void)> m_fnOnUndoRedo;
  qint32             m_iOriginalValue;
  qint32             m_iNewValue;
};

#endif // CCOMMANDCHANGEEMITTERCOUNT_H
