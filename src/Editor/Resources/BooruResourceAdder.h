#ifndef BOORURESOURCEADDER_H
#define BOORURESOURCEADDER_H

#include "IRemoteResourceAdder.h"

#include <QObject>
#include <QPointer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>

#include <chrono>
#include <optional>
#include <queue>

class CBooruResourceAdder : public QObject, public IRemoteResourceAdder
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

  explicit CBooruResourceAdder(QObject *parent = nullptr,
                               std::chrono::seconds resetTime = std::chrono::seconds(10),
                               qint32 iAllowedRequests = 10);
  ~CBooruResourceAdder() override;

  void AddResource(const QUrl& url, bool bDownloadAndAddAsFile = false) override;

  bool CanDownloadAndSaveAsFile() const override { return true; }

  void SetNetworkAccessManager(QNetworkAccessManager* pNAManager) override;

signals:
  void SignalNewResourceFile(const SResourceData& res, const std::vector<STagData>& vsTags,
                             const QByteArray& ba, bool bAddAsFile) override;

private slots:
  void SlotNetworkReplyError(QNetworkReply::NetworkError code);
  void SlotNetworkReplyFinished();
  void SlotTimeoutCheck();

protected:
  void AddResourceImpl(const QUrl& url, bool bDownloadAndAddAsFile,
                       ERequestType type,
                       std::optional<SResourceData> res,
                       std::optional<std::vector<STagData>> optvTags);
  virtual QUrl GetRequestUrl(const QUrl& url) = 0;
  virtual void HandleResponseJson(const QUrl& url, bool bDownloadAndAddAsFile, const QByteArray& arr) = 0;
  virtual void HandleResponseImage(const SRequestItem& item, const QByteArray& arr) = 0;
  virtual void NetworkReplyErrorImpl(QNetworkReply::NetworkError code) = 0;
  void PushRequest(const SRequestItem& item);
  void ProcessQueue();

  QPointer<QNetworkAccessManager>      m_pNAManager;
  QTimer                               m_timer;
  std::vector<QPointer<QNetworkReply>> m_vpResponses;
  std::queue<SRequestItem>             m_waitingQueue;
  std::chrono::time_point<std::chrono::steady_clock> m_lastTimePointReset;
  std::chrono::seconds                 m_resetTime;
  qint32                               m_iAllowedRequests = 0;
  const std::chrono::seconds           m_resetTimeInterval;
  const qint32                         m_iAllowedRequestsMax = 0;
};

#endif // BOORURESOURCEADDER_H
