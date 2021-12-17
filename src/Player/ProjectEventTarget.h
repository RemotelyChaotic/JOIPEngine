#ifndef CPROJECTEVENTTARGET_H
#define CPROJECTEVENTTARGET_H

#include <QObject>
#include <QJSValue>
#include <QPointer>

#include <map>
#include <vector>

class CProjectEventWrapper : public QObject
{
  Q_OBJECT
  Q_PROPERTY(QString target MEMBER m_sTarget)
  Q_PROPERTY(QString type MEMBER m_sType)

public:
  Q_INVOKABLE CProjectEventWrapper(const QString& sType);

  QString m_sTarget;
  QString m_sType;
};

struct SEvent
{
  QString m_sTarget;
  QString m_sType;

  bool operator==(const SEvent& other)
  {
    return m_sTarget == other.m_sTarget && m_sType == other.m_sType;
  }
};

inline bool operator<(const SEvent& l, const SEvent& r)
{
  return (l.m_sTarget<r.m_sTarget ||
          (l.m_sTarget==r.m_sTarget && l.m_sType<r.m_sType));
}

//----------------------------------------------------------------------------------------
//
class CProjectEventTargetWrapper;
class CProjectEventCallbackRegistry : public QObject
{
  Q_OBJECT

public:
  explicit CProjectEventCallbackRegistry(QObject* pParent = nullptr);
  ~CProjectEventCallbackRegistry() override;

  void AddDispatchTarget(const SEvent& sType, QPointer<CProjectEventTargetWrapper> pWrapper);
  void AddEventListener(const SEvent& sType, QJSValue callback);
  void Clear();
  void HandleError(QJSValue& value);
  void RemoveDispatchTarget(const SEvent& sType);
  void RemoveEventListener(const SEvent& sType, QJSValue callback);

signals:
  void SignalError(QString sError, QtMsgType type);

public slots:
  void Dispatch(const QString& sTarget, const QString& sEvent);

private:
  std::map<SEvent, std::vector<QJSValue>> m_callbackMap;
  std::map<SEvent, QPointer<CProjectEventTargetWrapper>> m_dispatchTargetMap;
};

//----------------------------------------------------------------------------------------
//
class CProjectEventTargetWrapper : public QObject
{
  Q_OBJECT

public:
  explicit CProjectEventTargetWrapper(QObject* pParent = nullptr);
  ~CProjectEventTargetWrapper() override;

  virtual void Dispatched(const QString& sEvent);
  virtual QString EventTarget() { return QString(); }
  virtual void InitializeEventRegistry(std::weak_ptr<CProjectEventCallbackRegistry> wpRegistry);

public slots:
  void addEventListener(QString sType, QJSValue callback);
  void removeEventListener(QString sType, QJSValue callback);
  void dispatch(QJSValue event);

protected:
  std::weak_ptr<CProjectEventCallbackRegistry> m_wpRegistry;
};

#endif // CPROJECTEVENTTARGET_H
