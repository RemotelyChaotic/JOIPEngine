#include "CommonRemoteResourceAdder.h"
#include "Settings.h"

#include "Systems/Database/Resource.h"

CCommonRemoteResourceAdder::CCommonRemoteResourceAdder(QObject *pParent)
    : QObject{pParent}, IRemoteResourceAdder()
{}

CCommonRemoteResourceAdder::~CCommonRemoteResourceAdder()
{
  for (auto pResponse : m_vpResponses)
  {
    pResponse->abort();
    pResponse->disconnect();
    delete pResponse;
  }
}

//----------------------------------------------------------------------------------------
//
void CCommonRemoteResourceAdder::AddResource(const QUrl& url, bool bDownloadAndAddAsFile)
{
  if (nullptr != m_pNAManager)
  {
    if (url.isValid())
    {
      QNetworkRequest req(url);
      req.setRawHeader(IRemoteResourceAdder::c_sUserAgent,
                       CSettings::c_sApplicationName.toUtf8());
      auto pResponse = m_pNAManager->get(req);
      pResponse->setProperty(c_sDownloadProperty, bDownloadAndAddAsFile);
      connect(pResponse, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
              this, &CCommonRemoteResourceAdder::SlotNetworkReplyError);
      connect(pResponse, &QNetworkReply::finished,
              this, &CCommonRemoteResourceAdder::SlotNetworkReplyFinished);
      m_vpResponses.push_back(pResponse);
    }
    else
    {
      qWarning() << "Non-valid url.";
    }
  }
}

//----------------------------------------------------------------------------------------
//
bool CCommonRemoteResourceAdder::CanDownloadAndSaveAsFile() const
{
  return true;
}

//----------------------------------------------------------------------------------------
//
bool CCommonRemoteResourceAdder::CanHandleUrl(const QUrl& url) const
{
  QStringList imageFormatsList = SResourceFormats::ImageFormats();
  QStringList videoFormatsList = SResourceFormats::VideoFormats();

  qint32 iLastIndex = url.fileName().lastIndexOf('.');
  const QString sFileName = url.fileName();
  QString sFormat = "*" + sFileName.mid(iLastIndex, sFileName.size() - iLastIndex);

  return imageFormatsList.contains(sFormat) || videoFormatsList.contains(sFormat);
}

//----------------------------------------------------------------------------------------
//
void CCommonRemoteResourceAdder::SetNetworkAccessManager(QNetworkAccessManager* pNAManager)
{
  m_pNAManager = pNAManager;
}

//----------------------------------------------------------------------------------------
//
void CCommonRemoteResourceAdder::SlotNetworkReplyError(QNetworkReply::NetworkError code)
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
}

//----------------------------------------------------------------------------------------
//
void CCommonRemoteResourceAdder::SlotNetworkReplyFinished()
{
  QPointer<QNetworkReply> pReply = dynamic_cast<QNetworkReply*>(sender());
  assert(nullptr != pReply);
  if (nullptr != pReply)
  {
    QUrl url = pReply->url();
    QByteArray arr = pReply->readAll();
    bool bAddAsFile = pReply->property(c_sDownloadProperty).toBool();

    emit SignalNewResourceFile(pReply->url(), arr, bAddAsFile);

    auto it = std::find(m_vpResponses.begin(), m_vpResponses.end(), pReply);
    if (m_vpResponses.end() != it)
    {
      m_vpResponses.erase(it);
      pReply->disconnect();
      pReply->deleteLater();
    }
  }
}
