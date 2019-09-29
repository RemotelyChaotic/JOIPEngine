#ifndef SCRIPTRUNNERSIGNALEMITER_H
#define SCRIPTRUNNERSIGNALEMITER_H

#include <enum.h>
#include <QColor>
#include <qlogging.h>
#include <QObject>
#include <QString>
#include <memory>

BETTER_ENUM(EScriptExecutionStatus, qint32,
            eRunning = 0,
            eStopped = 1);

struct SResource;
typedef std::shared_ptr<SResource> tspResource;

class CScriptRunnerSignalEmiter : public QObject
{
  Q_OBJECT

public:
  CScriptRunnerSignalEmiter();
  ~CScriptRunnerSignalEmiter();

public:
  void SetScriptExecutionStatus(EScriptExecutionStatus status);
  EScriptExecutionStatus ScriptExecutionStatus();

signals:
  // generic / controll
  void SignalInterruptLoops();
  void SignalShowError(QString sError, QtMsgType type);

  // background
  void SignalBackgroundColorChanged(QColor color);
  void SignalBackgroundTextureChanged(tspResource spResource);

  // icon
  void SignalHideIcon(QString sIconIdentifier);
  void SignalShowIcon(tspResource spResource);

  // media Player
  void SignalPauseVideo();
  void SignalPauseSound();
  void SignalPlayMedia(tspResource spResource);
  void SignalShowMedia(tspResource spResource);
  void SignalStopVideo();
  void SignalStopSound();

  // TextBox
  void SignalClearText();
  void SignalShowButtonPrompts(QStringList vsLabels);
  void SignalShowButtonReturnValue(qint32 iIndex);
  void SignalShowInput();
  void SignalShowInputReturnValue(QString sValue);
  void SignalShowText(QString sText);
  void SignalTextBackgroundColorsChanged(std::vector<QColor> vColors);
  void SignalTextColorsChanged(std::vector<QColor> vColors);

  // Timer
  void SignalHideTimer();
  void SignalSetTime(qint32 iTimeS);
  void SignalSetTimeVisible(bool bVisible);
  void SignalShowTimer();
  void SignalStartTimer();
  void SignalStopTimer();
  void SignalWaitForTimer();
  void SignalTimerFinished();

  // Thread
  void SignalSkippableWait(qint32 iTimeS);
  void SignalWaitSkipped();

private:
  QAtomicInt     m_bScriptExecutionStatus;
};

#endif // SCRIPTRUNNERSIGNALEMITER_H
