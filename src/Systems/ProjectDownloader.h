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

private slots:
  void SlotJobFinished(qint32 iProjId) override;
  void SlotJobStarted(qint32 iProjId) override;

private:
  void RunNextJobImpl(const QVariantList& args) override;

  std::vector<SDownloadJobConfig>                          m_vJobCfg;
};

#endif // CPROJECTDOWNLOADER_H
