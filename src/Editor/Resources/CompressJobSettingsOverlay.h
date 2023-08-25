#ifndef COMPRESSJOBSETTINGSOVERLAY_H
#define COMPRESSJOBSETTINGSOVERLAY_H

#include "Widgets/OverlayBase.h"

#include <memory>

namespace Ui {
  class CCompressJobSettingsOverlay;
}

class CCompressJobSettingsOverlay : public COverlayBase
{
  Q_OBJECT

public:
  struct SCompressJobSettings
  {
    qint32 m_iCompression;
  };

  explicit CCompressJobSettingsOverlay(QWidget* pParent = nullptr);
  ~CCompressJobSettingsOverlay();

public slots:
  void Climb() override;
  void Resize() override;

signals:
  void SignalJobSettingsConfirmed(const SCompressJobSettings& settings);

protected slots:
  void on_pConfirmButton_clicked();
  void on_pCancelButton_clicked();

private:
  std::unique_ptr<Ui::CCompressJobSettingsOverlay> m_spUi;
};

Q_DECLARE_METATYPE(CCompressJobSettingsOverlay::SCompressJobSettings)

#endif // COMPRESSJOBSETTINGSOVERLAY_H
