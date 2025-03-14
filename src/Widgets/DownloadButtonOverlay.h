#ifndef CDOWNLOADBUTTONOVERLAY_H
#define CDOWNLOADBUTTONOVERLAY_H

#include "OverlayButton.h"
#include <QPointer>

class CProgressBar;
class CTitleLabel;

class CDownloadCounterOverlay : public COverlayBase
{
  Q_OBJECT

public:
  explicit CDownloadCounterOverlay(QWidget* pParent = nullptr);
  ~CDownloadCounterOverlay() override;

  void Initialize();

  QPointer<CTitleLabel> Counter();

public slots:
  void Climb() override;
  void Resize() override;

private:
  QPointer<CTitleLabel>    m_pDlCounter;
};

//----------------------------------------------------------------------------------------
//
class CDownloadButtonOverlay : public COverlayButton
{
  Q_OBJECT

public:
  explicit CDownloadButtonOverlay(QWidget* pParent = nullptr);
  ~CDownloadButtonOverlay() override;

public slots:
  void Hide() override;
  void Resize() override;
  void Show() override;

private slots:
  void SlotDownloadFinished();
  void SlotDownloadStarted();
  void SlotJobAdded(qint32 iNumJobs);
  void SlotProgressChanged(qint32 iId, qint32 iProgress);

private:
  QPointer<CProgressBar>            m_pProgressBar;
  QPointer<CDownloadCounterOverlay> m_pCounterOverlay;
};

#endif // CDOWNLOADBUTTONOVERLAY_H
