#ifndef IDOWNLOADJOB_H
#define IDOWNLOADJOB_H

#include <QObject>
#include <QVariant>
#include <atomic>

#define ABORT_CHECK \
  if (CheckIsStoppedAndAbort()) { return; }

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

  virtual bool Finished() = 0;
  virtual qint32 Progress() = 0;
  virtual void Run(const QVariantList& args) = 0;
  virtual void Stop() { m_bStop = true; }

signals:
  virtual void SignalFinished() = 0;
  virtual void SignalProgressChanged(qint32 iProgress) = 0;
  virtual void SignalStarted() = 0;

protected:
  IDownloadJob() : m_bStop(false) {}
  virtual void AbortImpl() = 0;

  bool CheckIsStoppedAndAbort() {
    if (m_bStop) { AbortImpl(); emit SignalFinished(); return true; }
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
