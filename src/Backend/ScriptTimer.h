#ifndef SCRIPTTIMER_H
#define SCRIPTTIMER_H

#include <QJSEngine>
#include <QJSValue>

class CScriptTimer : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptTimer)

public:
  CScriptTimer(QJSEngine* pEngine);
  ~CScriptTimer();

public slots:
  void hide();
  void setTime(qint32 iTimeS);
  void setTimeHidden();
  void setTimeVisible();
  void show();
  void start();
  void stop();

private:
  QJSEngine*               m_pEngine;
};

#endif // SCRIPTTIMER_H
