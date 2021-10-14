#ifndef CDOWNLOADBUTTONOVERLAY_H
#define CDOWNLOADBUTTONOVERLAY_H

#include "OverlayButton.h"
#include <QPointer>

class CProgressBar;
class QLabel;

class CDownloadCounterOverlay : public COverlayBase
{
  Q_OBJECT

public:
  explicit CDownloadCounterOverlay(QWidget* pParent = nullptr);
  ~CDownloadCounterOverlay() override;

  void Initialize();

  QPointer<QLabel> Counter();

  void Climb() override;
  void Resize() override;

private:
  QPointer<QLabel>    m_pDlCounter;
};

//----------------------------------------------------------------------------------------
//
class CDownloadButtonOverlay : public COverlayButton
{
  Q_OBJECT

public:
  explicit CDownloadButtonOverlay(QWidget* pParent = nullptr);
  ~CDownloadButtonOverlay() override;

  void Hide() override;
  void Resize() override;
  void Show() override;

private slots:
  void SlotDownloadFinished();
  void SlotDownloadStarted();
  void SlotJobAdded(qint32 iNumJobs);
  void SlotProgressChanged(qint32 iProgress);

private:
  QPointer<CProgressBar>            m_pProgressBar;
  QPointer<CDownloadCounterOverlay> m_pCounterOverlay;
};

#endif // CDOWNLOADBUTTONOVERLAY_H
