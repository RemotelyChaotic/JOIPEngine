#include "ResourceDetailViewFetcherThread.h"
#include "Application.h"
#include "Systems/DatabaseImageProvider.h"
#include "Systems/DatabaseManager.h"

CResourceDetailViewFetcherThread::CResourceDetailViewFetcherThread() :
  CSystemBase(),
  m_spDbImageProvider(nullptr)
{}
CResourceDetailViewFetcherThread::~CResourceDetailViewFetcherThread()
{
}

//----------------------------------------------------------------------------------------
//
void CResourceDetailViewFetcherThread::AbortLoading()
{
  m_bLoading = false;
}

//----------------------------------------------------------------------------------------
//
void CResourceDetailViewFetcherThread::RequestResources(const QString& sProject,
                                                        const QStringList& vsResources,
                                                        const QSize& imageSize)
{
  AbortLoading();
  bool bOk = QMetaObject::invokeMethod(this, "SlotResourcesRequested", Qt::QueuedConnection,
                                       Q_ARG(QString, sProject),
                                       Q_ARG(QStringList, vsResources),
                                       Q_ARG(QSize, imageSize));
  assert(bOk); Q_UNUSED(bOk);
}

//----------------------------------------------------------------------------------------
//
bool CResourceDetailViewFetcherThread::IsLoading() const
{
  return m_bLoading;
}

//----------------------------------------------------------------------------------------
//
void CResourceDetailViewFetcherThread::Initialize()
{
  m_spDbImageProvider.reset(
        new CDatabaseImageProvider(CApplication::Instance()->System<CDatabaseManager>()));
  m_bLoading = false;
  SetInitialized(true);
}

//----------------------------------------------------------------------------------------
//
void CResourceDetailViewFetcherThread::Deinitialize()
{
  m_bLoading = false;
  m_spDbImageProvider.reset(nullptr);
  SetInitialized(false);
}

//----------------------------------------------------------------------------------------
//
void CResourceDetailViewFetcherThread::SlotResourcesRequested(const QString& sProject,
                                                              const QStringList& vsResources,
                                                              const QSize& imageSize)
{
  m_bLoading = true;
  emit LoadStarted();

  for (const QString& sResource : vsResources)
  {
    QSize actualSize;
    QImage img =
      m_spDbImageProvider->requestImage(sProject + "/" + sResource, &actualSize,
                                         imageSize);

    if (!img.isNull() && m_bLoading)
    {
      emit LoadFinished(sResource, QPixmap::fromImage(img));
    }

    // early stop because new images were requested
    if (!m_bLoading)
    {
      emit LoadFinished(QString(), QPixmap());
      return;
    }
  }

  m_bLoading = false;
  emit LoadFinished(QString(), QPixmap());
}
