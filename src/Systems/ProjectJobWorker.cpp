#include "ProjectJobWorker.h"

CProjectJobWorker::CProjectJobWorker() :
    CSystemBase(),
    m_vspJobs(),
    m_spCurrentJob(nullptr),
    m_jobMutex(QMutex::Recursive),
    m_waitForFinishCounter(1)
{
}
CProjectJobWorker::~CProjectJobWorker()
{

}

//----------------------------------------------------------------------------------------
//
void CProjectJobWorker::ClearQueue()
{
  bool bOk = QMetaObject::invokeMethod(this, "SlotClearQueue", Qt::QueuedConnection);
  Q_UNUSED(bOk);
  assert(bOk);
}

//----------------------------------------------------------------------------------------
//
bool CProjectJobWorker::HasRunningJobs() const
{
  QMutexLocker locker(&m_jobMutex);
  return m_spCurrentJob != nullptr || m_vspJobs.size() > 0;
}

//----------------------------------------------------------------------------------------
//
qint32 CProjectJobWorker::RunningJobsCount() const
{
  QMutexLocker locker(&m_jobMutex);
  return static_cast<qint32>(m_vspJobs.size()) + static_cast<qint32>(nullptr != m_spCurrentJob);
}

//----------------------------------------------------------------------------------------
//
void CProjectJobWorker::StopRunningJobs()
{
  QMutexLocker locker(&m_jobMutex);
  if (nullptr != m_spCurrentJob)
  {
    m_spCurrentJob->Stop();
  }
}

//----------------------------------------------------------------------------------------
//
void CProjectJobWorker::WaitForFinished()
{
  m_waitForFinishCounter.acquire(1);
  m_waitForFinishCounter.release(1);
}

//----------------------------------------------------------------------------------------
//
void CProjectJobWorker::Initialize()
{
  SlotClearQueue();
  StopRunningJobs();
  SetInitialized(true);
}

//----------------------------------------------------------------------------------------
//
void CProjectJobWorker::Deinitialize()
{
  SlotClearQueue();
  StopRunningJobs();
  SetInitialized(false);
}

//----------------------------------------------------------------------------------------
//
void CProjectJobWorker::CreatedNewJob(const tspRunnableJob& spJob, const QVariantList& args)
{
  if (!IsInitialized()) { return; }

  bool bIsJobRunning = false;
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
void CProjectJobWorker::SlotClearQueue()
{
  QMutexLocker locker(&m_jobMutex);
  while (m_vspJobs.size() > 0)
  {
    m_vspJobs.pop();
  }
}

//----------------------------------------------------------------------------------------
//
void CProjectJobWorker::SlotRunNextJob()
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

    JobPreRunImpl(m_spCurrentJob->Id(), m_spCurrentJob);

    bool bOk = m_spCurrentJob->Run(args);

    JobPostRunImpl(m_spCurrentJob->Id(), bOk, m_spCurrentJob);

    if (m_spCurrentJob->Finished())
    {
      JobFinishedImpl(m_spCurrentJob->Id(), m_spCurrentJob);
      SlotFinalizeJob();
    }

    m_jobMutex.lock();
  }
  m_jobMutex.unlock();
}

//----------------------------------------------------------------------------------------
//
void CProjectJobWorker::SlotFinalizeJob()
{
  JobFinalizeImpl(m_spCurrentJob);

  // reset current Job and delete object
  {
    QMutexLocker locker(&m_jobMutex);
    m_spCurrentJob = nullptr;
  }

  m_waitForFinishCounter.release(1);

  bool bOk = QMetaObject::invokeMethod(this, "SlotRunNextJob", Qt::QueuedConnection);
  Q_UNUSED(bOk);
  assert(bOk);
}
