#ifndef CUPDATER_H
#define CUPDATER_H

#include <QObject>
#include <QPointer>
#include <QNetworkAccessManager>

#include <atomic>

struct SSettingsData;

constexpr char c_sLinkJson[] =
    "https://raw.githubusercontent.com/RemotelyChaotic/JOIPEngine/master/changelog/latest-version.json";

class CUpdater : public QObject
{
  Q_OBJECT

public:
  explicit CUpdater(SSettingsData* pSettings, QObject* pParent = nullptr);
  ~CUpdater() override;

  void RunUpdate();
  void ContinueUpdate();

signals:
  void SignalMessage(const QString& sMsg);
  void SignalProgress(qint32 iCurrent, qint32 iMax);
  void SignalStartExe();

private slots:
  void SlotAuthenticationRequired(QNetworkReply* pReply, QAuthenticator* pAuthenticator);
  void SlotEncrypted(QNetworkReply* pReply);
  void SlotReplyFinished(QNetworkReply* pReply);
  void SlotSslErrors(QNetworkReply* pReply, const QList<QSslError>& errors);

private:
  enum EState
  {
    eNone = 0,
    eFetchJson,
    eEvaluateJson,
    eStartDownloadEngineFiles,
    eDownloadEngineFiles,
    eUnpacking,
    eContinuingUpdate,
    eCopyingFiles,

    eFinished
  };

  void HandleError(const QString& sError);
  void HandleState(EState state, const QByteArray& data);

  void FetchJson();
  void EvaluateJson(const QByteArray& arr);
  void DownloadEngineData(const QString& sVersion);
  void DownloadedEngineData(const QByteArray& arr);
  void FinishingUpdate(const QString& sVersion, bool bUpdated = true);
  void UnpackEngine(const QString& sFrom);
  void CopyFiles();

  QPointer<QNetworkAccessManager> m_pManager;
  SSettingsData*                  m_pSettings;
  std::atomic<EState>             m_pCurrentState = EState::eNone;
  std::string                     m_latestVersion;
  std::string                     m_latestUpdaterVersion;
  std::atomic<bool>               m_bShuttingDown = false;
  bool                            m_bUpdateLauncher = false;
};

#endif // CUPDATER_H
