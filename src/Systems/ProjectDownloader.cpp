#include "ProjectDownloader.h"
#include "DLJobs/DownloadJobRegistry.h"
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
    connect(dynamic_cast<QObject*>(m_spCurrentJob.get()), SIGNAL(SignalFinished()),
            this, SIGNAL(SignalDownloadFinished()), Qt::DirectConnection);
    connect(dynamic_cast<QObject*>(m_spCurrentJob.get()), SIGNAL(SignalProgressChanged(qint32)),
            this, SIGNAL(SignalProgressChanged(qint32)), Qt::DirectConnection);
    connect(dynamic_cast<QObject*>(m_spCurrentJob.get()), SIGNAL(SignalStarted()),
            this, SIGNAL(SignalDownloadStarted()), Qt::DirectConnection);
    connect(dynamic_cast<QObject*>(m_spCurrentJob.get()), SIGNAL(SignalFinished()),
            this, SLOT(SlotRunNextJob()), Qt::QueuedConnection);

    m_spCurrentJob->Run(args);
    if (!m_spCurrentJob->Finished())
    {
      qWarning() << "Download could not finish properly with args: " << args;
    }

    // reset current Job and delete object
    {
      QMutexLocker locker(&m_jobMutex);
      m_spCurrentJob = nullptr;
    }

    m_waitForFinishCounter.release(1);
  }
  m_jobMutex.unlock();
}
