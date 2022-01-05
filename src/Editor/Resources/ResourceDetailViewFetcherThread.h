#ifndef CRESOURCEDETAILVIEWFETCHERTHREAD_H
#define CRESOURCEDETAILVIEWFETCHERTHREAD_H

#include "Systems/ThreadedSystem.h"

#include <QMutex>

class CDatabaseImageProvider;

class CResourceDetailViewFetcherThread : public CSystemBase
{
  Q_OBJECT
public:
  CResourceDetailViewFetcherThread();
  ~CResourceDetailViewFetcherThread() override;

  void AbortLoading();
  void RequestResources(qint32 iProject,
                        const QStringList& vsResources,
                        const QSize& imageSize);
  bool IsLoading() const;

signals:
  void LoadFinished(const QString& sResource, const QPixmap& pixmap);
  void LoadStarted();

public slots:
  void Initialize() override;
  void Deinitialize() override;

private slots:
  void SlotResourcesRequested(qint32 iProject,
                              const QSize& imageSize);

private:
  std::unique_ptr<CDatabaseImageProvider> m_spDbImageProvider;
  std::atomic<bool>                       m_bLoading;
  QMutex                                  m_queueMutex;
  QStringList                             m_queue;

};

#endif // CRESOURCEDETAILVIEWFETCHERTHREAD_H
