#ifndef TIMERSNIPPETOVERLAY_H
#define TIMERSNIPPETOVERLAY_H

#include "CodeSnippetOverlayBase.h"
#include <QPointer>
#include <QTime>
#include <memory>

class CScriptEditorWidget;
namespace Ui {
  class CTimerSnippetOverlay;
}

//----------------------------------------------------------------------------------------
//
class CTimerSnippetOverlay : public CCodeSnippetOverlayBase
{
  Q_OBJECT

public:
  explicit CTimerSnippetOverlay(QWidget *parent = nullptr);
  ~CTimerSnippetOverlay() override;

public slots:
  void Resize() override;

protected slots:
  void on_pSetTimeCheckBox_toggled(bool bStatus);
  void on_pTimeEdit_timeChanged(const QTime &time);
  void on_pShowCheckBox_toggled(bool bStatus);
  void on_pHideCheckBox_toggled(bool bStatus);
  void on_pDisplayCheckBox_toggled(bool bStatus);
  void on_pStartCheckBox_toggled(bool bStatus);
  void on_pStopCheckBox_toggled(bool bStatus);
  void on_pWaitCheckBox_toggled(bool bStatus);
  void on_pConfirmButton_clicked();
  void on_pCancelButton_clicked();

private:
  std::unique_ptr<Ui::CTimerSnippetOverlay> m_spUi;
  STimerSnippetData                         m_data;
  QSize                                     m_preferredSize;
};

#endif // TIMERSNIPPETOVERLAY_H
