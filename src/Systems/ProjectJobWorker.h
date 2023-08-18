#ifndef CPROJECTJOBWORKER_H
#define CPROJECTJOBWORKER_H

#include "ThreadedSystem.h"
#include "IRunnableJob.h"

#include <QAtomicInt>
#include <QMutex>
#include <QSemaphore>

#include <memory>
#include <queue>

//----------------------------------------------------------------------------------------
//
class CProjectJobWorker : public CSystemBase
{
  Q_OBJECT
  Q_DISABLE_COPY(CProjectJobWorker)

public:
  CProjectJobWorker();
  ~CProjectJobWorker() override;

  void ClearQueue();
  bool HasRunningJobs() const;
  qint32 RunningJobsCount() const;
  void StopRunningJobs();
  void WaitForFinished();

signals:
  void SignalJobFinished(qint32 iId);
  void SignalJobStarted(qint32 iId);
  void SignalJobAdded(qint32 iNumJobs);
  void SignalProgressChanged(qint32 iId, qint32 iProgress);

public slots:
  void Initialize() override;
  void Deinitialize() override;

protected:
  void CreatedNewJob(const tspRunnableJob& spJob, const QVariantList& args);

protected slots:
  void SlotClearQueue();
  void SlotRunNextJob();
  void SlotFinalizeJob();

protected:
  virtual void JobFinalizeImpl(tspRunnableJob spJob) = 0;
  virtual void JobFinishedImpl(qint32 iId, tspRunnableJob spJob) = 0;
  virtual void JobPreRunImpl(qint32 iId, tspRunnableJob spJob) = 0;
  virtual void JobPostRunImpl(qint32 iId, bool bOk, tspRunnableJob spJob) = 0;
  virtual void JobStartedImpl(qint32 iId, tspRunnableJob spJob) = 0;

  std::queue<std::pair<tspRunnableJob, QVariantList>>      m_vspJobs;
  tspRunnableJob                                           m_spCurrentJob;
  mutable QMutex                                           m_jobMutex;
  mutable QSemaphore                                       m_waitForFinishCounter;
};

#endif // CPROJECTJOBWORKER_H
