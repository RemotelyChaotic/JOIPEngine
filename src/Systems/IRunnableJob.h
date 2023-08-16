#ifndef IRUNNABLEJOB_H
#define IRUNNABLEJOB_H

#include <QString>
#include <QVariant>
#include <atomic>
#include <memory>

class IRunnableJob : public std::enable_shared_from_this<IRunnableJob>
{
public:
  virtual ~IRunnableJob() {};

  virtual QString Error() const = 0;
  virtual bool Finished() const = 0;
  virtual bool HasError() const = 0;
  virtual qint32 Id() const = 0;
  virtual QString JobName() const = 0;
  virtual QString JobType() const = 0;
  virtual qint32 Progress() const = 0;

  virtual bool Run(const QVariantList& args) = 0;
  virtual void Stop() { m_bStop = true; }

  bool WasStopped() { return m_bStop; }

signals:
  virtual void SignalFinished(qint32 iId) = 0;
  virtual void SignalProgressChanged(qint32 iId, qint32 iProgress) = 0;
  virtual void SignalStarted(qint32 iId) = 0;

protected:
  IRunnableJob() : m_bStop(false) {}
  virtual void AbortImpl() = 0;

  bool CheckIsStoppedAndAbort(qint32 iId) {
    if (m_bStop) { AbortImpl(); emit SignalFinished(iId); return true; }
    return false;
  };

  std::atomic<bool>           m_bStop;

private:
  Q_DISABLE_COPY(IRunnableJob)
};

typedef std::shared_ptr<IRunnableJob> tspRunnableJob;

Q_DECLARE_INTERFACE(IRunnableJob, "IRunnableJob");
Q_DECLARE_METATYPE(tspRunnableJob)

#endif // IRUNNABLEJOB_H
