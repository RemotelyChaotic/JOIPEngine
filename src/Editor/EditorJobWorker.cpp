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
        emit SignalJobMessage(iId, sMsg);
      }
      else
      {
        QString sMsg = QString("%1 job finished.")
                           .arg(spJob->JobType());
        emit SignalJobMessage(iId, sMsg);
      }
    }
    else
    {
      QString sMsg = QString("%1 job stopped.")
                         .arg(spJob->JobType());
      emit SignalJobMessage(iId, sMsg);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorJobWorker::JobRunImpl(qint32 iId, bool bOk, tspRunnableJob spJob)
{
  if (!bOk)
  {
    if (!spJob->WasStopped())
    {
      QString sMsg = QString("Failed to run %1 job.\n%2")
                         .arg(spJob->JobType()).arg(spJob->Error());
      emit SignalJobMessage(iId, sMsg);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorJobWorker::JobStartedImpl(qint32 iId, tspRunnableJob spJob)
{
  if (nullptr != spJob)
  {
    bool bOkConn = connect(dynamic_cast<QObject*>(spJob.get()), SIGNAL(SignalJobMessage(qint32,QString)),
                           this, SLOT(SignalJobMessage(qint32,QString)), Qt::QueuedConnection);
    assert(bOkConn); Q_UNUSED(bOkConn);

    QString sMsg = QString("%1 job started.").arg(spJob->JobType());
    emit SignalJobMessage(iId, sMsg);
  }
}
