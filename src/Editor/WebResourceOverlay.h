#ifndef WEBRESOURCEOVERLAY_H
#define WEBRESOURCEOVERLAY_H

#include "Widgets/OverlayBase.h"
#include <QPointer>
#include <QPushButton>
#include <memory>

namespace Ui {
  class CWebResourceOverlay;
}

class CWebResourceOverlay : public COverlayBase
{
  Q_OBJECT

public:
  explicit CWebResourceOverlay(QPushButton* pButtonTarget, QWidget* pParent = nullptr);
  ~CWebResourceOverlay() override;

  void Resize() override;

signals:
  void SignalResourceSelected(const QString& sResource);

protected slots:
  void on_pConfirmButton_clicked();

private:
  std::unique_ptr<Ui::CWebResourceOverlay> m_spUi;
  QPointer<QPushButton>                    m_pButtonTarget;
};

#endif // WEBRESOURCEOVERLAY_H