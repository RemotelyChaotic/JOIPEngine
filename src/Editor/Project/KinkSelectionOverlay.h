#ifndef KINKSELECTIONOVERLAY_H
#define KINKSELECTIONOVERLAY_H

#include "Widgets/OverlayBase.h"
#include <memory>

class CKinkTreeModel;
namespace Ui {
  class CKinkSelectionOverlay;
}

class CKinkSelectionOverlay : public COverlayBase
{
  Q_OBJECT

public:
  explicit CKinkSelectionOverlay(QWidget* pParent = nullptr);
  ~CKinkSelectionOverlay() override;

  void Initialize(CKinkTreeModel* pKinkTreeModel);
  void LoadProject(bool bReadOnly);
  void UnloadProject();

public slots:
  void Climb() override;
  void Resize() override;

protected slots:
  void on_pFilter_SignalFilterChanged(const QString& sText);
  void on_pConfirmButton_clicked();

private:
  std::unique_ptr<Ui::CKinkSelectionOverlay> m_spUi;
  bool                                       m_bInitialized;
};

#endif // KINKSELECTIONOVERLAY_H
