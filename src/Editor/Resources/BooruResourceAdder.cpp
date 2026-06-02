#include "BooruResourceAdder.h"
#include "Settings.h"

Q_DECLARE_METATYPE(CBooruResourceAdder::SRequestItem)

using namespace std::chrono_literals;

namespace
{
  constexpr char c_sPropertyReqData[] = "RequestData";
}

CBooruResourceAdder::CBooruResourceAdder(QObject* pParent,
                                         std::chrono::seconds resetTime,
                                         qint32  iAllowedRequests) :
  QObject{pParent}, IRemoteResourceAdder(),
  m_timer(),
  m_lastTimePointReset(std::chrono::steady_clock::now()),
  m_resetTime(resetTime),
  m_iAllowedRequests(iAllowedRequests),
  m_resetTimeInterval(resetTime),
  m_iAllowedRequestsMax(iAllowedRequests)
{
  qRegisterMetaType<CBooruResourceAdder::SRequestItem>();

  m_timer.setInterval(1s);
  m_timer.setSingleShot(false);
  connect(&m_timer, &QTimer::timeout, this, &CBooruResourceAdder::SlotTimeoutCheck);
  m_timer.start();
}
CBooruResourceAdder::~CBooruResourceAdder()
{
  while(!m_waitingQueue.empty()) { m_waitingQueue.pop(); }
  m_timer.stop();
  for (auto pResponse : m_vpResponses)
  {
    pResponse->abort();
    pResponse->disconnect();
    delete pResponse;
  }
}

//----------------------------------------------------------------------------------------
//
void CBooruResourceAdder::AddResource(const QUrl& url, bool bDownloadAndAddAsFile)
{
  if (nullptr != m_pNAManager)
  {
    if (url.isValid())
    {
      AddResourceImpl(url, bDownloadAndAddAsFile, ERequestType::eJson, std::nullopt, std::nullopt);
    }
    else
    {
      qWarning() << "Non-valid url.";
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CBooruResourceAdder::SetNetworkAccessManager(QNetworkAccessManager* pNAManager)
{
  m_pNAManager = pNAManager;
}

//----------------------------------------------------------------------------------------
//
void CBooruResourceAdder::SlotNetworkReplyError(QNetworkReply::NetworkError code)
{
  QPointer<QNetworkReply> pReply = dynamic_cast<QNetworkReply*>(sender());
  assert(nullptr != pReply);
  if (nullptr != pReply)
  {
    qWarning() << code << pReply->errorString();
  }

  auto it = std::find(m_vpResponses.begin(), m_vpResponses.end(), pReply);
  if (m_vpResponses.end() != it)
  {
    m_vpResponses.erase(it);
    pReply->disconnect();
    pReply->deleteLater();
  }

  NetworkReplyErrorImpl(code);
}

//----------------------------------------------------------------------------------------
//
void CBooruResourceAdder::SlotNetworkReplyFinished()
{
  QPointer<QNetworkReply> pReply = dynamic_cast<QNetworkReply*>(sender());
  assert(nullptr != pReply);
  if (nullptr != pReply)
  {
    QUrl url = pReply->url();
    QByteArray arr = pReply->readAll();
    bool bAddAsFile = pReply->property(c_sDownloadProperty).toBool();
    SRequestItem reqItem = pReply->property(c_sPropertyReqData).value<CBooruResourceAdder::SRequestItem>();

    {
      auto it = std::find(m_vpResponses.begin(), m_vpResponses.end(), pReply);
      if (m_vpResponses.end() != it)
      {
        m_vpResponses.erase(it);
        pReply->disconnect();
        pReply->deleteLater();
      }
    }

    switch (reqItem.reqType)
    {
      case ERequestType::eJson:
        HandleResponseJson(url, bAddAsFile, arr);
        break;
      case ERequestType::eImage:
        HandleResponseImage(reqItem, arr);
        break;
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CBooruResourceAdder::SlotTimeoutCheck()
{
  auto now = std::chrono::steady_clock::now();
  if (std::chrono::duration_cast<std::chrono::seconds>(now - m_lastTimePointReset) >= m_resetTime)
  {
    m_iAllowedRequests = m_iAllowedRequestsMax;
    m_lastTimePointReset = std::chrono::steady_clock::now();
    m_resetTime = m_resetTimeInterval;

    ProcessQueue();
  }
}

//----------------------------------------------------------------------------------------
//
void CBooruResourceAdder::AddResourceImpl(
    const QUrl& url, bool bDownloadAndAddAsFile, ERequestType type,
    std::optional<SResourceData> res,
    std::optional<std::vector<STagData>> optvTags)
{
  if (m_iAllowedRequests > 0)
  {
    PushRequest({url, bDownloadAndAddAsFile, type, res, optvTags});
  }
  else
  {
    m_waitingQueue.push({url, bDownloadAndAddAsFile, type, res, optvTags});
  }
}

//----------------------------------------------------------------------------------------
//
void CBooruResourceAdder::PushRequest(const SRequestItem& item)
{
  m_iAllowedRequests--;

  QUrl copy = item.url;
  if (ERequestType::eJson == item.reqType)
  {
    copy = GetRequestUrl(item.url);
  }

  QNetworkRequest req(copy);
  req.setRawHeader(IRemoteResourceAdder::c_sUserAgent,
                   CSettings::c_sApplicationName.toUtf8());
  auto pResponse = m_pNAManager->get(req);
  pResponse->setProperty(c_sDownloadProperty, item.bDownloadAndAddAsFile);
  pResponse->setProperty(c_sPropertyReqData, QVariant::fromValue(item));
  connect(pResponse, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
          this, &CBooruResourceAdder::SlotNetworkReplyError);
  connect(pResponse, &QNetworkReply::finished,
          this, &CBooruResourceAdder::SlotNetworkReplyFinished);
  m_vpResponses.push_back(pResponse);
}

//----------------------------------------------------------------------------------------
//
void CBooruResourceAdder::ProcessQueue()
{
  if (m_waitingQueue.empty())
  {
    return;
  }

  if (m_iAllowedRequests <= 0)
  {
    return;
  }

  auto elem = m_waitingQueue.front();
  m_waitingQueue.pop();

  PushRequest(elem);
}
