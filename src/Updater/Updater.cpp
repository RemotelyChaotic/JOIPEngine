#include "Updater.h"
#include "SettingsData.h"
#include "SVersion.h"

#include <nlohmann/json.hpp>

#include <QApplication>
#include <QAuthenticator>
#include <QDir>
#include <QFileInfo>
#include <QNetworkReply>
#include <QStandardPaths>
#include <QTimer>

Q_DECLARE_METATYPE(QAuthenticator*)

namespace
{
  const char c_sLlatestVersion[] = "latestVersion";
  const char c_sLatestVersionCode[] = "latestVersionCode";
  const char c_sLatestUpdaterVersion[] = "latestUpdaterVersion";
  const char c_sLatestUpdaterVersionCode[] = "latestUpdaterVersionCode";
  const char c_sUrl[] = "url";
  const char c_sReleaseNotes[] = "releaseNotes";
}

//----------------------------------------------------------------------------------------
//
CUpdater::CUpdater(SSettingsData* pSettings, QObject* pParent) :
  QObject{pParent},
  m_pManager(new QNetworkAccessManager(this)),
  m_pSettings(pSettings)
{
  qRegisterMetaType<QAuthenticator*>();
  qRegisterMetaType<QNetworkReply*>();
  qRegisterMetaType<QList<QSslError>>();
  connect(m_pManager, &QNetworkAccessManager::authenticationRequired,
          this, &CUpdater::SlotAuthenticationRequired);
  connect(m_pManager, &QNetworkAccessManager::encrypted,
          this, &CUpdater::SlotEncrypted);
  connect(m_pManager, &QNetworkAccessManager::finished,
          this, &CUpdater::SlotReplyFinished);
  connect(m_pManager, &QNetworkAccessManager::sslErrors,
          this, &CUpdater::SlotSslErrors);
}
CUpdater::~CUpdater()
{

}

//----------------------------------------------------------------------------------------
//
void CUpdater::RunUpdate()
{
  HandleState(EState::eNone, QByteArray());
}

//----------------------------------------------------------------------------------------
//
void CUpdater::SlotAuthenticationRequired(QNetworkReply*,
                                          QAuthenticator*)
{

}

//----------------------------------------------------------------------------------------
//
void CUpdater::SlotEncrypted(QNetworkReply*)
{

}

//----------------------------------------------------------------------------------------
//
void CUpdater::SlotReplyFinished(QNetworkReply* pReply)
{
  if (nullptr != pReply)
  {
    pReply->deleteLater();
    if (pReply->error() != QNetworkReply::NoError)
    {
      HandleError(tr("Network request error: %1")
                      .arg(pReply->errorString()));
      return;
    }

    QVariant statusCode = pReply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    if (statusCode.toInt() != 200)
    {
      HandleError(tr("Network request error: %1")
                      .arg(pReply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString()));
      return;
    }
    else if ( statusCode.toInt() == 302 )
    {
      QVariant redirectionTargetUrl = pReply->attribute(QNetworkRequest::RedirectionTargetAttribute);
      QNetworkRequest request(redirectionTargetUrl.toUrl());
      m_pManager->get(request);
      return;
    }

    HandleState(m_pCurrentState, pReply->readAll());
  }
  else
  {
    HandleError(tr("Network request error: no reply"));
  }
}

//----------------------------------------------------------------------------------------
//
void CUpdater::SlotSslErrors(QNetworkReply*, const QList<QSslError>& errors)
{
  QStringList vsErr;
  for (QSslError err : qAsConst(errors))
  {
    vsErr << err.errorString();
  }

  HandleError(tr("Ssl error(s): %1").arg(vsErr.join("; ")));
}

