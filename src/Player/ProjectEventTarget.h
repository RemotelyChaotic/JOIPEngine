#ifndef CPROJECTEVENTTARGET_H
#define CPROJECTEVENTTARGET_H

#include <QObject>
#include <QJSValue>

#include <map>
#include <vector>

class CProjectEventWrapper : public QObject
{
  Q_OBJECT

public:
  Q_INVOKABLE CProjectEventWrapper(const QString& sType);

  QString m_sType;
};

//----------------------------------------------------------------------------------------
//
class CProjectEventCallbackRegistry : public QObject
{
  Q_OBJECT

public:
  explicit CProjectEventCallbackRegistry(QObject* pParent = nullptr);
  ~CProjectEventCallbackRegistry() override;

  void AddEventListener(QString sType, QJSValue callback);
  void Clear();
  void Dispatch(const QString& sEvent);
  void HandleError(QJSValue& value);
  void RemoveEventListener(QString sType, QJSValue callback);

signals:
  void SignalError(QString sError, QtMsgType type);

public slots:
  void SlotSceneChanged();

private:
  std::map<QString, std::vector<QJSValue>> m_callbackMap;
};

//----------------------------------------------------------------------------------------
//
class CProjectEventTargetWrapper : public QObject
{
  Q_OBJECT

public:
  explicit CProjectEventTargetWrapper(QObject* pParent = nullptr);
  ~CProjectEventTargetWrapper() override;

  void InitializeEventRegistry(std::weak_ptr<CProjectEventCallbackRegistry> wpRegistry);

public slots:
  void addEventListener(QString sType, QJSValue callback);
  void removeEventListener(QString sType, QJSValue callback);
  void dispatch(QJSValue event);

private:
  std::weak_ptr<CProjectEventCallbackRegistry> m_wpRegistry;
};

#endif // CPROJECTEVENTTARGET_H
