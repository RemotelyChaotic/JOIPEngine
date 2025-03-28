#ifndef CPROJECTDOWNLOADER_H
#define CPROJECTDOWNLOADER_H

#include "ProjectJobWorker.h"
#include "DLJobs/DownloadJobRegistry.h"

class CProjectDownloader : public CProjectJobWorker
{
  Q_OBJECT
  Q_DISABLE_COPY(CProjectDownloader)

public:
  CProjectDownloader(const std::vector<SDownloadJobConfig>& vJobCfg);
  ~CProjectDownloader() override;

  void CreateNewDownloadJob(const QString& sHost, const QVariantList& args);

protected slots:
  void SlotJobFinished(qint32 iId);
  void SlotJobStarted(qint32 iId);

protected:
  void JobFinalizeImpl(tspRunnableJob spJob) override;
  void JobFinishedImpl(qint32 iId, tspRunnableJob spJob) override;
  void JobPreRunImpl(qint32 iId, tspRunnableJob spJob) override;
  void JobPostRunImpl(qint32 iId, bool bOk, tspRunnableJob spJob) override;
  void JobStartedImpl(qint32 iId, tspRunnableJob spJob) override;

private:
  std::vector<SDownloadJobConfig>                          m_vJobCfg;
  QMetaObject::Connection                                  m_finalizeConn;
};

#endif // CPROJECTDOWNLOADER_H
