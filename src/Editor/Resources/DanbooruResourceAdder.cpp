#include "DanbooruResourceAdder.h"
#include "WebResourceDownloadManager.h"

#include "Systems/Database/Resource.h"

#include <QJsonDocument>
#include <QJsonArray>

using namespace std::chrono_literals;

namespace
{
  // Documented rate limit is 30 per 5s per normal request and 20 per API search Path
  // (/api/v1/json/search*) so we make sure we do not surpass either
  constexpr std::chrono::seconds c_requestRefreshInterval = 2s;
  constexpr qint32 c_iRequestLimit = 10;
  // Get sequest API path
  constexpr char c_sImageGetPath[] = "/posts/";
  // Known pages that support Philomena-Syntax
  // order of the items is important
  constexpr char* c_sAllowedSites[] = { "danbooru",
                                       "sonohara",
                                       "hijiribe",
                                       "safebooru",
                                       "yukkuri", // service not available anymore
                                       "dorkbooru", // closed
                                       "bronibooru", // service not available anymore
                                       "miya.nipah" // service not available anymore
                                       };
  // The https: protocol must be specified on all URLs.
  constexpr char c_sRequiredProtocol[] = "https";

  constexpr QNetworkReply::NetworkError c_errThrottled = QNetworkReply::NetworkError(429);

  // JSON elements to parse
  constexpr char c_sJSONElementId[] = "id";
  constexpr char c_sJSONElementSourceUrl[] = "source";
  constexpr char c_sJSONElementFileUrl[] = "file_url";
  constexpr char c_sJSONElementFileExt[] = "file_ext";
  constexpr char c_sJSONElementTags[] = "tag_string";
  constexpr char c_sJSONElementTagArtist[] = "tag_string_artist";
  constexpr char c_sTagArtist[] = "artist";

  //--------------------------------------------------------------------------------------
  //
  qint32 GetImageId(const QUrl& url)
  {
    // link in the form
    // https://danbooru.donmai.us/posts/1000.json
    QStringList vsPathElems = url.path().split("/");
    if (vsPathElems.size() > 0)
    {
      bool bOk = false;
      QString sLastPathElem = vsPathElems.last();
      qint32 iLastDot = sLastPathElem.lastIndexOf(".");
      if (-1 != iLastDot)
      {
        sLastPathElem = sLastPathElem.mid(0, iLastDot);
      }
      qint32 iId = sLastPathElem.toInt(&bOk);
      if (!bOk)
      {
        return -1;
      }
      return iId;
    }
    return -1;
  }
}

CDanbooruResourceAdder::CDanbooruResourceAdder(QObject* pParent) :
    CBooruResourceAdder{pParent, c_requestRefreshInterval, c_iRequestLimit}
{}
CDanbooruResourceAdder::~CDanbooruResourceAdder()
{
}

//----------------------------------------------------------------------------------------
//
bool CDanbooruResourceAdder::CanHandleUrl(const QUrl& url) const
{
  bool bHasAnOfSupportedHosts = false;
  for (size_t i = 0; std::size(c_sAllowedSites) > i; ++i)
  {
    bHasAnOfSupportedHosts |= url.host().contains(c_sAllowedSites[i]);
  }
  return url.scheme().contains(c_sRequiredProtocol) && bHasAnOfSupportedHosts &&
         GetImageId(url) > -1;
}

//----------------------------------------------------------------------------------------
//
QUrl CDanbooruResourceAdder::GetRequestUrl(const QUrl& url)
{
  QUrl copy = url;
  copy.setPath(c_sImageGetPath + QString::number(GetImageId(url)) + ".json");
  copy.setQuery(QString());
  return copy;
}

//----------------------------------------------------------------------------------------
//
void CDanbooruResourceAdder::HandleResponseJson(const QUrl& url, bool bDownloadAndAddAsFile,
                                                const QByteArray& arr)
{
  QJsonParseError err;
  QJsonDocument doc = QJsonDocument::fromJson(arr, &err);
  if (QJsonParseError::NoError != err.error)
  {
    qWarning() << tr("Could not parse Philomena response.");
    return;
  }

  QJsonObject root = doc.object();

  QString sProvider;
  for (size_t i = 0; std::size(c_sAllowedSites) > i; ++i)
  {
    if (url.host().contains(c_sAllowedSites[i]))
    {
      sProvider = QString(c_sAllowedSites[i]);
      break;
    }
  }

  std::vector<STagData> vTags;
  SResourceData res;
  res.m_sName = sProvider + "_" + QString::number(GetImageId(url));
  res.m_sSource = url;

  auto it = root.find(c_sJSONElementId);
  if (root.end() != it)
  {
    res.m_sName = sProvider + "_" + it.value().toVariant().toString();
  }
  it = root.find(c_sJSONElementSourceUrl);
  if (root.end() != it)
  {
    res.m_sSource = QUrl(it.value().toString());
  }
  it = root.find(c_sJSONElementFileExt);
  if (root.end() != it)
  {
    QString sFormat = it.value().toString();

    QStringList imageFormatsList = SResourceFormats::ImageFormats();
    QStringList videoFormatsList = SResourceFormats::VideoFormats();

    if (!sFormat.isEmpty())
    {
      if (imageFormatsList.contains(sFormat))
      {
        res.m_type = EResourceType::eImage;
      }
      else if (videoFormatsList.contains(sFormat))
      {
        res.m_type = EResourceType::eMovie;
      }
      else
      {
        res.m_type = EResourceType::eImage;
      }
    }
  }
  it = root.find(c_sJSONElementFileUrl);
  if (root.end() != it)
  {
    res.m_sPath = SResourcePath(QUrl(it.value().toString()));
  }
  it = root.find(c_sJSONElementTagArtist);
  if (root.end() != it)
  {
    vTags.push_back({c_sTagArtist,
                     QString("%1:%2").arg(c_sTagArtist).arg(it.value().toString()),
                     QString()});
  }
  it = root.find(c_sJSONElementTags);
  if (root.end() != it)
  {
    QString sTags = it.value().toString();
    QStringList vsTags = sTags.split(QRegExp("\\s"));
    if (!vsTags.empty())
    {
      for (qint32 i = 0; vsTags.size() > i; ++i)
      {
        Q_UNUSED(vsTags)
        Q_UNUSED(i)
        // TODO:
      }
    }
  }

  AddResourceImpl(static_cast<QUrl>(res.m_sPath), bDownloadAndAddAsFile, ERequestType::eImage, res, vTags);
}

//----------------------------------------------------------------------------------------
//
void CDanbooruResourceAdder::HandleResponseImage(const SRequestItem& item, const QByteArray& arr)
{
  if (!item.res.has_value())
  {
    qWarning() << tr("Internal response error in Philomena request.");
    return;
  }

  auto res = item.res.value();
  auto vTags = item.optvTags.value_or(std::vector<STagData>{});

  auto optRes =
      CWebResourceDownloadManager::RemoteUrlToResource(static_cast<QUrl>(res.m_sPath), arr);

  if (optRes.has_value())
  {
    emit SignalNewResourceFile(res, vTags, arr, item.bDownloadAndAddAsFile);
  }
  else
  {
    qWarning() << tr("Remote file %1 is not valid for adding.").arg(item.url.toString());
  }
}

//----------------------------------------------------------------------------------------
//
void CDanbooruResourceAdder::NetworkReplyErrorImpl(QNetworkReply::NetworkError code)
{
  if (c_errThrottled == code)
  {
    m_iAllowedRequests = 0;
    m_lastTimePointReset = std::chrono::steady_clock::now();
    m_resetTime = c_requestRefreshInterval;
  }
}
