#include "ProjectDownloader.h"
#include "DLJobs/DownloadJobRegistry.h"
#include "Systems/NotificationSender.h"
#include <QDebug>

CProjectDownloader::CProjectDownloader(const std::vector<SDownloadJobConfig>& vJobCfg) :
  CSystemBase(),
  m_vspJobs(),
  m_jobMutex(QMutex::Recursive),
  m_waitForFinishCounter(1),
  m_spCurrentJob(nullptr),
  m_vJobCfg(vJobCfg)
{

}

CProjectDownloader::~CProjectDownloader()
{

}

//----------------------------------------------------------------------------------------
//
void CProjectDownloader::ClearQueue()
{
  bool bOk = QMetaObject::invokeMethod(this, "SlotClearQueue", Qt::QueuedConnection);
  Q_UNUSED(bOk);
  assert(bOk);
}

//----------------------------------------------------------------------------------------
//
void CProjectDownloader::CreateNewDownloadJob(const QString& sHost, const QVariantList& args)
{
  if (!IsInitialized()) { return; }

  auto it = std::find_if(m_vJobCfg.begin(), m_vJobCfg.end(),
                         [&sHost](const SDownloadJobConfig& cfg) {
    for (const QString& sHostCfg : qAsConst(cfg.m_vsAllowedHosts))
    {
      if (sHostCfg == sHost) { return true; }
    }
    return false;
  });
  if (m_vJobCfg.end() == it)
  {
    return;
  }

  bool bIsJobRunning = false;
  tspDownloadJob spJob =
      std::shared_ptr<IDownloadJob>(CDownloadJobFactory::GetJob(it->m_sClassType));
  {
    QMutexLocker locker(&m_jobMutex);
    m_vspJobs.push({spJob, args});
    bIsJobRunning = nullptr != m_spCurrentJob;
    emit SignalJobAdded(static_cast<qint32>(m_vspJobs.size()) + static_cast<qint32>(bIsJobRunning));
  }

  if (!bIsJobRunning)
  {
    bool bOk = QMetaObject::invokeMethod(this, "SlotRunNextJob", Qt::QueuedConnection);
    Q_UNUSED(bOk);
    assert(bOk);
  }
}

//----------------------------------------------------------------------------------------
//
bool CProjectDownloader::HasRunningJobs() const
{
  QMutexLocker locker(&m_jobMutex);
  return m_spCurrentJob != nullptr || m_vspJobs.size() > 0;
}

//----------------------------------------------------------------------------------------
//
qint32 CProjectDownloader::RunningJobsCount() const
{
  QMutexLocker locker(&m_jobMutex);
  return static_cast<qint32>(m_vspJobs.size()) + static_cast<qint32>(nullptr != m_spCurrentJob);
}

//----------------------------------------------------------------------------------------
//
void CProjectDownloader::StopRunningJobs()
{
  QMutexLocker locker(&m_jobMutex);
  if (nullptr != m_spCurrentJob)
  {
    m_spCurrentJob->Stop();
  }
}

//----------------------------------------------------------------------------------------
//
void CProjectDownloader::WaitForFinished()
{
  m_waitForFinishCounter.acquire(1);
  m_waitForFinishCounter.release(1);
}

//----------------------------------------------------------------------------------------
//
void CProjectDownloader::Initialize()
{
  SlotClearQueue();
  StopRunningJobs();
  SetInitialized(true);
}

//----------------------------------------------------------------------------------------
//
void CProjectDownloader::Deinitialize()
{
  SlotClearQueue();
  StopRunningJobs();
  SetInitialized(false);
}

//----------------------------------------------------------------------------------------
//
void CProjectDownloader::SlotDownloadFinished(qint32 iProjId)
{
  IDownloadJob* pJob = dynamic_cast<IDownloadJob*>(sender());
  if (nullptr != pJob && !pJob->WasStopped())
  {
    Notifier()->SendNotification(QString("%1 Download").arg(pJob->JobType()),
                                 QString("Download of %1 finished.").arg(pJob->JobName()));
  }
  emit SignalDownloadFinished(iProjId);
}

//----------------------------------------------------------------------------------------
//
void CProjectDownloader::SlotDownloadStarted(qint32 iProjId)
{
  IDownloadJob* pJob = dynamic_cast<IDownloadJob*>(sender());
  if (nullptr != pJob)
  {
    Notifier()->SendNotification(QString("%1 Download").arg(pJob->JobType()),
                                 QString("%1 download started.").arg(pJob->JobName()));
  }
  emit SignalDownloadStarted(iProjId);
}

//----------------------------------------------------------------------------------------
//
void CProjectDownloader::SlotClearQueue()
{
  QMutexLocker locker(&m_jobMutex);
  while (m_vspJobs.size() > 0)
  {
    m_vspJobs.pop();
  }
}

//----------------------------------------------------------------------------------------
//
void CProjectDownloader::SlotRunNextJob()
{
  m_jobMutex.lock();
  if (m_vspJobs.size() > 0 && IsInitialized())
  {
    m_jobMutex.unlock();
    m_waitForFinishCounter.acquire(1);

    // get next job
    QVariantList args;
    {
      QMutexLocker locker(&m_jobMutex);
      m_spCurrentJob = m_vspJobs.front().first;
      args = m_vspJobs.front().second;
      m_vspJobs.pop();
    }

    // connections (will be removed once the object is deleted, so we won't disconnect
    connect(dynamic_cast<QObject*>(m_spCurrentJob.get()), SIGNAL(SignalFinished(qint32)),
            this, SLOT(SlotDownloadFinished(qint32)), Qt::DirectConnection);
    connect(dynamic_cast<QObject*>(m_spCurrentJob.get()), SIGNAL(SignalProgressChanged(qint32,qint32)),
            this, SIGNAL(SignalProgressChanged(qint32,qint32)), Qt::DirectConnection);
    connect(dynamic_cast<QObject*>(m_spCurrentJob.get()), SIGNAL(SignalStarted(qint32)),
            this, SLOT(SlotDownloadStarted(qint32)), Qt::DirectConnection);
    connect(dynamic_cast<QObject*>(m_spCurrentJob.get()), SIGNAL(SignalFinished(qint32)),
            this, SLOT(SlotRunNextJob()), Qt::QueuedConnection);

    bool bOk = m_spCurrentJob->Run(args);
    if (!m_spCurrentJob->Finished() || !bOk)
    {
      if (!m_spCurrentJob->WasStopped())
      {
        qWarning() << "Download could not finish properly: " << m_spCurrentJob->Error();
        Notifier()->SendNotification(QString("%1 Download").arg(m_spCurrentJob->JobType()),
                                     QString("Download of %1 could not finish properly:\n%2")
                                     .arg(m_spCurrentJob->JobName()).arg(m_spCurrentJob->Error()));
      }
      else
      {
        Notifier()->SendNotification(QString("%1 Download").arg(m_spCurrentJob->JobType()),
                                     "Download stopped.");
      }
    }

    // reset current Job and delete object
    {
      QMutexLocker locker(&m_jobMutex);
      m_spCurrentJob = nullptr;
    }

    m_waitForFinishCounter.release(1);
    m_jobMutex.lock();
  }
  m_jobMutex.unlock();
}
