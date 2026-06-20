#include "E6xBooruResourceAdder.h"
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
  constexpr qint32 c_iRequestLimit = 2;
  // Get sequest API path
  constexpr char c_sImageGetPath[] = "/posts/";
  // Known pages that support Philomena-Syntax
  // order of the items is important
  constexpr const char* c_sAllowedSites[] =
                                      { "e621",
                                        "e926",
                                        "e6ai"
                                      };
  // The https: protocol must be specified on all URLs.
  constexpr char c_sRequiredProtocol[] = "https";

  constexpr QNetworkReply::NetworkError c_errThrottled = QNetworkReply::NetworkError(503);

  // JSON elements to parse
  constexpr char c_sJSONElementPost[] = "post";
  constexpr char c_sJSONElementId[] = "id";
  constexpr char c_sJSONElementSourcesUrl[] = "sources";
  constexpr char c_sJSONElementFile[] = "file";
  constexpr char c_sJSONElementFileUrl[] = "url";
  constexpr char c_sJSONElementFileExt[] = "ext";
  constexpr char c_sJSONElementTags[] = "tags";
  constexpr char c_sJSONElementTagArtist[] = "artist";
  constexpr char c_sJSONElementTagDirector[] = "director";
  constexpr char c_sJSONElementTagPrompter[] = "prompter";

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

//----------------------------------------------------------------------------------------
//
CE6xBooruResourceAdder::CE6xBooruResourceAdder(QObject* pParent) :
    CBooruResourceAdder{pParent, c_requestRefreshInterval, c_iRequestLimit}
{}

CE6xBooruResourceAdder::~CE6xBooruResourceAdder()
{
}

//----------------------------------------------------------------------------------------
//
bool CE6xBooruResourceAdder::CanHandleUrl(const QUrl& url) const
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
QUrl CE6xBooruResourceAdder::GetRequestUrl(const QUrl& url)
{
  QUrl copy = url;
  copy.setPath(c_sImageGetPath + QString::number(GetImageId(url)) + ".json");
  copy.setQuery(QString());
  return copy;
}

//----------------------------------------------------------------------------------------
//
void CE6xBooruResourceAdder::HandleResponseJson(const QUrl& url, bool bDownloadAndAddAsFile,
                                                const QByteArray& arr)
{
  QJsonParseError err;
  QJsonDocument doc = QJsonDocument::fromJson(arr, &err);
  if (QJsonParseError::NoError != err.error)
  {
    qWarning() << tr("Could not parse eXXX Danbooru API response.");
    return;
  }

  QString sProvider;
  for (size_t i = 0; std::size(c_sAllowedSites) > i; ++i)
  {
    if (url.host().contains(c_sAllowedSites[i]))
    {
      sProvider = QString(c_sAllowedSites[i]);
      break;
    }
  }

  QJsonObject root = doc.object();
  auto itPost = root.find(c_sJSONElementPost);
  if (root.end() == itPost || !itPost->isObject())
  {
    qWarning() << tr("%1 response has unexpected format.").arg(sProvider);
    return;
  }

  std::vector<STagData> vTags;
  SResourceData res;
  res.m_sName = sProvider + "_" + QString::number(GetImageId(url));
  res.m_sSource = url;

  auto post = itPost->toObject();
  auto it = post.find(c_sJSONElementId);
  if (post.end() != it)
  {
    res.m_sName = sProvider + "_" + it.value().toVariant().toString();
  }
  it = post.find(c_sJSONElementSourcesUrl);
  if (post.end() != it && it->isArray())
  {
    QJsonArray arr = it.value().toArray();
    if (arr.size() > 0)
    {
      res.m_sSource = QUrl(arr.at(0).toString());
    }
  }

  it = post.find(c_sJSONElementFile);
  if (post.end() != it && it->isObject())
  {
    QJsonObject objFile = it->toObject();
    it = objFile.find(c_sJSONElementFileExt);
    if (objFile.end() != it)
    {
      QString sFormat = it.value().toString();

      QStringList imageFormatsList = SResourceFormats::ImageFormats();
      QStringList videoFormatsList = SResourceFormats::VideoFormats();

      if (!sFormat.isEmpty())
      {
        if (imageFormatsList.contains("*." + sFormat))
        {
          res.m_type = EResourceType::eImage;
        }
        else if (videoFormatsList.contains("*." + sFormat))
        {
          res.m_type = EResourceType::eMovie;
        }
        else
        {
          res.m_type = EResourceType::eImage;
        }
      }
    }
    it = objFile.find(c_sJSONElementFileUrl);
    if (objFile.end() != it)
    {
      res.m_sPath = SResourcePath(QUrl(it.value().toString()));
    }
  }
  it = post.find(c_sJSONElementTags);
  if (post.end() != it)
  {
    QJsonObject objTags = it->toObject();
    it = objTags.find(c_sJSONElementTagArtist);
    if (objTags.end() != it && it->isArray())
    {
      QJsonArray arr = it.value().toArray();
      if (arr.size() > 0)
      {
        vTags.push_back({c_sJSONElementTagArtist,
                         QString("%1:%2").arg(c_sJSONElementTagArtist).arg(it.value().toString()),
                         QString()});
      }
    }
    it = objTags.find(c_sJSONElementTagDirector);
    if (objTags.end() != it && it->isArray())
    {
      QJsonArray arr = it.value().toArray();
      if (arr.size() > 0)
      {
        vTags.push_back({c_sJSONElementTagPrompter,
                         QString("%1:%2").arg(c_sJSONElementTagPrompter).arg(it.value().toString()),
                         QString()});
      }
    }
  }

  AddResourceImpl(static_cast<QUrl>(res.m_sPath), bDownloadAndAddAsFile, ERequestType::eImage, res, vTags);
}

//----------------------------------------------------------------------------------------
//
void CE6xBooruResourceAdder::HandleResponseImage(const SRequestItem& item, const QByteArray& arr)
{
  if (!item.res.has_value())
  {
    qWarning() << tr("Internal response error in eXXX Danbooru API request.");
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
void CE6xBooruResourceAdder::NetworkReplyErrorImpl(QNetworkReply::NetworkError code)
{
  if (c_errThrottled == code)
  {
    m_iAllowedRequests = 0;
    m_lastTimePointReset = std::chrono::steady_clock::now();
    m_resetTime = c_requestRefreshInterval;
  }
}
