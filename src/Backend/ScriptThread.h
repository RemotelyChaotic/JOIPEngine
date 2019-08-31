#ifndef SCRIPTTHREAD_H
#define SCRIPTTHREAD_H

#include <QJSEngine>
#include <QJSValue>

class CScriptThread : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptThread)

public:
  CScriptThread(QJSEngine* pEngine);
  ~CScriptThread();

public slots:
  void sleep(qint32 iTimeMs);

private:
  QJSEngine*               m_pEngine;
};

#endif // SCRIPTTHREAD_H
