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
void CProjectDownloader::SlotJobFinished(qint32 iProjId)
{
  IDownloadJob* pJob = dynamic_cast<IDownloadJob*>(sender());
  if (nullptr != pJob && !pJob->WasStopped())
  {
    Notifier()->SendNotification(QString("%1 Download").arg(pJob->JobType()),
                                 QString("Download of %1 finished.").arg(pJob->JobName()));
  }
  emit SignalJobFinished(iProjId);
}

//----------------------------------------------------------------------------------------
//
void CProjectDownloader::SlotJobStarted(qint32 iProjId)
{
  IDownloadJob* pJob = dynamic_cast<IDownloadJob*>(sender());
  if (nullptr != pJob)
  {
    Notifier()->SendNotification(QString("%1 Download").arg(pJob->JobType()),
                                 QString("%1 download started.").arg(pJob->JobName()));
  }
  emit SignalJobStarted(iProjId);
}

//----------------------------------------------------------------------------------------
//
void CProjectDownloader::RunNextJobImpl(const QVariantList& args)
{
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
}
