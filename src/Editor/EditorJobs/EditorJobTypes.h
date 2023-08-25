#ifndef EDITORJOBTYPES_H
#define EDITORJOBTYPES_H

#include "Systems/IRunnableJob.h"
#include <QString>

namespace editor_job
{
  const QString c_sExport = "Export";
  const QString c_sCompress = "Compress";
}

//----------------------------------------------------------------------------------------
//
class IEditorJob : public QObject, public IRunnableJob
{
  Q_OBJECT
  Q_INTERFACES(IRunnableJob)

public:
  ~IEditorJob() override {};

  virtual QString ReturnValue() const = 0;

signals:
  void SignalFinished(qint32 iId) override;
  void SignalProgressChanged(qint32 iId, qint32 iProgress) override;
  void SignalStarted(qint32 iId) override;
  void SignalJobMessage(qint32 iId, QString sType, QString sMsg);

protected:
  IEditorJob(QObject* pParent = nullptr) : QObject(pParent), IRunnableJob() {}

private:
  Q_DISABLE_COPY(IEditorJob)
};


#endif // EDITORJOBTYPES_H
