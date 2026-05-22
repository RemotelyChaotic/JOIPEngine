#include "WebResourceOverlay.h"
#include "WebResourceDownloadManager.h"
#include "ui_WebResourceOverlay.h"

CWebResourceOverlay::CWebResourceOverlay(QWidget* pParent) :
  COverlayBase(1, pParent),
  m_spUi(new Ui::CWebResourceOverlay)
{
  m_spUi->setupUi(this);
}

CWebResourceOverlay::~CWebResourceOverlay()
{
}

//----------------------------------------------------------------------------------------
//
void CWebResourceOverlay::SetDownloadManager(std::weak_ptr<CWebResourceDownloadManager> wpDownloadManager)
{
  m_wpDownloadManager = wpDownloadManager;
}

//----------------------------------------------------------------------------------------
//
void CWebResourceOverlay::Show(bool bAllowDownloading)
{
  COverlayBase::Show();
  m_bDownloadMode = bAllowDownloading;
  m_spUi->pAddAsLocalCheckBox->setVisible(bAllowDownloading);
}

//----------------------------------------------------------------------------------------
//
void CWebResourceOverlay::Climb()
{
  ClimbToFirstInstanceOf("CEditorMainScreen", false);
}

//----------------------------------------------------------------------------------------
//
void CWebResourceOverlay::Hide()
{
  m_bDownloadMode = false;
  COverlayBase::Hide();
}

//----------------------------------------------------------------------------------------
//
void CWebResourceOverlay::Resize()
{
  QPoint newPos =
      QPoint(m_pTargetWidget->geometry().width() / 2, m_pTargetWidget->geometry().height() / 2) -
      QPoint(width() / 2, height() / 2);

  move(newPos.x(), newPos.y());
  resize(width(), height());
}

//----------------------------------------------------------------------------------------
//
void CWebResourceOverlay::on_pUrlLineEdit_textChanged(const QString &text)
{
  if (auto spDownloadManager = m_wpDownloadManager.lock();
      nullptr != spDownloadManager && m_bDownloadMode)
  {
    m_spUi->pAddAsLocalCheckBox->setEnabled(
        spDownloadManager->CanDownloadAndSaveAsFile(QUrl::fromUserInput(text)));
  }
}

//----------------------------------------------------------------------------------------
//
void CWebResourceOverlay::on_pConfirmButton_clicked()
{
  const QString sResource = m_spUi->pUrlLineEdit->text();
  bool bDownload = m_spUi->pAddAsLocalCheckBox->isChecked();

  if (auto spDownloadManager = m_wpDownloadManager.lock();
      nullptr != spDownloadManager && m_bDownloadMode)
  {
    spDownloadManager->AddResource(sResource, bDownload);
  }
  else
  {
    emit SignalResourceSelected(sResource);
  }
}

//----------------------------------------------------------------------------------------
//
void CWebResourceOverlay::on_pCancelButton_clicked()
{
  Hide();
}
