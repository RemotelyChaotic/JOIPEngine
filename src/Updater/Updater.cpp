#include "Updater.h"
#include "SettingsData.h"
#include "SVersion.h"
#include "Unzipper.h"

#include <nlohmann/json.hpp>

#include <QApplication>
#include <QAuthenticator>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QNetworkReply>
#include <QProcess>
#include <QStandardPaths>
#include <QTimer>

#include <future>
#include <thread>

Q_DECLARE_METATYPE(QAuthenticator*)

//----------------------------------------------------------------------------------------
//
class CGuiThreadObject : public QObject
{
  Q_OBJECT

public:
  explicit CGuiThreadObject(std::function<void()> fn) : QObject(), m_fnFunction(fn)
  {
    moveToThread(qApp->thread());
  }
  ~CGuiThreadObject() override {}

public slots:
  void Call()
  {
    if (m_fnFunction)
    {
      m_fnFunction();
    }
    deleteLater();
  }

signals:
  void Done();

private:
  std::function<void()> m_fnFunction;
};

#include "Updater.moc"

namespace
{
  const char c_sLlatestVersion[] = "latestVersion";
  const char c_sLatestVersionCode[] = "latestVersionCode";
  const char c_sLatestUpdaterVersion[] = "latestUpdaterVersion";
  const char c_sLatestUpdaterVersionCode[] = "latestUpdaterVersionCode";
  const char c_sUrl[] = "url";
  const char c_sReleaseNotes[] = "releaseNotes";

  const QString c_sFolderDownload = "JOIPEngine";
  const QString c_sFolderUnpacked = "JOIPEngine";
  const QString c_sFileDownload = "JOIPEngine.zip";
  const QString c_sBackupFolder = "backup";
  const QString c_sDataFolder = "data";
  const QString c_sStylesFolder = "styles";
  const QString c_sUpDaterFolder = "updater";

#if defined(Q_OS_WIN)
  const QString c_sCopyCmd = R"(/C /S "TIMEOUT /T 5 & xcopy "%1\updater" "%2\updater" /E/H/C/I & start "" "%3" -c -t=%4")";
#else
  // TODO: const QString c_sCopyCmd = ???
#endif

  //--------------------------------------------------------------------------------------
  //
  void RunInUiThread(std::function<void()> fn)
  {
    CGuiThreadObject* pObj = new CGuiThreadObject(fn);
    const bool bOk = QMetaObject::invokeMethod(pObj, "Call", Qt::QueuedConnection);
    assert(bOk); Q_UNUSED(bOk)
  }
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

