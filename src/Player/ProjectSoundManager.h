#ifndef CPROJECTSOUNDMANAGER_H
#define CPROJECTSOUNDMANAGER_H

#include "ProjectEventTarget.h"
#include <QPointer>
#include <map>
#include <memory>

struct SResource;

class CSoundInstanceMessageSender : public QObject
{
  Q_OBJECT

public:
  CSoundInstanceMessageSender();
  ~CSoundInstanceMessageSender() override;

signals:
  void SignalPlay(const QString& sId, const QString& sResource, qint32 iLoops, qint32 iStartAt);
  void SignalPause(const QString& sId);
  void SignalStop(const QString& sId);
  void SignalSeek(const QString& sId, double dTime);
  void SignalDestroy(const QString& sId);
};

//----------------------------------------------------------------------------------------
//
class CSoundInstanceWrapper : public CProjectEventTargetWrapper
{
  Q_OBJECT
  Q_DISABLE_COPY(CSoundInstanceWrapper)
  CSoundInstanceWrapper() = delete;

public:  
  explicit CSoundInstanceWrapper(QPointer<QJSEngine> pEngine,
                                 const std::shared_ptr<SResource>& spResource,
                                 const QString& sId,
                                 qint32 iLoops,
                                 qint32 iStartAt);
  ~CSoundInstanceWrapper() override;

  void Dispatched(const QString& sEvent) override;
  QString EventTarget() override;
  void InitializeEventRegistry(std::weak_ptr<CProjectEventCallbackRegistry> wpRegistry) override;

  std::unique_ptr<CSoundInstanceMessageSender> m_spMsgSender;

public slots:
  void play();
  void pause();
  void stop();
  void seek(double dTime);
  void setVolume(double dVal);
  void destroy();

private:
  std::shared_ptr<SResource> m_spResource;
  QPointer<QJSEngine>        m_pEngine;
  QString                    m_sId;
  qint32                     m_iLoops;
  qint32                     m_iStartAt;
};

//----------------------------------------------------------------------------------------
//
class CProjectSoundManager : public CProjectEventTargetWrapper
{
  Q_OBJECT
  Q_DISABLE_COPY(CProjectSoundManager)
  CProjectSoundManager() = delete;

  struct SSoundRegistryData
  {
    std::shared_ptr<SResource> m_spResource;
    qint32 m_iLoops;
    qint32 m_iStartAt;
  };

public:
  CProjectSoundManager(QPointer<QJSEngine> pEngine, QObject* pParent = nullptr);
  ~CProjectSoundManager() override;

  void Dispatched(const QString& sEvent) override;
  QString EventTarget() override;
  void Initalize(std::weak_ptr<CProjectEventCallbackRegistry> wpRegistry);

public slots:
  void clearRegistry();
  void deRregisterId(QString sId);
  QJSValue get(QString sSounId);
  void registerId(QString sId, QJSValue sound, qint32 iLoops, qint32 iStartAt);

signals:
  void signalPlay(QString sId, QString sResource, qint32 iLoops, qint32 iStartAt);
  void signalPause(const QString& sId);
  void signalStop(const QString& sId);
  void signalSeek(const QString& sId, double dTime);

private slots:
  void SlotDestroy(const QString& sId);

private:
  QPointer<QJSEngine>                   m_pEngine;
  std::map<QString, SSoundRegistryData> m_registry;
};

#endif // CPROJECTSOUNDMANAGER_H
