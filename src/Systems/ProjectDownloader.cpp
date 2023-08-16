#include "ProjectDownloader.h"
#include "DLJobs/DownloadJobRegistry.h"
#include "Systems/NotificationSender.h"
#include <QDebug>

CProjectDownloader::CProjectDownloader(const std::vector<SDownloadJobConfig>& vJobCfg) :
  CProjectJobWorker(),
  m_vJobCfg(vJobCfg)
{
}

CProjectDownloader::~CProjectDownloader()
{
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

  tspDownloadJob spJob =
      std::shared_ptr<IDownloadJob>(CDownloadJobFactory::GetJob(it->m_sClassType));
  CProjectJobWorker::CreatedNewJob(spJob, args);
}

//----------------------------------------------------------------------------------------
//
void CProjectDownloader::JobFinishedImpl(qint32, tspRunnableJob spJob)
{
  if (nullptr != spJob)
  {
    if (!spJob->WasStopped())
    {
      if (spJob->HasError())
      {
        qWarning() << "Download could not finish properly: " << spJob->Error();
        Notifier()->SendNotification(QString("%1 Download").arg(spJob->JobType()),
                                     QString("Download of %1 could not finish properly:\n%2")
                                         .arg(spJob->JobName()).arg(spJob->Error()));
      }
      else
      {
        Notifier()->SendNotification(QString("%1 Download").arg(spJob->JobType()),
                                     QString("Download of %1 finished.").arg(spJob->JobName()));
      }
    }
    else
    {
      Notifier()->SendNotification(QString("%1 Download").arg(spJob->JobType()),
                                   "Download stopped.");
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CProjectDownloader::JobRunImpl(qint32 iId, bool bOk, tspRunnableJob spJob)
{
  if (!bOk)
  {
    if (!spJob->WasStopped())
    {
      qWarning() << "Download could not finish properly: " << spJob->Error();
      Notifier()->SendNotification(QString("%1 Download").arg(spJob->JobType()),
                                   QString("Download of %1 could not finish properly:\n%2")
                                       .arg(spJob->JobName()).arg(spJob->Error()));
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CProjectDownloader::JobStartedImpl(qint32, tspRunnableJob spJob)
{
  if (nullptr != spJob)
  {
    Notifier()->SendNotification(QString("%1 Download").arg(spJob->JobType()),
                                 QString("%1 download started.").arg(spJob->JobName()));
  }
}
