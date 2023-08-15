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

private slots:
  void SlotJobFinished(qint32 iProjId) override;
  void SlotJobStarted(qint32 iProjId) override;

private:
  void RunNextJobImpl(const QVariantList& args) override;

  mutable QMutex    m_idMutex;
  qint32            m_iLastId;
};

#endif // CEDITORJOBWORKER_H
