#ifndef CEDITORIMAGECOMPRESSIONJOB_H
#define CEDITORIMAGECOMPRESSIONJOB_H

#include "EditorJobTypes.h"

#include <QObject>

#include <memory>

typedef std::shared_ptr<struct SProject> tspProject;

class CEditorImageCompressionJob : public IEditorJob
{
  Q_OBJECT

public:
  CEditorImageCompressionJob(QObject* pParent = nullptr);
  ~CEditorImageCompressionJob() override;

  QString Error() const override;
  bool Finished() const override;
  bool HasError() const override;
  qint32 Id() const override;
  QString JobName() const override;
  QString JobType() const override;
  qint32 Progress() const override;
  QString ReturnValue() const override;

  bool Run(const QVariantList& args) override;
  void Stop() override;

protected:
  void AbortImpl() override;

  tspProject                             m_spProject;
  qint32                                 m_iId = -1;
  qint32                                 m_iProgress = 0;
  QString                                m_sName;
  QString                                m_sError;
  QString                                m_sReturnValue;
  bool                                   m_bHasError;
  bool                                   m_bFinished;
};

#endif // CEDITORIMAGECOMPRESSIONJOB_H
