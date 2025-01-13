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
  if (QObject* pObj = dynamic_cast<QObject*>(spJob.get()))
  {
    pObj->moveToThread(thread());
  }
  CProjectJobWorker::CreatedNewJob(spJob, args);
}

//----------------------------------------------------------------------------------------
//
void CProjectDownloader::SlotJobFinished(qint32 iId)
{
  IRunnableJob* pJob = dynamic_cast<IRunnableJob*>(sender());
  if (nullptr != pJob)
  {
    JobFinishedImpl(iId, pJob->shared_from_this());
  }
}

//----------------------------------------------------------------------------------------
//
void CProjectDownloader::SlotJobStarted(qint32 iId)
{
  IRunnableJob* pJob = dynamic_cast<IRunnableJob*>(sender());
  if (nullptr != pJob)
  {
    JobStartedImpl(iId, pJob->shared_from_this());
  }
}

//----------------------------------------------------------------------------------------
//
void CProjectDownloader::JobFinalizeImpl(tspRunnableJob spJob)
{
  Q_UNUSED(spJob)
  disconnect(m_finalizeConn);
}

//----------------------------------------------------------------------------------------
//
void CProjectDownloader::JobFinishedImpl(qint32 iId, tspRunnableJob spJob)
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

    emit SignalJobFinished(iId);
  }
}

//----------------------------------------------------------------------------------------
//
void CProjectDownloader::JobPreRunImpl(qint32 iId, tspRunnableJob spJob)
{
  Q_UNUSED(iId)
  Q_UNUSED(spJob)
  // connections (will be removed once the object is deleted, so we won't disconnect
  connect(dynamic_cast<QObject*>(m_spCurrentJob.get()), SIGNAL(SignalFinished(qint32)),
          this, SLOT(SlotJobFinished(qint32)), Qt::DirectConnection);

  m_finalizeConn =
      connect(dynamic_cast<QObject*>(m_spCurrentJob.get()), SIGNAL(SignalFinished(qint32)),
              this, SLOT(SlotFinalizeJob()), Qt::DirectConnection);

  connect(dynamic_cast<QObject*>(m_spCurrentJob.get()), SIGNAL(SignalProgressChanged(qint32,qint32)),
          this, SIGNAL(SignalProgressChanged(qint32,qint32)), Qt::DirectConnection);
  connect(dynamic_cast<QObject*>(m_spCurrentJob.get()), SIGNAL(SignalStarted(qint32)),
          this, SLOT(SlotJobStarted(qint32)), Qt::DirectConnection);
}

//----------------------------------------------------------------------------------------
//
void CProjectDownloader::JobPostRunImpl(qint32 iId, bool bOk, tspRunnableJob spJob)
{
  Q_UNUSED(iId)
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
void CProjectDownloader::JobStartedImpl(qint32 iId, tspRunnableJob spJob)
{
  if (nullptr != spJob)
  {
    Notifier()->SendNotification(QString("%1 Download").arg(spJob->JobType()),
                                 QString("%1 download started.").arg(spJob->JobName()));
    emit SignalJobStarted(iId);
  }
}