//----------------------------------------------------------------------------------------
//
void CUpdater::HandleError(const QString& sError)
{
  QString sState;
  switch (m_pCurrentState)
  {
    case EState::eNone:
      sState = tr("during Update");
      break;
    case EState::eFetchJson:
      sState = tr("fetching latest-version.json");
      break;
    case EState::eEvaluateJson:
      sState = tr("reading latest-version.json");
      break;
    case EState::eStartDownloadEngineFiles:
      sState = tr("downloading engine data");
      break;
    case EState::eDownloadEngineFiles:
      sState = tr("downloading engine data");
      break;
    case EState::eFinished:
      sState = tr("finishing update");
      break;
  }

  m_bShuttingDown = true;
  emit SignalMessage(tr("Error %1: %2.").arg(sState).arg(sError));
  using namespace std::chrono_literals;
  QTimer::singleShot(5s, this, [this]() {
    emit SignalStartExe();
  });
}

//----------------------------------------------------------------------------------------
//
void CUpdater::HandleState(EState state, const QByteArray& data)
{
  if (m_bShuttingDown) { return; }

  switch (state)
  {
    case EState::eNone:
      FetchJson();
      break;
    case EState::eFetchJson:
      EvaluateJson(data);
      break;
    case EState::eEvaluateJson: break;
    case EState::eStartDownloadEngineFiles:
      DownloadEngineData(QString::fromUtf8(data));
      break;
    case EState::eDownloadEngineFiles:
      DownloadedEngineData(data);
      break;
    case EState::eFinished:
      FinishingUpdate(QString::fromUtf8(data));
      break;
  }
}

//----------------------------------------------------------------------------------------
//
void CUpdater::FetchJson()
{
  m_pCurrentState = EState::eFetchJson;
  emit SignalMessage(tr("Fetching latest-version.json..."));

  QNetworkRequest request;
  request.setUrl(QUrl(c_sLinkJson));
  QNetworkReply* pReply = m_pManager->get(request);

  connect(pReply, &QNetworkReply::downloadProgress, this,
          [this](qint64 iBytesReceived, qint64 iBytesTotal) {
    double dPercent = 0 == iBytesTotal ? 0 : static_cast<double>(iBytesReceived) / iBytesTotal;
    emit SignalMessage(tr("Fetching latest-version.json...%1%%").arg(dPercent));
  }); // Performs the function
}

//----------------------------------------------------------------------------------------
//
void CUpdater::EvaluateJson(const QByteArray& arr)
{
  m_pCurrentState = EState::eEvaluateJson;
  emit SignalMessage(tr("Reading latest-version.json..."));

  nlohmann::json json;
  try
  {
    json = nlohmann::json::parse(QString::fromUtf8(arr).toStdString());
  }
  catch (const std::exception &e)
  {
    HandleError(e.what());
    return;
  }

  auto itLatestVersion = json.find(c_sLlatestVersion);
  auto itLatestVersionCode = json.find(c_sLatestVersionCode);
  auto itLatestUpdaterVersion = json.find(c_sLatestUpdaterVersion);
  auto itLatestUpdaterVersionCode = json.find(c_sLatestUpdaterVersionCode);
  auto itUrl = json.find(c_sUrl);
  auto itReleaseNotes = json.find(c_sReleaseNotes);
  Q_UNUSED(itReleaseNotes)

  if (json.end() == itLatestVersion || !itLatestVersion.value().is_string())
  {
    HandleError(tr("%1 is missing or could not be read").arg(c_sLlatestVersion));
    return;
  }
  if (json.end() == itLatestVersionCode || !itLatestVersionCode.value().is_number())
  {
    HandleError(tr("%1 is missing or could not be read").arg(c_sLatestVersionCode));
    return;
  }
  if (json.end() == itLatestUpdaterVersion || !itLatestUpdaterVersion.value().is_string())
  {
    HandleError(tr("%1 is missing or could not be read").arg(c_sLatestUpdaterVersion));
    return;
  }
  if (json.end() == itLatestUpdaterVersionCode || !itLatestUpdaterVersionCode.value().is_number())
  {
    HandleError(tr("%1 is missing or could not be read").arg(c_sLatestUpdaterVersionCode));
    return;
  }
  if (json.end() == itUrl || !itUrl.value().is_string())
  {
    HandleError(tr("%1 is missing or could not be read").arg(c_sUrl));
    return;
  }

  qint32 iLatestVersionCode = itLatestVersionCode.value();
  qint32 iLatestUpdaterVersionCode = itLatestUpdaterVersionCode.value();
  qint32 iCompareVersion = m_pSettings->version;
  qint32 iCompareUpdaterVersion = SVersion(VERSION_XYZ);

  m_latestVersion = itLatestVersion.value();
  m_latestUpdaterVersion = itLatestUpdaterVersion.value();

  bool bUpdate = false;
  if (iLatestUpdaterVersionCode > iCompareUpdaterVersion)
  {
    bUpdate = true;
    m_bUpdateLauncher = true;
  }
  if (iLatestVersionCode > iCompareVersion)
  {
    bUpdate = true;
    m_bUpdateLauncher = false;
  }

  if (bUpdate)
  {
    //https://github.com/RemotelyChaotic/JOIPEngine/releases/download/v1.3.1/JOIPEngine_1_3_1.zip
    QString sLatestVersion = QString::fromStdString(m_latestVersion);
    QString sUrl = QString("%1/v%2/JOIPEngine_%4%3.zip")
                       .arg(QString::fromStdString(static_cast<std::string>(itUrl.value())))
                       .arg(sLatestVersion).arg(QString(sLatestVersion).replace(".", "_"))
#if defined(DOWNLOAD_POSTFIX)
                       .arg(DOWNLOAD_POSTFIX);
#else
                        .arg("");
#endif
    HandleState(EState::eStartDownloadEngineFiles, sUrl.toUtf8());
  }
  else
  {
    HandleState(EState::eFinished,
                QString::fromStdString(m_latestVersion).toUtf8());
  }
}

