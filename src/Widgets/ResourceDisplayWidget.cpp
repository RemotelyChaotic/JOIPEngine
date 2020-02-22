#include "ResourceDisplayWidget.h"
#include "Application.h"
#include "Settings.h"
#include "Backend/Project.h"
#include "ui_ResourceDisplayWidget.h"

#include <QFileInfo>
#include <QImageReader>
#include <QMovie>
#include <QResizeEvent>
#include <QtConcurrent/QtConcurrent>

BETTER_ENUM(EResourceDisplayType, qint32,
            eLoading = 0,
            eLocalImage = 1,
            eLocalMedia = 2,
            eWebResource = 3,
            eOther = 4,
            eError = 5);

namespace
{
  const qint32 c_iSpinnerMinWidth = 128;
  const qint32 c_iSpinnerHeight = 128;
}

//----------------------------------------------------------------------------------------
//
CResourceDisplayWidget::CResourceDisplayWidget(QWidget* pParent) :
  QWidget(pParent),
  m_spUi(std::make_unique<Ui::CResourceDisplayWidget>()),
  m_spFutureWatcher(std::make_unique<QFutureWatcher<void>>()),
  m_spSpinner(std::make_unique<QMovie>("://resources/gif/spinner_transparent.gif")),
  m_spSettings(CApplication::Instance()->Settings()),
  m_spLoadedMovie(nullptr),
  m_spLoadedPixmap(nullptr),
  m_future(),
  m_spResource(nullptr),
  m_iLoadState(ELoadState::eUnstarted),
  m_iProjectId(-1)
{
  m_spUi->setupUi(this);
  m_spUi->pWebView->page()->setBackgroundColor(Qt::transparent);

  m_spSpinner->start();
  m_spUi->pLoadingSpinner->setMovie(m_spSpinner.get());

  m_spUi->pStackedWidget->setCurrentIndex(EResourceDisplayType::eError);

  connect(m_spFutureWatcher.get(), &QFutureWatcher<void>::finished,
          this, &CResourceDisplayWidget::SlotLoadFinished);
  connect(m_spUi->pWebView, &QWebEngineView::loadFinished,
          this, &CResourceDisplayWidget::SlotWebLoadFinished);

  connect(m_spSettings.get(), &CSettings::MutedChanged,
          this, &CResourceDisplayWidget::SlotMutedChanged, Qt::QueuedConnection);
  connect(m_spSettings.get(), &CSettings::VolumeChanged,
          this, &CResourceDisplayWidget::SlotVolumeChanged, Qt::QueuedConnection);

  connect(m_spUi->pMediaPlayer, &CMediaPlayer::MediaStatusChanged,
          this, &CResourceDisplayWidget::SlotStatusChanged, Qt::DirectConnection);
}

CResourceDisplayWidget::~CResourceDisplayWidget()
{
  m_spSpinner->stop();
  UnloadResource();
}

