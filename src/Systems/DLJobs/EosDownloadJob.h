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
#include <QNetworkAccessManager>
#include <QObject>
#include <memory>

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

public:
  explicit CEosDownloadJob(QObject* pParent = nullptr);
  ~CEosDownloadJob() override;

  QString Error() override;
  bool Finished() override;
  QString JobName() const override;
  qint32 Progress() const override;
  bool Run(const QVariantList& args) override;

signals:
  void SignalFinished() override;
  void SignalProgressChanged(qint32 iProgress) override;
  void SignalStarted() override;

protected:
  void AbortImpl() override;

private:
  QUrl EncodeForCorsProxy(const QUrl& url, const QString& sQuery);
  QByteArray Fetch(const QUrl& url);
  QString ParseTeaseURI(const QUrl& url);
  bool RequestRemoteScript(const QUrl& url, QString& sId, QJsonDocument& jsonScript);

  std::shared_ptr<QNetworkAccessManager> m_spNetworkAccessManager;
  qint32                                 m_iProgress = 0;
  QString                                m_sName;
  QString                                m_sError;
};

DECLARE_DOWNLOADJOB(CEosDownloadJob, "EOS/hostcfg", {"milovana.com"})
//DECLARE_JOBSETTINGS_WIDGETPROVIDER(CEosDownloadJobWidgetProvider, CEosDownloadJob)

#endif // CEOSDOWNLOADJOB_H
