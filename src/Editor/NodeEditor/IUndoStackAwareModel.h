#ifndef IUNDOSTACKAWAREMODEL_H
#define IUNDOSTACKAWAREMODEL_H

#include <QPointer>
#include <QUndoStack>

class IUndoStackAwareModel
{
public:
  void SetUndoStack(QPointer<QUndoStack> pUndostack)
  {
    m_pUndoStack = pUndostack;
    OnUndoStackSet();
  }
  QPointer<QUndoStack> UndoStack() { return m_pUndoStack; }

  virtual void UndoRestore(const QJsonObject& obj) {}

protected:
  virtual ~IUndoStackAwareModel() {}
  virtual void OnUndoStackSet() {}

  QPointer<QUndoStack> m_pUndoStack = nullptr;
};

#endif // IUNDOSTACKAWAREMODEL_H
