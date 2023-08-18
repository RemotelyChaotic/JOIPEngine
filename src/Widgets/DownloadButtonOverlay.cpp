#include "DownloadButtonOverlay.h"

#include "Application.h"

#include "Systems/ProjectDownloader.h"
#include "Widgets/ProgressBar.h"
#include "Widgets/TitleLabel.h"

#include <QVBoxLayout>

CDownloadCounterOverlay::CDownloadCounterOverlay(QWidget* pParent) :
  COverlayBase(COverlayButton::c_iOverlayButtonZOrder + 1, pParent),
  m_pDlCounter(new CTitleLabel("0", this))
{
  setAttribute(Qt::WA_TranslucentBackground);
  setAttribute(Qt::WA_TransparentForMouseEvents);
  setObjectName(QStringLiteral("DownloadCounter"));
  setFrameStyle(QFrame::Plain);
  setFrameShape(QFrame::NoFrame);
  setFrameShadow(QFrame::Plain);

  m_pDlCounter->SetFontSize(14);
  m_pDlCounter->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred));

  QLayout* pLayout = new QVBoxLayout(this);
  pLayout->setMargin(0);
  pLayout->setContentsMargins(0, 0, 0, 0);
  pLayout->addWidget(m_pDlCounter.data());
  setLayout(pLayout);
}
CDownloadCounterOverlay::~CDownloadCounterOverlay()
{

}

//----------------------------------------------------------------------------------------
//
void CDownloadCounterOverlay::Initialize()
{

}

//----------------------------------------------------------------------------------------
//
QPointer<CTitleLabel> CDownloadCounterOverlay::Counter()
{
  return m_pDlCounter;
}

//----------------------------------------------------------------------------------------
//
void CDownloadCounterOverlay::Climb()
{
  ClimbToTop();
  Resize();
}

//----------------------------------------------------------------------------------------
//
void CDownloadCounterOverlay::Resize()
{
  resize(m_pDlCounter->SuggestedSize() + QSize(10, 10));
}

//----------------------------------------------------------------------------------------
//
CDownloadButtonOverlay::CDownloadButtonOverlay(QWidget* pParent) :
  COverlayButton(QStringLiteral("DownloadButtonOverlay"),
                 QStringLiteral("DownloadButton"),
                 QStringLiteral("Open download page"),
                 QStringLiteral("Download"), pParent),
  m_pProgressBar(new CProgressBar(this)),
  m_pCounterOverlay(new CDownloadCounterOverlay(pParent))
{
  m_pProgressBar->raise();
  m_pProgressBar->setAttribute(Qt::WA_TransparentForMouseEvents);
  m_pProgressBar->SetRange(0, 100);
  m_pProgressBar->setValue(0);
  m_pProgressBar->setAlignment(Qt::AlignCenter);
  m_pProgressBar->setTextVisible(false);
  m_pProgressBar->hide();

  m_pCounterOverlay->Climb();
  m_pCounterOverlay->Hide();

  if (auto spDownloader = CApplication::Instance()->System<CProjectDownloader>().lock())
  {
    connect(spDownloader.get(), &CProjectDownloader::SignalJobFinished,
            this, &CDownloadButtonOverlay::SlotDownloadFinished, Qt::QueuedConnection);
    connect(spDownloader.get(), &CProjectDownloader::SignalJobStarted,
            this, &CDownloadButtonOverlay::SlotDownloadStarted, Qt::QueuedConnection);
    connect(spDownloader.get(), &CProjectDownloader::SignalJobAdded,
            this, &CDownloadButtonOverlay::SlotJobAdded, Qt::QueuedConnection);
    connect(spDownloader.get(), &CProjectDownloader::SignalProgressChanged,
            this, &CDownloadButtonOverlay::SlotProgressChanged, Qt::QueuedConnection);

    SlotJobAdded(spDownloader->RunningJobsCount());
  }
}

CDownloadButtonOverlay::~CDownloadButtonOverlay()
{

}

//----------------------------------------------------------------------------------------
//
void CDownloadButtonOverlay::Hide()
{
  COverlayButton::Hide();
  m_pCounterOverlay->hide();
}

//----------------------------------------------------------------------------------------
//
void CDownloadButtonOverlay::Resize()
{
  QPoint pos{50, 50};
  move(pos);

  m_pProgressBar->move(0, 0);
  m_pProgressBar->resize(sizeHint());

  m_pCounterOverlay->Resize();
  m_pCounterOverlay->move(pos.x() - m_pCounterOverlay->size().width() / 2,
                          pos.y() - m_pCounterOverlay->size().height() / 2);
}

//----------------------------------------------------------------------------------------
//
void CDownloadButtonOverlay::Show()
{
  COverlayButton::Show();
  if (m_pCounterOverlay->Counter()->text() != "0")
  {
    m_pCounterOverlay->Show();
  }
}

//----------------------------------------------------------------------------------------
//
void CDownloadButtonOverlay::SlotDownloadFinished()
{
  m_pProgressBar->hide();

  if (auto spDownloader = CApplication::Instance()->System<CProjectDownloader>().lock())
  {
    // we're currently still in the download process, and the queue has not been decremented yet
    SlotJobAdded(spDownloader->RunningJobsCount());
  }
}

//----------------------------------------------------------------------------------------
//
void CDownloadButtonOverlay::SlotDownloadStarted()
{
  m_pProgressBar->SetRange(0, 0);
  m_pProgressBar->setValue(0);
  m_pProgressBar->show();
}

//----------------------------------------------------------------------------------------
//
void CDownloadButtonOverlay::SlotJobAdded(qint32 iNumJobs)
{
  m_pCounterOverlay->Counter()->setText(QString::number(iNumJobs));
  if (iNumJobs > 0)
  {
    m_pCounterOverlay->Show();
  }
  else
  {
    m_pCounterOverlay->Hide();
  }
}

//----------------------------------------------------------------------------------------
//
void CDownloadButtonOverlay::SlotProgressChanged(qint32 iId, qint32 iProgress)
{
  Q_UNUSED(iId)
  if (0 < iProgress)
  {
    m_pProgressBar->SetRange(0, 100);
  }
  m_pProgressBar->setValue(iProgress);
}

