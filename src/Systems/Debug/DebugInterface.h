#ifndef CDEBUGINTERFACE_H
#define CDEBUGINTERFACE_H

#include <QJSEngine>
#include <QObject>
#include <QPointer>

class CDebugInterface : public QObject
{
  Q_OBJECT

public:
  explicit CDebugInterface(QObject* pParent = nullptr);
  ~CDebugInterface() override;

  void ResetEngine();
  void Register(const QString& objectName, QObject* pObj);
  QString TryEval(const QString& sCode);
  void UnRegister(const QString& objectName);

private:
  void DeleteEngine();

  QPointer<QJSEngine> m_spOwnEngine;
  std::map<QString, std::pair<QPointer<QJSEngine>, QPointer<QObject>>> m_registeredObjects;
};

#endif // CDEBUGINTERFACE_H
