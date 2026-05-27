#ifndef PHILOMENAREMOTERESOURCEADDER_H
#define PHILOMENAREMOTERESOURCEADDER_H

#include "IRemoteResourceAdder.h"

#include <QObject>
#include <QPointer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>

#include <chrono>
#include <optional>
#include <queue>

class CPhilomenaRemoteResourceAdder : public QObject, public IRemoteResourceAdder
{
  Q_OBJECT
public:
  enum ERequestType
  {
    eJson,
    eImage,
  };
  struct SRequestItem
  {
    QUrl url;
    bool bDownloadAndAddAsFile;
    ERequestType reqType;
    std::optional<SResourceData> res;
    std::optional<std::vector<STagData>> optvTags;
  };

  explicit CPhilomenaRemoteResourceAdder(QObject* pParent = nullptr);
  ~CPhilomenaRemoteResourceAdder() override;

  void AddResource(const QUrl& url, bool bDownloadAndAddAsFile = false) override;

  bool CanDownloadAndSaveAsFile() const override;

  bool CanHandleUrl(const QUrl& url) const override;

  QString Name() const override { return "Philomena Downloader"; };

  void SetNetworkAccessManager(QNetworkAccessManager* pNAManager);

signals:
  void SignalNewResourceFile(const SResourceData& res, const std::vector<STagData>& vsTags,
                             const QByteArray& ba, bool bAddAsFile) override;

private slots:
  void SlotNetworkReplyError(QNetworkReply::NetworkError code);
  void SlotNetworkReplyFinished();
  void SlotTimeoutCheck();

private:
  void AddResourceImpl(const QUrl& url, bool bDownloadAndAddAsFile,
                       ERequestType type,
                       std::optional<SResourceData> res,
                       std::optional<std::vector<STagData>> optvTags);
  qint32 GetImageId(const QUrl& url) const;
  void HandleResponseJson(const QUrl& url, bool bDownloadAndAddAsFile, const QByteArray& arr);
  void HandleResponseImage(const SRequestItem& item, const QByteArray& arr);
  void PushRequest(const SRequestItem& item);
  void ProcessQueue();

  QPointer<QNetworkAccessManager>      m_pNAManager;
  QTimer                               m_timer;
  std::vector<QPointer<QNetworkReply>> m_vpResponses;
  std::queue<SRequestItem>             m_waitingQueue;
  std::chrono::time_point<std::chrono::steady_clock> m_lastTimePointReset;
  std::chrono::seconds                 m_resetTime;
  qint32                               m_iAllowedRequests = 0;
};

#endif // PHILOMENAREMOTERESOURCEADDER_H
