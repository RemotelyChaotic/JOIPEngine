#include "ResourceDetailViewFetcherThread.h"
#include "Application.h"
#include "Systems/DatabaseImageProvider.h"
#include "Systems/DatabaseManager.h"

#include <QBuffer>

CResourceDetailViewFetcherThread::CResourceDetailViewFetcherThread() :
  CSystemBase(),
  m_spDbImageProvider(nullptr),
  m_queueMutex(QMutex::Recursive),
  m_queue()
{}
CResourceDetailViewFetcherThread::~CResourceDetailViewFetcherThread()
{
}

//----------------------------------------------------------------------------------------
//
void CResourceDetailViewFetcherThread::AbortLoading()
{
  m_bLoading = false;
  QMutexLocker locker(&m_queueMutex);
  m_queue = QStringList();
}

//----------------------------------------------------------------------------------------
//
void CResourceDetailViewFetcherThread::RequestResources(qint32 iProject,
                                                        const QStringList& vsResources,
                                                        const QSize& imageSize)
{
  if (vsResources.size() > 0)
  {
    {
      QMutexLocker locker(&m_queueMutex);
      m_queue = vsResources + m_queue;
    }
    if (!m_bLoading)
    {
      bool bOk = QMetaObject::invokeMethod(this, "SlotResourcesRequested", Qt::QueuedConnection,
                                           Q_ARG(qint32, iProject),
                                           Q_ARG(QSize, imageSize));
      assert(bOk); Q_UNUSED(bOk);
    }
  }
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
void CResourceDetailViewFetcherThread::SlotResourcesRequested(qint32 iProject,
                                                              const QSize& imageSize)
{
  m_bLoading = true;
  emit LoadStarted();

  QMutexLocker locker(&m_queueMutex);
  while (m_queue.size() > 0)
  {
    QString sResource = m_queue[0];
    sResource.detach();
    m_queue.erase(m_queue.begin());
    locker.unlock();

    QSize actualSize;
    QImage img =
      m_spDbImageProvider->requestImage(QString::number(iProject) + "/" + sResource,
                                        &actualSize, imageSize);

    if (!img.isNull() && m_bLoading)
    {
      emit LoadFinished(sResource, QPixmap::fromImage(img));
      QByteArray ba;
      QBuffer buffer(&ba);
      if (buffer.open(QIODevice::ReadWrite))
      {
        img.save(&buffer, "png");
        emit LoadFinished(sResource, QString::fromUtf8(ba.toBase64()));
      }
    }

    // early stop because new images were requested
    if (!m_bLoading)
    {
      emit LoadFinished(QString(), QPixmap());
      emit LoadFinished(QString(), QString());
      return;
    }

    locker.relock();
  }
  locker.unlock();

  m_bLoading = false;
  emit LoadFinished(QString(), QPixmap());
  emit LoadFinished(QString(), QString());
}
