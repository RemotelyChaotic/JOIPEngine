#ifndef IDOWNLOADJOB_H
#define IDOWNLOADJOB_H

#include "Systems/IRunnableJob.h"
#include <QObject>

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
class IDownloadJob : public IRunnableJob
{
public:
  ~IDownloadJob() override {};

protected:
  IDownloadJob() : IRunnableJob() {}

private:
  Q_DISABLE_COPY(IDownloadJob)
};

typedef std::shared_ptr<IDownloadJob> tspDownloadJob;

Q_DECLARE_INTERFACE(IDownloadJob, "IDownloadJob");
Q_DECLARE_METATYPE(tspDownloadJob)

#endif // IDOWNLOADJOB_H