  m_latestVersion = static_cast<QString>(m_pSettings->targetVersion).toStdString();
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
void CUpdater::ContinueUpdate()
{
  HandleState(EState::eContinuingUpdate, QByteArray());
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
    if ( statusCode.toInt() == 302)
    {
      QVariant redirectionTargetUrl = pReply->attribute(QNetworkRequest::RedirectionTargetAttribute);
      QNetworkRequest request(redirectionTargetUrl.toUrl());
      QNetworkReply* pReply = m_pManager->get(request);
      connect(pReply, &QNetworkReply::downloadProgress, this,
              [this](qint64 iBytesReceived, qint64 iBytesTotal) {
                double dPercent = 0 == iBytesTotal ?
                                      0.0 :
                                      100.0 * static_cast<double>(iBytesReceived) / iBytesTotal;
                emit SignalMessage(tr("Downloading...%1%").arg(static_cast<qint32>(dPercent)));
                emit SignalProgress(static_cast<qint32>(dPercent), 100);
              }); // Performs the function
      return;
    }
    else if (statusCode.toInt() != 200)
    {
      HandleError(tr("Network request error: %1")
                      .arg(pReply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString()));
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
    case EState::eUnpacking:
      sState = tr("extracting engine data");
      break;
    case EState::eContinuingUpdate:
      sState = tr("during Update");
      break;
    case EState::eCopyingFiles:
      sState = tr("copying files");
      break;
    case EState::eFinished:
      sState = tr("finishing update");
      break;
  }

  m_bShuttingDown = true;
  emit SignalMessage(tr("Error %1: %2.").arg(sState).arg(sError));
  using namespace std::chrono_literals;

  RunInUiThread([this]() {
    QTimer::singleShot(5s, this, [this]() {
      emit SignalStartExe();
      });
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
    case EState::eUnpacking:
      break;
    case EState::eContinuingUpdate:
      CopyFiles();
      break;
    case EState::eCopyingFiles:
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
    double dPercent = 0 == iBytesTotal ?
                          0.0 :
                          100.0 * static_cast<double>(iBytesReceived) / iBytesTotal;
    emit SignalMessage(tr("Fetching latest-version.json...%1%").arg(static_cast<qint32>(dPercent)));
    emit SignalProgress(static_cast<qint32>(dPercent), 100);
  }); // Performs the function
}

//----------------------------------------------------------------------------------------
//
void CUpdater::EvaluateJson(const QByteArray& arr)
{
  m_pCurrentState = EState::eEvaluateJson;
  emit SignalMessage(tr("Reading latest-version.json..."));
  emit SignalProgress(0, -1);

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

  // DEBUG:
  //bUpdate = true;
  if (bUpdate)
  {
    //https://github.com/RemotelyChaotic/JOIPEngine/releases/download/v1.3.1/JOIPEngine_1_3_1.zip
    const QString sLatestVersion = QString::fromStdString(m_latestVersion);
    const QString sFormatedVersion = QString(sLatestVersion).replace(".", "_");
    QString sUrl = QString("%1/v%2/JOIPEngine_%3%4.zip")
                       .arg(QString::fromStdString(static_cast<std::string>(itUrl.value())))
                       .arg(sLatestVersion)
#if defined(DOWNLOAD_POSTFIX)
                       .arg(DOWNLOAD_POSTFIX);
#else
                        .arg("")
#endif
                        .arg(sFormatedVersion);
    HandleState(EState::eStartDownloadEngineFiles, sUrl.toUtf8());
  }
  else
  {
    FinishingUpdate(static_cast<QString>(m_pSettings->version).toUtf8(), false);
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
            emit SignalMessage(tr("Fetching engine data...%1%%").arg(static_cast<qint32>(dPercent)));
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

  QFileInfo info(vsLocations[0] + "/" + c_sFolderDownload);
  if (!info.exists())
  {
    bool bOk = QDir(vsLocations[0]).mkpath(c_sFolderDownload);
    if (!bOk)
    {
      HandleError(tr("Could not create <temp>/%1 folder").arg(c_sFolderDownload));
      return;
    }
  }

  const QString sPath = vsLocations[0] + "/" + c_sFolderDownload + "/" + c_sFileDownload;
  QFile info2(sPath);
  if (!info2.open(QIODevice::ReadWrite | QIODevice::Truncate))
  {
    HandleError(tr("Could not open <temp>/%1/%2 for writing").arg(c_sFolderDownload).arg(c_sFileDownload));
    return;
  }

  info2.write(arr);
  info2.close();

  UnpackEngine(sPath);
}

//----------------------------------------------------------------------------------------
//
void CUpdater::FinishingUpdate(const QString& sVersion, bool bUpdated)
{
  m_pCurrentState = EState::eFinished;

  emit SignalProgress(0, 100);

  if (bUpdated)
  {
    emit SignalMessage(tr("Finished update to version %1").arg(sVersion));
  }
  else
  {
    emit SignalMessage(tr("Engine and launcher are up-to-date: version %1").arg(sVersion));
  }
  using namespace std::chrono_literals;
  QTimer::singleShot(5s, this, [this]() {
    emit SignalStartExe();
  });
}

//----------------------------------------------------------------------------------------
//
void CUpdater::UnpackEngine(const QString& sFrom)
{
  m_pCurrentState = EState::eUnpacking;

  emit SignalMessage(tr("Extracting..."));

  QString sErr;
  bool bOk = zipper::Unzip(sFrom, [this](const QString& sMsg) {
        emit SignalMessage(sMsg);
      }, [this](qint32 iCurrent, qint32 iMax) {
        emit SignalProgress(iCurrent, iMax);
      }, &sErr);
  if (!bOk)
  {
    HandleError(tr("Could not extract %1: %2").arg(sFrom).arg(sErr));
    return;
  }

  QFileInfo infoArch(sFrom);
  const QString sUnpacked = infoArch.absolutePath() + "/" + c_sFolderUnpacked;

  // remove zip file
  if (!QFile(infoArch.absoluteFilePath()).remove())
  {
    emit SignalMessage(tr("Could not remove old archive."));
  }

  if (m_bUpdateLauncher)
  {
    // start cmd to copy the updater and then start it
    const QString thisExe = QCoreApplication::applicationFilePath();
    QProcess proc;
    bool bOk =
        proc.startDetached("cmd", QStringList() <<
                                             c_sCopyCmd.arg(sUnpacked)
                                                       .arg(QFileInfo(thisExe).absolutePath() + "/../")
                                                       .arg(thisExe)
                                                       .arg(QString::fromStdString(m_latestVersion)));
    if (!bOk)
    {
      HandleError(tr("Could not copy updater files: %1").arg(proc.errorString()));
      return;
    }
    else
    {
      qApp->quit();
    }
  }
  else
  {
    emit SignalProgress(0, -1);
    ContinueUpdate();
  }
}

//----------------------------------------------------------------------------------------
//
void CUpdater::CopyFiles()
{
  m_pCurrentState = EState::eCopyingFiles;

  emit SignalMessage(tr("Patching..."));

  std::packaged_task<bool()> task([this]() -> bool
  {
    QStringList vsLocations =
        QStandardPaths::standardLocations(QStandardPaths::TempLocation);
    if (vsLocations.size() == 0)
    {
      HandleError(tr("Could not get temp folder"));
      return false;
    }

    QFileInfo info(vsLocations[0] + "/" + c_sFolderDownload + "/" + c_sFolderUnpacked);
    if (!info.exists())
    {
      HandleError(tr("Could not patch engine: files not found"));
      return false;
    }

    emit SignalMessage(tr("Patching... backup local files..."));

    const QString sThisRoot = QFileInfo(QCoreApplication::applicationDirPath() + "/..").absoluteFilePath();
    QDirIterator iterThis(sThisRoot,
                          QDir::NoDotAndDotDot | QDir::Dirs,
                          QDirIterator::NoIteratorFlags);
    while (iterThis.hasNext())
    {
      QString sPath = iterThis.next();
      QString sPathShortened = sPath;
      if (!sThisRoot.endsWith("/"))
      {
        sPathShortened = sPath.replace(sThisRoot, "");
      }
      else
      {
        sPathShortened = sPath.replace(sThisRoot + "/", "");
      }

      if (!sPathShortened.startsWith(c_sUpDaterFolder) &&
          !sPathShortened.startsWith(c_sDataFolder) &&
          !sPathShortened.startsWith(c_sStylesFolder) &&
          !sPathShortened.startsWith(c_sBackupFolder))
      {
        QDir dir(sPath);
        bool bOk = QFile::rename(sPath, sThisRoot + "/" + c_sBackupFolder + "/" + dir.dirName());
        if (!bOk)
        {
          HandleError(tr("Could backup folder %1").arg(sPath));
          return false;
        }
      }
    }

    emit SignalMessage(tr("Patching... gathering files..."));

    // gather the files to copy
    QStringList vsData;
    const QString sAbsolutePath = info.absolutePath();
    QDirIterator iter(info.absoluteFilePath(),
                      QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files,
                      QDirIterator::Subdirectories);
    while (iter.hasNext())
    {
      QString sPath = iter.next();
      QString sPathShortened = sPath;
      if (!sAbsolutePath.endsWith("/"))
      {
        sPathShortened = sPath.replace(sAbsolutePath, "");
      }
      else
      {
        sPathShortened = sPath.replace(sAbsolutePath + "/", "");
      }

      if (!sPathShortened.startsWith(c_sUpDaterFolder) &&
          !sPathShortened.startsWith(c_sDataFolder))
      {
        vsData << sPathShortened;
      }
    }

    // copy everything
    for (qint32 i = 0; vsData.size() > i; ++i)
    {
      emit SignalMessage(tr("Patching...%1/%2").arg(i+1).arg(vsData.size()));
      emit SignalProgress(i+1, vsData.size());

      QFileInfo info(sAbsolutePath + "/" + vsData[i]);
      if (info.isDir())
      {
        if (!QDir(sThisRoot).mkpath(vsData[i]))
        {
          HandleError(tr("Could not create directory %1").arg(vsData[i]));
          return false;
        }
      }
      else
      {
        QFile file(info.absoluteFilePath());
        if (!file.copy(sThisRoot + "/" + vsData[i]))
        {
          HandleError(tr("Could not create directory %1").arg(vsData[i]));
          return false;
        }
      }
    }

    emit SignalMessage(tr("Finishing up..."));

    // removing Download folder
    QDir downloadDir(vsLocations[0] + "/" + c_sFolderDownload);
    if (!downloadDir.removeRecursively())
    {
      HandleError(tr("Could not remove download directory"));
      return false;
    }

    // removing backup folder
    QDir backupDir(sThisRoot + "/" + c_sBackupFolder);
    if (!backupDir.removeRecursively())
    {
      HandleError(tr("Could not remove backup directory"));
      return false;
    }

    return true;
  });

  std::future<bool> future = task.get_future();
  std::thread t(std::move(task));

  while (future.wait_for(std::chrono::seconds(0)) != std::future_status::ready)
  {
    qApp->processEvents();
  }

  t.join();
  if (!future.get())
  {
    return;
  }

  FinishingUpdate(QString::fromStdString(m_latestVersion).toUtf8(), true);
}
