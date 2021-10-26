/*-------------------------------------------------------------------------------------*/
/*! Logic in this file is mostly a C++ conversion of fapnips openeos software, which can
 *  be found  here:
 *
 * https://github.com/fapnip/openeos
 *
 * Functions which have been directly converted retained their original names.
 **//*----------------------------------------------------------------------------------*/

#ifndef CEOSDOWNLOADJOB_H
#define CEOSDOWNLOADJOB_H

#include "DownloadJobRegistry.h"
#include "EosPagesToScenesTransformer.h"
#include "EosResourceLocator.h"

#include <QBuffer>
#include <QNetworkAccessManager>
#include <QObject>
#include <memory>

class RCCResourceLibrary;
typedef std::shared_ptr<struct SProject> tspProject;
typedef std::shared_ptr<struct SResource> tspResource;

//----------------------------------------------------------------------------------------
//
class CEosDownloadJobWidgetProvider : public IDownloadJobWidgetProvider
{
public:
  CEosDownloadJobWidgetProvider();
  ~CEosDownloadJobWidgetProvider();
  QWidget* operator()() const override;
};

//----------------------------------------------------------------------------------------
//
class CEosDownloadJob : public QObject, public IDownloadJob
{
  Q_OBJECT
  Q_INTERFACES(IDownloadJob)

  struct SScriptMetaData
  {
    QString m_sTitle;
    QString m_sAuthor;
    QString m_sAuthorId;
    QString m_sTeaseId;
    QString m_sDataKey;
  };

public:
  const QString c_sScriptFormat = ".eos";

  explicit CEosDownloadJob(QObject* pParent = nullptr);
  ~CEosDownloadJob() override;

  QString Error() override;
  bool Finished() override;
  QString JobName() const override;
  qint32 Progress() const override;
  bool Run(const QVariantList& args) override;

signals:
  void SignalFinished(qint32 iProjId) override;
  void SignalProgressChanged(qint32 iProjId, qint32 iProgress) override;
  void SignalStarted(qint32 iProjId) override;

protected:
  void AbortImpl() override;

private:
  bool CreateResourceFiles(const CEosResourceLocator::tGaleryData& resourceMap,
                           tspResource spScript, size_t uiScriptSize, qint32 iMaxProgress,
                           QBuffer& errorBuffer,
                           CEosResourceLocator& locator,
                           QString* psError);
  bool CreateScriptFiles(const std::vector<CEosPagesToScenesTransformer::SPageScene>& vScenes,
                         qint32 iMaxProgress,
                         QBuffer& errorBuffer,
                         CEosPagesToScenesTransformer& sceneTransformer,
                         QString* psError);
  QUrl EncodeForCorsProxy(const QUrl& url, const QString& sQuery);
  QByteArray Fetch(const QUrl& url, QString* psError);
  bool ParseHtml(const QByteArray& arr, SScriptMetaData& data, QString* psError);
  QString ParseTeaseURI(const QUrl& url);
  void ResetResourceLibrary(QBuffer& errorBuffer);
  bool RequestRemoteScript(const QUrl& url, QString& sId, QJsonDocument& jsonScript,
                           QString* psError);
  bool RequestRemoteScriptMetadata(const QString& sId, SScriptMetaData& data,
                                   QString* psError);
  bool ValidateJson(const QByteArray& arr, QString* psError);
  bool WriteResourceBlob(const tspProject& spProject, const QString& sName, qint32& iBlob);

  std::shared_ptr<QNetworkAccessManager> m_spNetworkAccessManager;
  std::shared_ptr<RCCResourceLibrary>    m_spResourceLib;
  tspProject                             m_spProject;
  qint32                                 m_iProgress = 0;
  QString                                m_sName;
  QString                                m_sError;
  qint32                                 m_iProjId;
  qint32                                 m_iFileBlobCounter = 0;
  qint32                                 m_iProgressCounter = 0;
};

DECLARE_DOWNLOADJOB(CEosDownloadJob, "EOS/hostcfg", {"milovana.com"})
//DECLARE_JOBSETTINGS_WIDGETPROVIDER(CEosDownloadJobWidgetProvider, CEosDownloadJob)

#endif // CEOSDOWNLOADJOB_H
