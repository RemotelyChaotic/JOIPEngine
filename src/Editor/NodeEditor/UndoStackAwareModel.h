#ifndef IUNDOSTACKAWAREMODEL_H
#define IUNDOSTACKAWAREMODEL_H

#include "Systems/Nodes/NodeModelBase.h"

#include <QPointer>
#include <QUndoStack>

class CUndoStackAwareModel
{
public:
  void SetUndoStack(QPointer<QUndoStack> pUndostack)
  {
    m_pUndoStack = pUndostack;
    OnUndoStackSet();
  }
  QPointer<QUndoStack> UndoStack() { return m_pUndoStack; }

  virtual void UndoRestore(const QJsonObject& obj)
  {
    if (auto pNModel = dynamic_cast<CNodeModelBase*>(this))
    {
      pNModel->restore(obj);
    }
  }

protected:
  virtual ~CUndoStackAwareModel() {}
  virtual void OnUndoStackSet() {}

  QPointer<QUndoStack> m_pUndoStack = nullptr;
};

#endif // IUNDOSTACKAWAREMODEL_H
