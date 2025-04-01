#ifndef CEDITOREXPORTJOB_H
#define CEDITOREXPORTJOB_H

#include "EditorJobTypes.h"

#include <QObject>
#include <QProcess>
#include <QUrl>

#include <memory>

typedef std::shared_ptr<struct SProject> tspProject;

class CEditorExportJob : public IEditorJob
{
  Q_OBJECT

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

protected:
  void AbortImpl() override;

protected:
  struct SExportFile
  {
    QString                   m_sName;
    QString                   m_sPath;
  };

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
