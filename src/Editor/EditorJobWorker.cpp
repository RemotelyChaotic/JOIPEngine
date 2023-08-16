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
void CEditorJobWorker::JobFinalizeImpl(tspRunnableJob spJob)
{
  disconnect(m_finalizeConn);
}

//----------------------------------------------------------------------------------------
//
void CEditorJobWorker::JobFinishedImpl(qint32 iId, tspRunnableJob spJob)
{
  if (nullptr != spJob)
  {
    if (!spJob->WasStopped())
    {
      if (spJob->HasError())
      {
        QString sMsg = QString("Failed to run %1 job.\n%2")
                           .arg(spJob->JobType()).arg(spJob->Error());
        emit SignalEditorJobMessage(iId, spJob->JobType(), sMsg);
      }
      else
      {
        QString sMsg = QString("%1 job finished.")
                           .arg(spJob->JobType());
        emit SignalEditorJobMessage(iId, spJob->JobType(), sMsg);
      }
    }
    else
    {
      QString sMsg = QString("%1 job stopped.")
                         .arg(spJob->JobType());
      emit SignalEditorJobMessage(iId, spJob->JobType(), sMsg);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorJobWorker::JobPreRunImpl(qint32 iId, tspRunnableJob spJob)
{
  // connections (will be removed once the object is deleted, so we won't disconnect
  connect(dynamic_cast<QObject*>(m_spCurrentJob.get()), SIGNAL(SignalFinished(qint32)),
          this, SLOT(SlotJobFinishedPrivate(qint32)), Qt::DirectConnection);

  m_finalizeConn =
      connect(dynamic_cast<QObject*>(m_spCurrentJob.get()), SIGNAL(SignalFinished(qint32)),
              this, SLOT(SlotFinalizeJob()), Qt::DirectConnection);

  connect(dynamic_cast<QObject*>(m_spCurrentJob.get()), SIGNAL(SignalProgressChanged(qint32,qint32)),
          this, SLOT(SlotProgressChanged(qint32,qint32)), Qt::DirectConnection);
  connect(dynamic_cast<QObject*>(m_spCurrentJob.get()), SIGNAL(SignalStarted(qint32)),
          this, SLOT(SlotJobStartedPrivate(qint32)), Qt::DirectConnection);
}

//----------------------------------------------------------------------------------------
//
void CEditorJobWorker::JobPostRunImpl(qint32 iId, bool bOk, tspRunnableJob spJob)
{
  if (!bOk)
  {
    if (!spJob->WasStopped())
    {
      QString sMsg = QString("Failed to run %1 job.\n%2")
                         .arg(spJob->JobType()).arg(spJob->Error());
      emit SignalEditorJobMessage(iId, spJob->JobType(), sMsg);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorJobWorker::JobStartedImpl(qint32 iId, tspRunnableJob spJob)
{
  if (nullptr != spJob)
  {
    bool bOkConn = connect(dynamic_cast<QObject*>(spJob.get()), SIGNAL(SignalJobMessage(qint32,QString,QString)),
                           this, SIGNAL(SignalEditorJobMessage(qint32,QString,QString)), Qt::QueuedConnection);
    assert(bOkConn); Q_UNUSED(bOkConn);

    QString sMsg = QString("%1 job started.").arg(spJob->JobType());
    emit SignalEditorJobMessage(iId, spJob->JobType(), sMsg);
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorJobWorker::SlotJobFinishedPrivate(qint32 iId)
{
  IRunnableJob* pJob = dynamic_cast<IRunnableJob*>(sender());
  if (nullptr != pJob)
  {
    JobFinishedImpl(iId, pJob->shared_from_this());
  }
  emit SignalEditorJobFinished(iId, m_spCurrentJob->JobType());
}

//----------------------------------------------------------------------------------------
//
void CEditorJobWorker::SlotJobStartedPrivate(qint32 iId)
{
  IRunnableJob* pJob = dynamic_cast<IRunnableJob*>(sender());
  if (nullptr != pJob)
  {
    JobStartedImpl(iId, pJob->shared_from_this());
  }
  emit SignalEditorJobStarted(iId, m_spCurrentJob->JobType());
}

//----------------------------------------------------------------------------------------
//
void CEditorJobWorker::SlotProgressChanged(qint32 iId, qint32 iProgress)
{
  emit SignalEditorJobProgress(iId, m_spCurrentJob->JobType(), iProgress);
}