//----------------------------------------------------------------------------------------
//
void CResourceDisplayWidget::LoadResource(tspResource spResource)
{
  if (nullptr == spResource)
  {
    m_iLoadState = ELoadState::eError;
    m_spUi->pStackedWidget->setCurrentIndex(EResourceDisplayType::eError);
    return;
  }

  SlotStop();

  m_iLoadState = ELoadState::eFinished;
  m_spUi->pStackedWidget->setCurrentIndex(EResourceDisplayType::eLoading);

  m_spResource = spResource;
  QReadLocker resourceLocker(&m_spResource->m_rwLock);
  const tspProject& spProject = m_spResource->m_spParent;
  QReadLocker projectLocker(&spProject->m_rwLock);
  SetProjectId(spProject->m_iId);
  if (m_spResource->m_sPath.isLocalFile())
  {
    QUrl path = m_spResource->m_sPath;
    switch (m_spResource->m_type)
    {
      case EResourceType::eImage:
      {
        projectLocker.unlock();
        QString sPath = ResourceUrlToAbsolutePath(path, PhysicalProjectName(spProject));
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
        QString sPath = ResourceUrlToAbsolutePath(path, PhysicalProjectName(spProject));
        m_spUi->pMediaPlayer->OpenMedia(sPath);
        m_spUi->pStackedWidget->setCurrentIndex(EResourceDisplayType::eLocalMedia);
        m_iLoadState = ELoadState::eFinished;
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
  m_iLoadState = ELoadState::eFinished;
  m_spResource = nullptr;
  if (m_spFutureWatcher->isRunning())
  {
    m_spFutureWatcher->cancel();
    m_spFutureWatcher->waitForFinished();
  }
  SlotStop();
  m_spUi->pWebView->setUrl(QUrl("about:blank"));
}

//----------------------------------------------------------------------------------------
//
EResourceType CResourceDisplayWidget::ResourceType()
{
  if (nullptr != m_spResource)
  {
    QReadLocker resourceLocker(&m_spResource->m_rwLock);
    EResourceType type = m_spResource->m_type;
    QMutexLocker imageLocker(&m_imageMutex);
    if (type._to_integral() == EResourceType::eImage)
    {
      if (nullptr != m_spLoadedMovie)
      {
        type = EResourceType::eMovie;
      }
    }
    return type;
  }
  else
  {
    return EResourceType::eOther;
  }
}

//----------------------------------------------------------------------------------------
//
bool CResourceDisplayWidget::IsRunning()
{
  if (m_iLoadState != ELoadState::eFinished) { return false; }

  bool bRunning = false;
  switch ( m_spUi->pStackedWidget->currentIndex())
  {
    case EResourceDisplayType::eLocalImage:
    {
      m_imageMutex.lock();
      if (nullptr != m_spLoadedMovie)
      {
        bRunning = m_spLoadedMovie->state() == QMovie::Running;
      }
      m_imageMutex.unlock();
      break;
    }
    case EResourceDisplayType::eLocalMedia:
      bRunning = m_spUi->pMediaPlayer->IsPlaying();
      break;
    case EResourceDisplayType::eWebResource:
    {
      // TODO:
      break;
    }
  default: break;
  }

  return bRunning;
}

//----------------------------------------------------------------------------------------
//
void CResourceDisplayWidget::SetMargins(qint32 iLeft, qint32 iTop, qint32 iRight, qint32 iBottom)
{
  m_spUi->pPageError->setContentsMargins(iLeft, iTop, iRight, iBottom);
  m_spUi->pPageImage->setContentsMargins(iLeft, iTop, iRight, iBottom);
  m_spUi->pPageLoading->setContentsMargins(iLeft, iTop, iRight, iBottom);
  m_spUi->pPageMedia->setContentsMargins(iLeft, iTop, iRight, iBottom);
  m_spUi->pPageOther->setContentsMargins(iLeft, iTop, iRight, iBottom);
  m_spUi->pPageWeb->setContentsMargins(iLeft, iTop, iRight, iBottom);
}

//----------------------------------------------------------------------------------------
//
void CResourceDisplayWidget::SlotPlayPause()
{
  if (m_iLoadState != ELoadState::eFinished) { return; }

  switch ( m_spUi->pStackedWidget->currentIndex())
  {
    case EResourceDisplayType::eLocalImage:
    {
      m_imageMutex.lock();
      if (nullptr != m_spLoadedMovie)
      {
        if (m_spLoadedMovie->state() == QMovie::Running)
        {
          m_spLoadedMovie->stop();
        }
        else
        {
          m_spLoadedMovie->start();
        }
      }
      m_imageMutex.unlock();
      break;
    }
    case EResourceDisplayType::eLocalMedia:
      m_spUi->pMediaPlayer->PlayPause();
      break;
    case EResourceDisplayType::eWebResource:
    {
      m_spUi->pWebView->page()->runJavaScript(
            "var video = document.querySelector('video');"
            "if (video !== null) {"
            "  if (video.paused) { "
            "    video.play(); "
            "  } else { "
            "    video.pause();"
            "  }"
            "}"
            );
      break;
    }
  default: break;
  }
}

//----------------------------------------------------------------------------------------
//
void CResourceDisplayWidget::SlotStop()
{
  switch ( m_spUi->pStackedWidget->currentIndex())
  {
    case EResourceDisplayType::eLocalImage:
    {
      m_imageMutex.lock();
      if (nullptr != m_spLoadedMovie)
      {
        if (m_spLoadedMovie->state() == QMovie::Running)
        {
          m_spLoadedMovie->stop();
        }
      }
      m_imageMutex.unlock();
      break;
    }
    case EResourceDisplayType::eLocalMedia:
      m_spUi->pMediaPlayer->Stop();
      break;
    case EResourceDisplayType::eWebResource:
    {
      m_spUi->pWebView->page()->runJavaScript(
            "var video = document.querySelector('video'); "
            "if (video !== null) { "
            "  if (!video.paused) { "
            "    video.pause(); "
            "    video.load(); "
            "  } else { "
            "    video.load(); "
            "  } "
            "  video.pause(); "
            "}"
            );
      break;
    }
  default: break;
  }
}

//----------------------------------------------------------------------------------------
//
void CResourceDisplayWidget::SlotSetSliderVisible(bool bVisible)
{
  switch ( m_spUi->pStackedWidget->currentIndex())
  {
    case EResourceDisplayType::eLocalMedia:
      m_spUi->pMediaPlayer->SetSliderVisible(bVisible);
      break;
    case EResourceDisplayType::eWebResource:
    {
      m_spUi->pWebView->page()->runJavaScript(QString() +
            "var video = document.querySelector('video');"
            "if (video !== null) {"
            "  video.controls = " + (bVisible ? "true" : "false") + ";"
            "}"
            );
      break;
    }
    default:
    {
      m_spUi->pMediaPlayer->SetSliderVisible(bVisible);
      break;
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CResourceDisplayWidget::mousePressEvent(QMouseEvent* pEvent)
{
  Q_UNUSED(pEvent);
  emit OnClick();
}

//----------------------------------------------------------------------------------------
//
void CResourceDisplayWidget::resizeEvent(QResizeEvent* pEvent)
{
  if (nullptr != pEvent)
  {
    if (pEvent->size().width() < c_iSpinnerMinWidth ||
        pEvent->size().height() < c_iSpinnerHeight)
    {
      m_spUi->pLoadingSpinner->setFixedSize(pEvent->size().width(), pEvent->size().height());
    }
    else
    {
      m_spUi->pLoadingSpinner->setFixedSize(c_iSpinnerMinWidth, c_iSpinnerHeight);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CResourceDisplayWidget::SlotImageLoad(QString sPath)
{
  m_imageMutex.lock();
  QImageReader reader(sPath);
  if (reader.canRead())
  {
    if (!reader.supportsAnimation())
    {
      // QImage
      QImage image = reader.read();
      if (!image.isNull())
      {
        m_spLoadedPixmap = std::make_shared<QPixmap>(
              QPixmap::fromImage(image).scaled(width(), height(),
                                               Qt::KeepAspectRatio, Qt::SmoothTransformation));
        if (nullptr != m_spLoadedMovie)
        {
          m_spLoadedMovie->stop();
        }
        m_spLoadedMovie = nullptr;
        m_iLoadState = ELoadState::eFinished;
      }
      else
      {
        qWarning() << reader.errorString();
        if (nullptr != m_spLoadedMovie)
        {
          m_spLoadedMovie->stop();
        }
        m_spLoadedMovie = nullptr;
        m_spLoadedPixmap = nullptr;
        m_iLoadState = ELoadState::eError;
      }
    }
    else
    {
      // QMovie
      m_spLoadedMovie->setFileName(sPath);
      m_spLoadedMovie->setBackgroundColor(Qt::transparent);
      QImage image = reader.read();
      QSize size = image.size();
      QSize resultingSize = size.scaled(width(), height(), Qt::KeepAspectRatio);
      m_spLoadedMovie->setScaledSize(resultingSize);
      m_spLoadedPixmap = nullptr;
      m_iLoadState = ELoadState::eFinished;
    }
  }
  else
  {
    m_iLoadState = ELoadState::eError;
  }
  m_imageMutex.unlock();
}

//----------------------------------------------------------------------------------------
//
void CResourceDisplayWidget::SlotLoadFinished()
{
  if (m_iLoadState == ELoadState::eFinished)
  {
    if (nullptr == m_spResource)
    {
      m_iLoadState = ELoadState::eError;
      m_spUi->pStackedWidget->setCurrentIndex(EResourceDisplayType::eError);
    }
    else
    {
      QReadLocker locker(&m_spResource->m_rwLock);
      switch (m_spResource->m_type)
      {
        case EResourceType::eImage:
        {
          m_imageMutex.lock();
          if (nullptr != m_spLoadedPixmap)
          {
            m_spUi->pImage->setPixmap(*m_spLoadedPixmap);
            m_spUi->pStackedWidget->setCurrentIndex(EResourceDisplayType::eLocalImage);
            emit SignalLoadFinished();
          }
          else if (nullptr != m_spLoadedMovie)
          {
            m_spLoadedMovie->start();
            m_spUi->pImage->setMovie(m_spLoadedMovie.get());
            m_spUi->pStackedWidget->setCurrentIndex(EResourceDisplayType::eLocalImage);
            emit SignalLoadFinished();
          }
          else
          {
            m_iLoadState = ELoadState::eError;
            m_spUi->pStackedWidget->setCurrentIndex(EResourceDisplayType::eError);
          }
          m_imageMutex.unlock();
          break;
        }
        case EResourceType::eOther:
        {
          break;
        }
        default: break;
      }
    }
  }
  else
  {
    m_spUi->pStackedWidget->setCurrentIndex(EResourceDisplayType::eError);
  }
}

//----------------------------------------------------------------------------------------
//
void CResourceDisplayWidget::SlotMutedChanged()
{
  if (m_iLoadState == ELoadState::eFinished)
  {
    bool bMuted = m_spSettings->Muted();
    switch ( m_spUi->pStackedWidget->currentIndex())
    {
      case EResourceDisplayType::eLocalMedia:
      {
        m_spUi->pMediaPlayer->MuteUnmute(bMuted);
      } break;
      case EResourceDisplayType::eWebResource:
      {
        QString sJs = "var video = document.querySelector('video');"
                      "if (video !== null) {"
                      "  video.muted = %1;"
                      "}";
        m_spUi->pWebView->page()->runJavaScript(sJs.arg(bMuted ? "true" : "false"));
      } break;
    default: break;
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CResourceDisplayWidget::SlotStatusChanged(QtAV::MediaStatus status)
{
  switch (status)
  {
    case QtAV::InvalidMedia:
    case QtAV::UnknownMediaStatus:
    case QtAV::NoMedia:
    {
      m_iLoadState = ELoadState::eError;
      m_spUi->pStackedWidget->setCurrentIndex(EResourceDisplayType::eError);
      break;
    }
    case QtAV::LoadingMedia:
    {
      m_iLoadState = ELoadState::eLoading;
      m_spUi->pStackedWidget->setCurrentIndex(EResourceDisplayType::eLoading);
      break;
    }
    case QtAV::LoadedMedia:
    {
      m_iLoadState = ELoadState::eFinished;
      SlotSetSliderVisible(false);
      SlotVolumeChanged();
      SlotMutedChanged();
      m_spUi->pStackedWidget->setCurrentIndex(EResourceDisplayType::eLocalMedia);
      emit SignalLoadFinished();
      break;
    }
    case QtAV::EndOfMedia:
    {
      emit SignalPlaybackFinished();
      break;
    }
    case QtAV::BufferingMedia:
    case QtAV::BufferedMedia:
    case QtAV::StalledMedia:
    default: break;
  }
}

//----------------------------------------------------------------------------------------
//
void CResourceDisplayWidget::SlotVolumeChanged()
{
  if (m_iLoadState == ELoadState::eFinished)
  {
    double dVolume = m_spSettings->Volume();
    switch ( m_spUi->pStackedWidget->currentIndex())
    {
      case EResourceDisplayType::eLocalMedia:
      {
        m_spUi->pMediaPlayer->SetVolume(dVolume);
      } break;
      case EResourceDisplayType::eWebResource:
      {
        QString sJs = "var video = document.querySelector('video');"
                      "if (video !== null) {"
                      "  video.volume = %1;"
                      "}";
        m_spUi->pWebView->page()->runJavaScript(sJs.arg(dVolume));
      } break;
    default: break;
    }
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
      m_spUi->pWebView->page()->runJavaScript(
            "var video = document.querySelector('video');"
            "if (video !== null) {"
            "  video.autoplay = false;"
            "}"
            );
      SlotSetSliderVisible(false);
      SlotVolumeChanged();
      SlotMutedChanged();
      m_spUi->pStackedWidget->setCurrentIndex(EResourceDisplayType::eWebResource);
      emit SignalLoadFinished();
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
  if (m_spFutureWatcher->isRunning())
  {
    m_spFutureWatcher->cancel();
    m_spFutureWatcher->waitForFinished();
  }
  m_imageMutex.lock();
  if (nullptr != m_spLoadedMovie)
  {
    m_spLoadedMovie->stop();
  }
  m_spLoadedMovie = std::make_shared<QMovie>();
  m_imageMutex.unlock();
  m_future = QtConcurrent::run(this, &CResourceDisplayWidget::SlotImageLoad, sPath);
  m_spFutureWatcher->setFuture(m_future);
}
