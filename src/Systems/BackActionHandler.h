#ifndef CBACKACTIONHANDLER_H
#define CBACKACTIONHANDLER_H

#include <QObject>
#include <QPointer>

#include <optional>

class CBackActionHandler : public QObject
{
  Q_OBJECT

public:
  CBackActionHandler();
  ~CBackActionHandler() override;

  void ClearSlotToCall();
  void RegisterSlotToCall(QPointer<QObject> pObject, QString sSlot);

protected:
  bool eventFilter(QObject* pObj, QEvent* pEvt) override;

private:
  bool HandleBackAction();

  std::optional<std::pair<QPointer<QObject>, QString>> m_optSlotToCallOnBack;
};

#endif // CBACKACTIONHANDLER_H
