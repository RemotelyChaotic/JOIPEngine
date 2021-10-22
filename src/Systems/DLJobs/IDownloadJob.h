#ifndef IDOWNLOADJOB_H
#define IDOWNLOADJOB_H

#include <QObject>
#include <QVariant>
#include <atomic>

#define ABORT_CHECK(id) \
  if (CheckIsStoppedAndAbort(id)) { return false; }

//----------------------------------------------------------------------------------------
//
class IDownloadJobWidgetProvider
{
public:
  virtual ~IDownloadJobWidgetProvider() {};

  virtual QWidget* operator()() const = 0;

protected:
  IDownloadJobWidgetProvider() {}
};

//----------------------------------------------------------------------------------------
//
class IDownloadJob
{
public:
  virtual ~IDownloadJob() {};

  virtual QString Error() = 0;
  virtual bool Finished() = 0;
  virtual QString JobName() const = 0;
  virtual qint32 Progress() const = 0;
  virtual bool Run(const QVariantList& args) = 0;
  virtual void Stop() { m_bStop = true; }

  bool WasStopped() { return m_bStop; }

signals:
  virtual void SignalFinished(qint32 iProjId) = 0;
  virtual void SignalProgressChanged(qint32 iProjId, qint32 iProgress) = 0;
  virtual void SignalStarted(qint32 iProjId) = 0;

protected:
  IDownloadJob() : m_bStop(false) {}
  virtual void AbortImpl() = 0;

  bool CheckIsStoppedAndAbort(qint32 iProjId) {
    if (m_bStop) { AbortImpl(); emit SignalFinished(iProjId); return true; }
    return false;
  };

  std::atomic<bool>           m_bStop;

private:
  Q_DISABLE_COPY(IDownloadJob)
};

typedef std::shared_ptr<IDownloadJob> tspDownloadJob;

Q_DECLARE_INTERFACE(IDownloadJob, "IDownloadJob");
Q_DECLARE_METATYPE(tspDownloadJob)

#endif // IDOWNLOADJOB_H
