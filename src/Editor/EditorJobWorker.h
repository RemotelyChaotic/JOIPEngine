#ifndef CEDITORJOBWORKER_H
#define CEDITORJOBWORKER_H

#include "Systems/ProjectJobWorker.h"
#include <type_traits>

class CEditorJobWorker : public CProjectJobWorker
{
  Q_OBJECT
  Q_DISABLE_COPY(CEditorJobWorker)

public:
  CEditorJobWorker();
  ~CEditorJobWorker() override;

  template<typename T,
           typename = std::enable_if_t<std::is_base_of_v<IRunnableJob, T>>>
  void CreateNewEditorJob(const QVariantList& args)
  {
    if (!IsInitialized()) { return; }
    std::shared_ptr<T> spJob = std::make_shared<T>();
    spJob->moveToThread(thread());
    CProjectJobWorker::CreatedNewJob(spJob, args);
  }
  qint32 GenerateNewId();

signals:
  void SignalEditorJobStarted(qint32 iId, QString type);
  void SignalEditorJobFinished(qint32 iId, QString type);
  void SignalEditorJobProgress(qint32 iId, QString type, qint32 iProgress);
  void SignalEditorJobMessage(qint32 iId, QString type, QString sMsg);

protected:
  void JobFinalizeImpl(tspRunnableJob spJob) override;
  void JobFinishedImpl(qint32 iId, tspRunnableJob spJob) override;
  void JobPreRunImpl(qint32 iId, tspRunnableJob spJob) override;
  void JobPostRunImpl(qint32 iId, bool bOk, tspRunnableJob spJob) override;
  void JobStartedImpl(qint32 iId, tspRunnableJob spJob) override;

private slots:
  void SlotJobFinishedPrivate(qint32 iId);
  void SlotJobStartedPrivate(qint32 iId);
  void SlotProgressChanged(qint32 iId, qint32 iProgress);

private:
  mutable QMutex    m_idMutex;
  qint32            m_iLastId;
  QMetaObject::Connection m_finalizeConn;
};

#endif // CEDITORJOBWORKER_H
