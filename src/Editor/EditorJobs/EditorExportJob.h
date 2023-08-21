#ifndef CEDITOREXPORTJOB_H
#define CEDITOREXPORTJOB_H

#include "EditorJobTypes.h"

#include <QObject>
#include <QProcess>
#include <QUrl>

#include <memory>

typedef std::shared_ptr<struct SProject> tspProject;

class CEditorExportJob : public QObject, public IEditorJob
{
  Q_OBJECT
  Q_INTERFACES(IRunnableJob)

public:
  explicit CEditorExportJob(QObject* pParent = nullptr);
  ~CEditorExportJob() override;

  enum class EExportFormat : qint32 {
    eArchive,
    eBinary
  };

  enum class EExportError : qint32 {
    eWriteFailed,
    eCleanupFailed,
    eProcessError
  };
  Q_ENUM(EExportError)

  QString Error() const override;
  bool Finished() const override;
  bool HasError() const override;
  qint32 Id() const override;
  QString JobName() const override;
  QString JobType() const override;
  qint32 Progress() const override;
  QString ReturnValue() const override;

  bool Run(const QVariantList& args) override;
  bool RunBinaryExport(const QString& sName, const QString& sFolder);
  bool RunZipExport(const QString& sName, const QString& sFolder);
  void Stop() override;

signals:
  void SignalFinished(qint32 iId) override;
  void SignalProgressChanged(qint32 iId, qint32 iProgress) override;
  void SignalStarted(qint32 iId) override;
  void SignalJobMessage(qint32 iId, QString sType, QString sMsg);

protected:
  void AbortImpl() override;
  void CreateProcess();

protected slots:
  void SlotExportErrorOccurred(QProcess::ProcessError error);
  void SlotExportFinished(int exitCode, QProcess::ExitStatus exitStatus);
  void SlotExportStarted();
  void SlotExportStateChanged(QProcess::ProcessState newState);
  void SlotReadErrorOut();
  void SlotReadStandardOut();

protected:
  struct SExportFile
  {
    QString                   m_sName;
    QString                   m_sPath;
  };

  std::unique_ptr<QProcess>              m_spExportProcess;
  tspProject                             m_spProject;
  qint32                                 m_iId = -1;
  qint32                                 m_iProgress = 0;
  QString                                m_sName;
  QString                                m_sError;
  QString                                m_sReturnValue;
  bool                                   m_bHasError;
  bool                                   m_bFinished;
};

#endif // CEDITOREXPORTJOB_H
