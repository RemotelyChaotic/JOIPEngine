#include "EditorJobWorker.h"

CEditorJobWorker::CEditorJobWorker() :
  CProjectJobWorker(),
  m_iLastId(0)
{
}
CEditorJobWorker::~CEditorJobWorker()
{
}

//----------------------------------------------------------------------------------------
//
qint32 CEditorJobWorker::GenerateNewId()
{
  QMutexLocker locker(&m_idMutex);
  qint32 iId = m_iLastId;
  m_iLastId++;
  return iId;
}

//----------------------------------------------------------------------------------------
//
void CEditorJobWorker::SlotJobFinished(qint32 iId)
{
  IRunnableJob* pJob = dynamic_cast<IRunnableJob*>(sender());
  if (nullptr != pJob && !pJob->WasStopped())
  {
    /*
    Notifier()->SendNotification(QString("%1 Download").arg(pJob->JobType()),
                                 QString("Download of %1 finished.").arg(pJob->JobName()));
    */
  }

  emit SignalJobFinished(iId);
}

//----------------------------------------------------------------------------------------
//
void CEditorJobWorker::SlotJobStarted(qint32 iId)
{
  IRunnableJob* pJob = dynamic_cast<IRunnableJob*>(sender());
  if (nullptr != pJob && !pJob->WasStopped())
  {
    /*
    Notifier()->SendNotification(QString("%1 Download").arg(pJob->JobType()),
                                 QString("%1 download started.").arg(pJob->JobName()));
    */
  }
  emit SignalJobStarted(iId);
}

//----------------------------------------------------------------------------------------
//
void CEditorJobWorker::RunNextJobImpl(const QVariantList& args)
{
  bool bOkConn = connect(dynamic_cast<QObject*>(m_spCurrentJob.get()), SIGNAL(SignalJobMessage(qint32,QString)),
          this, SLOT(SignalJobMessage(qint32,QString)), Qt::QueuedConnection);
  assert(bOkConn); Q_UNUSED(bOkConn);

  bool bOk = m_spCurrentJob->Run(args);
  if (!m_spCurrentJob->Finished() || !bOk)
  {
    if (!m_spCurrentJob->WasStopped())
    {
      /*
      qWarning() << "Download could not finish properly: " << m_spCurrentJob->Error();
      Notifier()->SendNotification(QString("%1 Download").arg(m_spCurrentJob->JobType()),
                                   QString("Download of %1 could not finish properly:\n%2")
                                       .arg(m_spCurrentJob->JobName()).arg(m_spCurrentJob->Error()));
      */
    }
    else
    {
      /*
      Notifier()->SendNotification(QString("%1 Download").arg(m_spCurrentJob->JobType()),
                                   "Download stopped.");
      */
    }
  }
}