//----------------------------------------------------------------------------------------
//
void CUpdater::DownloadEngineData(const QString& sUrl)
{
  m_pCurrentState = EState::eDownloadEngineFiles;

  emit SignalMessage(tr("Fetching engine data..."));

  QNetworkRequest request;
  request.setUrl(QUrl(sUrl));
  QNetworkReply* pReply = m_pManager->get(request);

  connect(pReply, &QNetworkReply::downloadProgress, this,
          [this](qint64 iBytesReceived, qint64 iBytesTotal) {
            double dPercent = 0 == iBytesTotal ? 0 : static_cast<double>(iBytesReceived) / iBytesTotal;
            emit SignalMessage(tr("Fetching engine data...%1%%").arg(dPercent));
          }); // Performs the function
}

//----------------------------------------------------------------------------------------
//
void CUpdater::DownloadedEngineData(const QByteArray& arr)
{
  QStringList vsLocations =
      QStandardPaths::standardLocations(QStandardPaths::TempLocation);
  if (vsLocations.size() == 0)
  {
    HandleError(tr("Could not get temp folder"));
    return;
  }

  QFileInfo info(vsLocations[0] + "/JOIPEngine");
  if (!info.exists())
  {
    bool bOk = QDir().mkpath(info.absolutePath());
    if (!bOk)
    {
      HandleError(tr("Could not create <temp>/JOIPEngine folder"));
      return;
    }
  }

  QFile info2(vsLocations[0] + "/JOIPEngine/JOIPEngine.zip");
  if (!info2.open(QIODevice::ReadWrite | QIODevice::Truncate))
  {
    HandleError(tr("Could not open <temp>/JOIPEngine/JOIPEngine.zip for writing"));
    return;
  }
}

//----------------------------------------------------------------------------------------
//
void CUpdater::FinishingUpdate(const QString& sVersion)
{
  m_pCurrentState = EState::eFinished;

  emit SignalMessage(tr("Finished update to version %1").arg(sVersion));
  using namespace std::chrono_literals;
  QTimer::singleShot(5s, this, [this]() {
    emit SignalStartExe();
  });
}
