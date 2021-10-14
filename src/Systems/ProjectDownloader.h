#ifndef CPROJECTDOWNLOADER_H
#define CPROJECTDOWNLOADER_H

#include "ThreadedSystem.h"
#include "DLJobs/DownloadJobRegistry.h"
#include <QAtomicInt>
#include <QMutex>
#include <QSemaphore>
#include <memory>
#include <queue>

class CProjectDownloader : public CSystemBase
{
  Q_OBJECT
  Q_DISABLE_COPY(CProjectDownloader)

public:
  CProjectDownloader(const std::vector<SDownloadJobConfig>& vJobCfg);
  ~CProjectDownloader() override;

  void ClearQueue();
  void CreateNewDownloadJob(const QString& sHost, const QVariantList& args);
  bool HasRunningJobs() const;
  qint32 RunningJobsCount() const;
  void StopRunningJobs();
  void WaitForFinished();

signals:
  void SignalDownloadFinished();
  void SignalDownloadStarted();
  void SignalJobAdded(qint32 iNumJobs);
  void SignalProgressChanged(qint32 iProgress);

public slots:
  void Initialize() override;
  void Deinitialize() override;

private slots:
  void SlotDownloadFinished();
  void SlotDownloadStarted();
  void SlotClearQueue();
  void SlotRunNextJob();

private:
  std::queue<std::pair<tspDownloadJob, QVariantList>>      m_vspJobs;
  mutable QMutex                                           m_jobMutex;
  mutable QSemaphore                                       m_waitForFinishCounter;
  tspDownloadJob                                           m_spCurrentJob;
  std::vector<SDownloadJobConfig>                          m_vJobCfg;
};

#endif // CPROJECTDOWNLOADER_H
