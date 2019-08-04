#include "ResourceDisplayWidget.h"
#include "Backend/Project.h"
#include "ui_ResourceDisplayWidget.h"

#include <QFileInfo>
#include <QMovie>
#include <QtConcurrent/QtConcurrent>

BETTER_ENUM(EResourceDisplayType, qint32,
            eLoading = 0,
            eLocalImage = 1,
            eLocalMedia = 2,
            eWebResource = 3,
            eOther = 4,
            eError = 5);

//----------------------------------------------------------------------------------------
//
CResourceDisplayWidget::CResourceDisplayWidget(QWidget* pParent) :
  QWidget(pParent),
  m_spUi(std::make_unique<Ui::CResourceDisplayWidget>()),
  m_spFutureWatcher(std::make_unique<QFutureWatcher<void>>()),
  m_spSpinner(std::make_unique<QMovie>("://resources/gif/spinner_transparent.gif")),
  m_spLoadedPixmap(nullptr),
  m_future(),
  m_spResource(nullptr),
  m_iLoadState(ELoadState::eUnstarted)
{
  m_spUi->setupUi(this);

  m_spSpinner->start();
  m_spUi->pLoadingSpinner->setMovie(m_spSpinner.get());

  connect(m_spFutureWatcher.get(), &QFutureWatcher<void>::finished,
          this, &CResourceDisplayWidget::SlotLoadFinished);
  //connect(m_spUi->pMediaPlayer, &CMediaPlayer::SignalStatusChanged,
  //        this, &CResourceDisplayWidget::SlotStatusChanged);
  connect(m_spUi->pWebView, &QWebEngineView::loadFinished,
          this, &CResourceDisplayWidget::SlotWebLoadFinished);
}

CResourceDisplayWidget::~CResourceDisplayWidget()
{
  m_spSpinner->stop();
  if (m_spFutureWatcher->isRunning())
  {
    m_spFutureWatcher->cancel();
    m_spFutureWatcher->waitForFinished();
  }
}

//----------------------------------------------------------------------------------------
//
void CResourceDisplayWidget::LoadResource(tspResource spResource)
{
  m_iLoadState = ELoadState::eFinished;
  m_spUi->pStackedWidget->setCurrentIndex(EResourceDisplayType::eLoading);

  m_spResource = spResource;
  QReadLocker resourceLocker(&m_spResource->m_rwLock);
  const tspProject& spProject = m_spResource->m_spParent;
  QReadLocker projectLocker(&spProject->m_rwLock);
  if (m_spResource->m_sPath.isLocalFile())
  {
    QUrl path = m_spResource->m_sPath;
    switch (m_spResource->m_type)
    {
      case EResourceType::eImage:
      {
        QString sPath = ResourceUrlToAbsolutePath(path, spProject->m_sName);
        if (QFileInfo(sPath).exists())
        {
          StartImageLoad(sPath);
        }
        else
        {
          m_spUi->pStackedWidget->setCurrentIndex(EResourceDisplayType::eError);
        }
        break;
      }
      case EResourceType::eMovie: // fallthrough
      case EResourceType::eSound:
      {
        //m_spUi->pMediaPlayer->SetPlaylist(path);
        break;
      }
      case EResourceType::eOther:
      {
        break;
      }
      default: break;
    }
  }
  else
  {
    m_spUi->pWebView->setUrl(m_spResource->m_sPath);
    m_iLoadState = ELoadState::eLoading;
  }
}

//----------------------------------------------------------------------------------------
//
void CResourceDisplayWidget::UnloadResource()
{
  m_spResource = nullptr;
  if (m_spFutureWatcher->isRunning())
  {
    m_spFutureWatcher->cancel();
    m_spFutureWatcher->waitForFinished();
  }
  m_iLoadState = ELoadState::eFinished;
}

//----------------------------------------------------------------------------------------
//
void CResourceDisplayWidget::SlotImageLoad(QString sPath)
{
  m_pixmapMutex.lock();
  QImage image;
  bool bOk = image.load(sPath);
  if (bOk)
  {
    m_spLoadedPixmap = std::make_shared<QPixmap>(
          QPixmap::fromImage(image).scaled(m_spUi->pImage->width(), m_spUi->pImage->height(),
                                           Qt::KeepAspectRatio, Qt::SmoothTransformation));
    m_iLoadState = ELoadState::eFinished;
  }
  else
  {
    m_iLoadState = ELoadState::eError;
  }
  m_pixmapMutex.unlock();
}

//----------------------------------------------------------------------------------------
//
void CResourceDisplayWidget::SlotLoadFinished()
{
  if (m_iLoadState == ELoadState::eFinished)
  {
    QReadLocker locker(&m_spResource->m_rwLock);
    switch (m_spResource->m_type)
    {
      case EResourceType::eImage:
      {
        m_pixmapMutex.lock();
        m_spUi->pImage->setPixmap(*m_spLoadedPixmap);
        m_pixmapMutex.unlock();
        m_spUi->pStackedWidget->setCurrentIndex(EResourceDisplayType::eLocalImage);
        break;
      }
      case EResourceType::eOther:
      {
        break;
      }
      default: break;
    }
  }
  else
  {
    m_spUi->pStackedWidget->setCurrentIndex(EResourceDisplayType::eError);
  }
}

//----------------------------------------------------------------------------------------
//
void CResourceDisplayWidget::SlotStatusChanged(QMediaPlayer::MediaStatus status)
{
  switch (status)
  {
    case QMediaPlayer::InvalidMedia:
    case QMediaPlayer::UnknownMediaStatus:
    case QMediaPlayer::NoMedia:
    {
      m_iLoadState = ELoadState::eError;
      m_spUi->pStackedWidget->setCurrentIndex(EResourceDisplayType::eError);
      break;
    }
    case QMediaPlayer::BufferingMedia:
    case QMediaPlayer::BufferedMedia:
    case QMediaPlayer::StalledMedia:
    case QMediaPlayer::LoadingMedia:
    {
      m_iLoadState = ELoadState::eLoading;
      break;
    }
    case QMediaPlayer::LoadedMedia:
    {
      m_iLoadState = ELoadState::eFinished;
      m_spUi->pStackedWidget->setCurrentIndex(EResourceDisplayType::eLocalMedia);
      break;
    }
    case QMediaPlayer::EndOfMedia:
    default: break;
  }
}

//----------------------------------------------------------------------------------------
//
void CResourceDisplayWidget::SlotWebLoadFinished(bool bOk)
{
  if (ELoadState::eLoading == m_iLoadState)
  {
    if (bOk)
    {
      m_iLoadState = ELoadState::eFinished;
      m_spUi->pStackedWidget->setCurrentIndex(EResourceDisplayType::eWebResource);
    }
    else
    {
      m_iLoadState = ELoadState::eError;
      m_spUi->pStackedWidget->setCurrentIndex(EResourceDisplayType::eError);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CResourceDisplayWidget::StartImageLoad(QString sPath)
{
  m_iLoadState = ELoadState::eLoading;
  m_future = QtConcurrent::run(this, &CResourceDisplayWidget::SlotImageLoad, sPath);
  m_spFutureWatcher->setFuture(m_future);
}
