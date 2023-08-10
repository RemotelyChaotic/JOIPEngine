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
  void SignalJobFinished(qint32 iProjId);
  void SignalJobStarted(qint32 iProjId);
  void SignalJobAdded(qint32 iNumJobs);
  void SignalProgressChanged(qint32 iProjId, qint32 iProgress);

public slots:
  void Initialize() override;
  void Deinitialize() override;

protected:
  void CreatedNewJob(const tspRunnableJob& spJob, const QVariantList& args);

  virtual void RunNextJobImpl(const QVariantList& args) = 0;

protected slots:
  virtual void SlotJobFinished(qint32 iProjId) = 0;
  virtual void SlotJobStarted(qint32 iProjId) = 0;

  void SlotClearQueue();
  void SlotRunNextJob();

protected:
  std::queue<std::pair<tspRunnableJob, QVariantList>>      m_vspJobs;
  tspRunnableJob                                           m_spCurrentJob;
  mutable QMutex                                           m_jobMutex;
  mutable QSemaphore                                       m_waitForFinishCounter;
};

#endif // CPROJECTJOBWORKER_H
