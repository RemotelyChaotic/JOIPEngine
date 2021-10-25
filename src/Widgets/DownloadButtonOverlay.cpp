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

  QFont font = m_pDlCounter->font();
  font.setPointSize(14);
  m_pDlCounter->setFont(font);
  m_pDlCounter->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
  m_pDlCounter->setFixedSize(QSize(100, 100));

  QLayout* pLayout = new QVBoxLayout(this);
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
QPointer<QLabel> CDownloadCounterOverlay::Counter()
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
  resize(m_pDlCounter->sizeHint());
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
    connect(spDownloader.get(), &CProjectDownloader::SignalDownloadFinished,
            this, &CDownloadButtonOverlay::SlotDownloadFinished, Qt::QueuedConnection);
    connect(spDownloader.get(), &CProjectDownloader::SignalDownloadStarted,
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
  m_pCounterOverlay->move(pos.x() - m_pCounterOverlay->size().width() / 2 - 10,
                          pos.y() - m_pCounterOverlay->size().height() / 2 - 10);
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

