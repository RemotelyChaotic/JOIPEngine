#ifndef WEBRESOURCEOVERLAY_H
#define WEBRESOURCEOVERLAY_H

#include "Widgets/OverlayBase.h"
#include <QPointer>
#include <QPushButton>
#include <memory>

class CWebResourceDownloadManager;
namespace Ui {
  class CWebResourceOverlay;
}

class CWebResourceOverlay : public COverlayBase
{
  Q_OBJECT

public:
  explicit CWebResourceOverlay(QWidget* pParent = nullptr);
  ~CWebResourceOverlay() override;

  void SetDownloadManager(std::weak_ptr<CWebResourceDownloadManager> wpDownloadManager);

public slots:
  void Show(bool bAllowDownloading);
  void Climb() override;
  void Hide() override;
  void Resize() override;

signals:
  void SignalResourceSelected(const QString& sResource);

protected slots:
  void on_pUrlLineEdit_textChanged(const QString &text);
  void on_pConfirmButton_clicked();
  void on_pCancelButton_clicked();

private:
  std::unique_ptr<Ui::CWebResourceOverlay> m_spUi;
  std::weak_ptr<CWebResourceDownloadManager> m_wpDownloadManager;
  QSize                                      m_preferredSize;
  bool                                       m_bDownloadMode = false;
};

#endif // WEBRESOURCEOVERLAY_H
