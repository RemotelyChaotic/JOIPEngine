#ifndef THREADSNIPPETOVERLAY_H
#define THREADSNIPPETOVERLAY_H

#include "Widgets/OverlayBase.h"
#include <QWidget>
#include <memory>

namespace Ui {
  class CThreadSnippetOverlay;
}

class CThreadSnippetOverlay : public COverlayBase
{
  Q_OBJECT

public:
  explicit CThreadSnippetOverlay(QWidget* pParent = nullptr);
  ~CThreadSnippetOverlay() override;

signals:
  void SignalThreadCode(const QString& code);

public slots:
  void Climb() override;
  void Resize() override;

protected slots:
  void on_pSleepSpinBox_valueChanged(double dValue);
  void on_pSkippableCheckBox_toggled(bool bStatus);
  void on_pConfirmButton_clicked();
  void on_pCancelButton_clicked();

private:
  std::unique_ptr<Ui::CThreadSnippetOverlay> m_spUi;
  double                                     m_bSleepTimeS;
  bool                                       m_bSkippable;
};

#endif // THREADSNIPPETOVERLAY_H