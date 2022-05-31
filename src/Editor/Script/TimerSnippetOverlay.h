#ifndef TIMERSNIPPETOVERLAY_H
#define TIMERSNIPPETOVERLAY_H

#include "Widgets/OverlayBase.h"
#include <QPointer>
#include <QTime>
#include <memory>

class CScriptEditorWidget;
namespace Ui {
  class CTimerSnippetOverlay;
}

struct STimerSnippetData
{
  bool m_bSetTime = false;
  qint32 m_iTimeS = 0;
  bool m_bShow = false;
  bool m_bHide = false;
  bool m_bTimerVisible = false;
  bool m_bStart = false;
  bool m_bStop = false;
  bool m_bWait = false;
};

//----------------------------------------------------------------------------------------
//
class CTimerSnippetOverlay : public COverlayBase
{
  Q_OBJECT

public:
  explicit CTimerSnippetOverlay(CScriptEditorWidget *parent = nullptr);
  ~CTimerSnippetOverlay() override;

signals:
  void SignalTimerCode(const QString& code);

public slots:
  void Climb() override;
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
  QPointer<CScriptEditorWidget>             m_pEditor;
  STimerSnippetData                         m_data;
  QSize                                     m_preferredSize;
};

#endif // TIMERSNIPPETOVERLAY_H
