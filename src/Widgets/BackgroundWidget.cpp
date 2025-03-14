#include "BackgroundWidget.h"
#include "Enums.h"
#include "Systems/Project.h"
#include "Systems/Resource.h"

#include <QFileInfo>
#include <QImageReader>
#include <QLinearGradient>
#include <QPainter>
#include <QtConcurrent/QtConcurrent>
#include <QStyle>

CBackgroundWidget::CBackgroundWidget(QWidget* pParent) :
  QWidget(pParent),
  m_spFutureWatcher(std::make_unique<QFutureWatcher<void>>()),
  m_spLoadedPixmap(nullptr),
  m_imageMutex(),
  m_future(),
  m_iLoadState(ELoadState::eUnstarted),
  m_backgroundPixmap(),
  m_backgroundColor(QColor(0, 0, 0, 0))
{
  connect(m_spFutureWatcher.get(), &QFutureWatcher<void>::finished,
          this, &CBackgroundWidget::SlotLoadFinished);
}

CBackgroundWidget::~CBackgroundWidget()
{
  m_iLoadState = ELoadState::eFinished;
  if (m_spFutureWatcher->isRunning())
  {
    m_spFutureWatcher->cancel();
    m_spFutureWatcher->waitForFinished();
  }
}

//----------------------------------------------------------------------------------------
//
void CBackgroundWidget::SetBackgroundColor(const QColor& color)
{
  m_backgroundColor = color;
  repaint();
}

//----------------------------------------------------------------------------------------
//
void CBackgroundWidget::SetBackgroundTexture(const QString& sTexture)
{
  m_sBgTexture = sTexture;
  m_iLoadState = ELoadState::eFinished;
  if (QFileInfo(sTexture).exists())
  {
    StartImageLoad(sTexture);
  }
  else
  {
    m_imageMutex.lock();
    m_backgroundPixmap = QPixmap();
    m_imageMutex.unlock();
  }
}

//----------------------------------------------------------------------------------------
//
void CBackgroundWidget::SlotBackgroundColorChanged(QColor color)
{
  SetBackgroundColor(color);
}

//----------------------------------------------------------------------------------------
//
void CBackgroundWidget::SlotBackgroundTextureChanged(tspResource spResource)
{
  if (nullptr == spResource || nullptr == spResource->m_spParent)
  {
    SetBackgroundTexture(QString());
  }
  else
  {
    QReadLocker locker(&spResource->m_rwLock);
    QReadLocker projLocker(&spResource->m_spParent->m_rwLock);
    if (EResourceType::eImage == spResource->m_type._to_integral())
    {
      if (IsLocalFile(spResource->m_sPath))
      {
        locker.unlock();
        projLocker.unlock();
        SetBackgroundTexture(ResourceUrlToAbsolutePath(spResource));
      }
      else
      {
        QString sError(tr("Background texture must be a local file."));
        qWarning() << sError;
        emit SignalError(sError, QtMsgType::QtWarningMsg);
      }
    }
    else
    {
      QString sError(tr("Resource must be of image type for backgrounds."));
      qWarning() << sError;
      emit SignalError(sError, QtMsgType::QtWarningMsg);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CBackgroundWidget::paintEvent(QPaintEvent* /*pEvent*/)
{
  QPainter painter(this);
  QPainter::CompositionMode oldCompositionMode = painter.compositionMode();
  painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform |
                           QPainter::TextAntialiasing, true);

  QBrush bgBrush;
  bgBrush.setTexture(m_backgroundPixmap);
  painter.fillRect(rect(), bgBrush);

  painter.setCompositionMode(QPainter::CompositionMode_Screen);

  painter.setBrush(m_backgroundColor);
  painter.setPen(Qt::NoPen);
  painter.drawRect(rect());

  painter.setCompositionMode(oldCompositionMode);

  QLinearGradient linGrad(0, 0, width(), 0);
  linGrad.setColorAt(0.0, QColor(0, 0, 0, 150));
  linGrad.setColorAt(0.4, QColor(0, 0, 0, 50));
  linGrad.setColorAt(0.6, QColor(0, 0, 0, 50));
  linGrad.setColorAt(1.0, QColor(0, 0, 0, 150));

  painter.setBrush(linGrad);
  painter.setPen(Qt::NoPen);
  painter.drawRect(rect());
}

//----------------------------------------------------------------------------------------
//
void CBackgroundWidget::resizeEvent(QResizeEvent* pEvent)
{
  Q_UNUSED(pEvent)
  SetBackgroundTexture(m_sBgTexture);
  style()->unpolish(this);
  style()->polish(this);
}

//----------------------------------------------------------------------------------------
//
void CBackgroundWidget::SlotImageLoad(QString sPath)
{
  m_imageMutex.lock();
  QImageReader reader(sPath);
  if (reader.canRead())
  {
    QImage image = reader.read();
    if (!image.isNull())
    {
      m_spLoadedPixmap = std::make_shared<QPixmap>(QPixmap::fromImage(image));
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
void CBackgroundWidget::SlotLoadFinished()
{
  if (m_iLoadState == ELoadState::eFinished)
  {
    QMutexLocker locker(&m_imageMutex);
    if (nullptr != m_spLoadedPixmap)
    {
      m_backgroundPixmap = *m_spLoadedPixmap;
      repaint();
    }
  }
  else
  {
    QString sError(tr("Error while loading background image."));
    qWarning() << sError;
    emit SignalError(sError, QtMsgType::QtWarningMsg);
  }
}

//----------------------------------------------------------------------------------------
//
void CBackgroundWidget::StartImageLoad(QString sPath)
{
  m_iLoadState = ELoadState::eLoading;
  if (m_spFutureWatcher->isRunning())
  {
    m_spFutureWatcher->cancel();
    m_spFutureWatcher->waitForFinished();
  }
  m_future = QtConcurrent::run(this, &CBackgroundWidget::SlotImageLoad, sPath);
  m_spFutureWatcher->setFuture(m_future);
}
