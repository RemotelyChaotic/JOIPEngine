#ifndef IEDITORJOBSTATELISTENER_H
#define IEDITORJOBSTATELISTENER_H

#include <QString>

class IEditorJobStateListener
{
public:
  virtual void JobFinished(qint32 iId) = 0;
  virtual void JobStarted(qint32 iId) = 0;
  virtual void JobMessage(qint32 iId, const QString& sMsg) = 0;
  virtual void JobProgressChanged(qint32 iId, qint32 iProgress) = 0;

protected:
  virtual ~IEditorJobStateListener() {}
};

#endif // IEDITORJOBSTATELISTENER_H
