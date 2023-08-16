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
    CProjectJobWorker::CreatedNewJob(spJob, args);
  }
  qint32 GenerateNewId();

signals:
  void SignalJobMessage(qint32 iId, QString sMsg);

protected:
  void JobFinishedImpl(qint32 iId, tspRunnableJob spJob) override;
  void JobRunImpl(qint32 iId, bool bOk, tspRunnableJob spJob) override;
  void JobStartedImpl(qint32 iId, tspRunnableJob spJob) override;

  mutable QMutex    m_idMutex;
  qint32            m_iLastId;
};

#endif // CEDITORJOBWORKER_H
