#ifndef CAGECHECKOVERLAY_H
#define CAGECHECKOVERLAY_H

#include "OverlayBase.h"
#include <QObject>
#include <memory>

namespace Ui {
  class CAgeCheckOverlay;
}

class CAgeCheckOverlay : public COverlayBase
{
  Q_OBJECT

public:
  explicit CAgeCheckOverlay(QWidget* pParent = nullptr);
  ~CAgeCheckOverlay() override;

public slots:
  void Climb() override;
  void Resize() override;

private slots:
  void on_pConfirmButton_clicked();
  void on_pCancelButton_clicked();

private:
  std::unique_ptr<Ui::CAgeCheckOverlay> m_spUi;
};

#endif // CAGECHECKOVERLAY_H
