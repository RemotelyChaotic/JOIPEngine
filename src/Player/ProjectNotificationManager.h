#ifndef CPROJECTNOTIFICATIONMANAGER_H
#define CPROJECTNOTIFICATIONMANAGER_H

#include "ProjectEventTarget.h"
#include <QPointer>
#include <map>
#include <memory>

//----------------------------------------------------------------------------------------
//
class CNotificationInstanceMessageSender : public QObject
{
  Q_OBJECT

public:
  CNotificationInstanceMessageSender();
  ~CNotificationInstanceMessageSender() override;

signals:
  void SignalRemove(const QString& sId);
  void SignalSetTitle(const QString& sId, const QString& sTitle);
};

//----------------------------------------------------------------------------------------
//
class CNotificationInstanceWrapper : public CProjectEventTargetWrapper
{
  Q_OBJECT
  Q_DISABLE_COPY(CNotificationInstanceWrapper)
  CNotificationInstanceWrapper() = delete;

public:
  explicit CNotificationInstanceWrapper(QPointer<QJSEngine> pEngine,
                                        const QString& sId);
  ~CNotificationInstanceWrapper() override;

  void Dispatched(const QString& sEvent) override;
  QString EventTarget() override;
  void InitializeEventRegistry(std::weak_ptr<CProjectEventCallbackRegistry> wpRegistry) override;

  std::unique_ptr<CNotificationInstanceMessageSender> m_spMsgSender;

public slots:
  void setTitle(const QString& sTitle);
  void remove();

private:
  QPointer<QJSEngine>        m_pEngine;
  QString                    m_sId;
};

//----------------------------------------------------------------------------------------
//
class CProjectNotificationManager : public CProjectEventTargetWrapper
{
  Q_OBJECT
  Q_DISABLE_COPY(CProjectNotificationManager)
  CProjectNotificationManager() = delete;

public:
  explicit CProjectNotificationManager(QPointer<QJSEngine> pEngine, QObject* pParent = nullptr);
  ~CProjectNotificationManager();

  void Dispatched(const QString& sEvent) override;
  QString EventTarget() override;
  void Initalize(std::weak_ptr<CProjectEventCallbackRegistry> wpRegistry);

public slots:
  void clearRegistry();
  void deRregisterId(QString sId);
  QJSValue get(QString sSounId);
  void registerId(QString sId);

signals:
  void signalRemove(const QString& sId);
  void signalSetTitle(const QString& sId, const QString& sTitle);

private slots:
  void SlotDestroy(const QString& sId);

private:
  QPointer<QJSEngine>        m_pEngine;
  std::map<QString, QString> m_registry;
};

#endif // CPROJECTNOTIFICATIONMANAGER_H
