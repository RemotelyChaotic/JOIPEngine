#include "ProjectCardWidget.h"
#include "ui_ProjectCardWidget.h"
#include <QtConcurrent/QtConcurrent>
#include <QFileInfo>
#include <QFuture>
#include <QMovie>

CProjectCardWidget::CProjectCardWidget(std::shared_ptr<QMovie> spSpinner, QWidget* pParent) :
  QWidget(pParent),
  m_spUi(std::make_unique<Ui::CProjectCardWidget>()),
  m_spFutureWatcher(std::make_unique<QFutureWatcher<void>>()),
  m_spSpinner(spSpinner),
  m_spLoadedPixmap(nullptr),
  m_future(),
  m_iLoadState(ELoadState::eUnstarted),
  m_iProjectId(-1)
{
  m_spUi->setupUi(this);

  connect(m_spFutureWatcher.get(), &QFutureWatcher<void>::finished,
          this, &CProjectCardWidget::LoadFinished);
}

CProjectCardWidget::~CProjectCardWidget()
{
  if (m_spFutureWatcher->isRunning())
  {
    m_spFutureWatcher->cancel();
    m_spFutureWatcher->waitForFinished();
  }
}

//----------------------------------------------------------------------------------------
//
void CProjectCardWidget::Initialize(const tspProject& spProject)
{
  spProject->m_rwLock.lockForRead();
  auto it = spProject->m_spResourcesMap.find(spProject->m_sTitleCard);

  m_iProjectId = spProject->m_iId;
  m_spUi->pTitle->setText(spProject->m_sName);

  if (spProject->m_spResourcesMap.end() != it)
  {
    it->second->m_rwLock.lockForRead();
    QString sPath = it->second->m_sPath;
    it->second->m_rwLock.unlock();
    if (QFileInfo(sPath).exists())
    {
      StartLoad(sPath);
    }
    else
    {
      StartLoad("://resources/img/TitleCardMissing.png");
    }
  }
  else
  {
    StartLoad("://resources/img/TitleCardMissing.png");
  }
  spProject->m_rwLock.unlock();
}

//----------------------------------------------------------------------------------------
//
void CProjectCardWidget::mousePressEvent(QMouseEvent* pEvent)
{
  Q_UNUSED(pEvent);
  emit Clicked();
}

//----------------------------------------------------------------------------------------
//
void CProjectCardWidget::Load(QString sPath)
{
  m_pixmapMutex.lock();
  QImage image;
  bool bOk = image.load(sPath);
  if (bOk)
  {
    m_spLoadedPixmap = std::make_shared<QPixmap>(QPixmap::fromImage(image));
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
void CProjectCardWidget::LoadFinished()
{
  m_pixmapMutex.lock();
  if (ELoadState::eFinished == m_iLoadState)
  {
    m_spUi->pTitleImage->setPixmap(*m_spLoadedPixmap);
  }
  else
  {
    m_spUi->pTitleImage->setPixmap(QPixmap("://resources/img/TitleCardMissing.png"));
  }
  m_pixmapMutex.unlock();
}

//----------------------------------------------------------------------------------------
//
void CProjectCardWidget::StartLoad(QString sPath)
{
  if (nullptr != m_spSpinner)
  {
    m_spSpinner->start();
    m_spUi->pTitleImage->setMovie(m_spSpinner.get());
  }

  m_iLoadState = ELoadState::eLoading;
  m_future = QtConcurrent::run(this, &CProjectCardWidget::Load, sPath);
  m_spFutureWatcher->setFuture(m_future);
}
