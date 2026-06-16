#include "PhilomenaRemoteResourceAdder.h"
#include "WebResourceDownloadManager.h"

#include "Systems/Database/Resource.h"

#include <QJsonDocument>
#include <QJsonArray>

using namespace std::chrono_literals;

namespace
{
  // Documented rate limit is 30 per 5s per normal request and 20 per API search Path
  // (/api/v1/json/search*) so we make sure we do not surpass either
  constexpr std::chrono::seconds c_requestRefreshInterval = 6s;
  constexpr qint32 c_iRequestLimit = 20;
  // Documented block duration is 15 minutes
  constexpr std::chrono::seconds c_requestBlockTime = 15min + 1s;
  // Get sequest API path
  constexpr char c_sImageGetPathV1[] = "/api/v1/json/images/";
  constexpr char c_sImageGetPathV3[] = "/api/v3/posts/";
  // Known pages that support Philomena-Syntax
  // order of the items is important
  constexpr char* c_sAllowedSites[] = { "derpibooru",
                                       "tantabus",
                                       "manebooru",
                                       "twibooru",
                                       "furbooru" };
  constexpr char* c_sAllowedCdnSites[] = { "derpicdn",
                                           "tantabuscdn",
                                           "manebooru",
                                           "cdn.twibooru",
                                           "furrycdn" };
  static_assert(std::size(c_sAllowedSites) == std::size(c_sAllowedCdnSites));
  // The https: protocol must be specified on all URLs.
  constexpr char c_sRequiredProtocol[] = "https";

  constexpr QNetworkReply::NetworkError c_errBlocked = QNetworkReply::NetworkError(500);
  constexpr QNetworkReply::NetworkError c_errMiddleWareChallange = QNetworkReply::NetworkError(501);

  // JSON elements to parse
  constexpr char c_sJSONElementImage[] = "image";
  constexpr char c_sJSONElementId[] = "id";
  constexpr char c_sJSONElementSourceUrl[] = "source_url";
  //constexpr char c_sJSONElementRepresentations[] = "representations";
  //constexpr char c_sJSONElementFull[] = "full";
  constexpr char c_sJSONElementViewUrl[] = "view_url";
  constexpr char c_sJSONElementMimeType[] = "mime_type";
  constexpr char c_sJSONElementTags[] = "tags";
  constexpr char c_sJSONElementTagArtist[] = "artist";
  constexpr char c_sJSONElementTagEditor[] = "editor";
  constexpr char c_sJSONElementTagPrompter[] = "prompter";
  constexpr char c_sJSONElementTagCreator[] = "creator";
  constexpr char c_sJSONElementFormat[] = "format";
  //constexpr char c_sJSONElementAnimated[] = "animated";

  //--------------------------------------------------------------------------------------
  //
  QString GetAPIHostName(const QUrl& url)
  {
    static_assert(5 == std::size(c_sAllowedSites));
    static const std::map<QString, QString> c_map =
        {
         {QString(c_sAllowedSites[0]), QString("derpibooru.org")},
         {QString(c_sAllowedSites[1]), QString("tantabus.ai")},
         {QString(c_sAllowedSites[2]), QString("manebooru.art")},
         {QString(c_sAllowedSites[3]), QString("twibooru.org")},
         {QString(c_sAllowedSites[4]), QString("furbooru.org")},
         {QString(c_sAllowedCdnSites[0]), QString("derpibooru.org")},
         {QString(c_sAllowedCdnSites[1]), QString("tantabus.ai")},
         //{QString(c_sAllowedCdnSites[2]), QString("manebooru.art")}, manebooru has the same hostname
         {QString(c_sAllowedCdnSites[3]), QString("twibooru.org")},
         {QString(c_sAllowedCdnSites[4]), QString("furbooru.org")},
         };

    for (size_t i = 0; std::size(c_sAllowedSites) > i; ++i)
    {
      if(url.host().contains(c_sAllowedSites[i]))
      {
        return c_map.find(c_sAllowedSites[i])->second;
      }
    }

    for (size_t i = 0; std::size(c_sAllowedCdnSites) > i; ++i)
    {
      if(url.host().contains(c_sAllowedCdnSites[i]))
      {
        return c_map.find(c_sAllowedCdnSites[i])->second;
      }
    }

    return QString();
  }

  //--------------------------------------------------------------------------------------
  //
  QString GetAPIImagePath(const QUrl& url)
  {
    static_assert(5 == std::size(c_sAllowedSites));
    static const std::map<QString, QString> c_map =
    {
      {QString(c_sAllowedSites[0]), QString(c_sImageGetPathV1)},
      {QString(c_sAllowedSites[1]), QString(c_sImageGetPathV1)},
      {QString(c_sAllowedSites[2]), QString(c_sImageGetPathV1)},
      {QString(c_sAllowedSites[3]), QString(c_sImageGetPathV3)}, // Twibooru uses a different syntax
      {QString(c_sAllowedSites[4]), QString(c_sImageGetPathV1)},
    };

    assert(c_map.size() == std::size(c_sAllowedSites));

    for (size_t i = 0; std::size(c_sAllowedSites) > i; ++i)
    {
      if(url.host().contains(c_sAllowedSites[i]))
      {
        return c_map.find(c_sAllowedSites[i])->second;
      }
    }

    for (size_t i = 0; std::size(c_sAllowedCdnSites) > i; ++i)
    {
      if(url.host().contains(c_sAllowedCdnSites[i]))
      {
        return c_map.find(c_sAllowedSites[i])->second;
      }
    }

    return QString(c_sImageGetPathV1);
  }

  //--------------------------------------------------------------------------------------
  //
  qint32 GetImageId(const QUrl& url)
  {
    QString sActualFileName = url.fileName();
    if (!sActualFileName.contains("."))
    {
      sActualFileName = QString();
    }

    if (sActualFileName.isEmpty())
    {
      // link in the form
      // https://derpibooru.org/images/3826319
      QStringList vsPathElems = url.path().split("/");
      if (vsPathElems.size() > 0)
      {
        bool bOk = false;
        qint32 iId = vsPathElems.last().toInt(&bOk);
        if (!bOk)
        {
          return -1;
        }
        return iId;
      }
    }
    else
    {
      // link in any of the forms below. The id is the last number in the path
      // or the fist number in the filename:
      // https://furrycdn.org/img/view/2020/4/24/2.png
      // https://static.manebooru.art/img/2020/10/19/2/large.gif
      // https://tantabuscdn.net/img/view/2020/7/15/100__safe_ai+generated_automatically+imported_derpibooru+import_generator-colon-thisponydoesnotexist_oc_oc+only_pony_generation+errors_solo.jpg
      QStringList vsPathElems = url.path().split("/");
      qint32 iBackMostInt = -1;
      qint32 iNumberOfIntsInPath = 0;
      for (qint32 i = vsPathElems.size()-1; 0 <= i; --i)
      {
        bool bOk = false;
        qint32 iPathValInt = vsPathElems[i].toInt(&bOk);
        if (bOk)
        {
          if (0 == iNumberOfIntsInPath)
          {
            iBackMostInt = iPathValInt;
          }
          ++iNumberOfIntsInPath;
        }
      }

      // we have a date and an id in the path
      if (4 == iNumberOfIntsInPath)
      {
        return iBackMostInt;
      }
      // the id is in the file
      else
      {
        QStringList vsParts = sActualFileName.split(QRegExp("\\.|_"));
        // id must be the first element
        if (vsParts.size() > 0)
        {
          bool bOk = false;
          qint32 iId = vsParts[0].toInt(&bOk);
          return bOk ? iId : -1;
        }
      }
    }
    return -1;
  }
}

//----------------------------------------------------------------------------------------
//
CPhilomenaRemoteResourceAdder::CPhilomenaRemoteResourceAdder(QObject* pParent) :
  CBooruResourceAdder{pParent, c_requestRefreshInterval, c_iRequestLimit}
{
}
CPhilomenaRemoteResourceAdder::~CPhilomenaRemoteResourceAdder()
{
}

//----------------------------------------------------------------------------------------
//
bool CPhilomenaRemoteResourceAdder::CanHandleUrl(const QUrl& url) const
{
  bool bHasAnOfSupportedHosts = false;
  for (size_t i = 0; std::size(c_sAllowedSites) > i; ++i)
  {
    bHasAnOfSupportedHosts |= url.host().contains(c_sAllowedSites[i]);
  }
  for (size_t i = 0; std::size(c_sAllowedCdnSites) > i; ++i)
  {
    bHasAnOfSupportedHosts |= url.host().contains(c_sAllowedCdnSites[i]);
  }
  return url.scheme().contains(c_sRequiredProtocol) && bHasAnOfSupportedHosts &&
         GetImageId(url) > -1;
}

//----------------------------------------------------------------------------------------
//
QUrl CPhilomenaRemoteResourceAdder::GetRequestUrl(const QUrl& url)
{
  QUrl copy = url;
  copy.setHost(GetAPIHostName(url));
  copy.setPath(GetAPIImagePath(url) + QString::number(GetImageId(url)));
  copy.setQuery(QString());
  return copy;
}

//----------------------------------------------------------------------------------------
//
void CPhilomenaRemoteResourceAdder::HandleResponseJson(const QUrl& url, bool bDownloadAndAddAsFile,
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
  auto itImage = root.find(c_sJSONElementImage);
  if (root.end() == itImage)
  {
    qWarning() << tr("Could not parse Philomena response. Image object not found.");
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

  std::vector<STagData> vTags;
  SResourceData res;
  res.m_sName = sProvider + "_" + QString::number(GetImageId(url));
  res.m_sSource = url;

  auto imgObj = itImage.value().toObject();
  auto it = imgObj.find(c_sJSONElementId);
  if (imgObj.end() != it)
  {
    res.m_sName = sProvider + "_" + it.value().toVariant().toString();
  }
  it = imgObj.find(c_sJSONElementSourceUrl);
  if (imgObj.end() != it)
  {
    res.m_sSource = QUrl(it.value().toString());
  }
  QString sFormat;
  it = imgObj.find(c_sJSONElementFormat);
  if (imgObj.end() != it)
  {
    sFormat = it.value().toString();
  }
  it = imgObj.find(c_sJSONElementMimeType);
  if (imgObj.end() != it)
  {
    QString sMimeType = it.value().toString();
    QStringList vsTypeElems = sMimeType.split("/");

    QStringList imageFormatsList = SResourceFormats::ImageFormats();
    QStringList videoFormatsList = SResourceFormats::VideoFormats();

    if (vsTypeElems.size() > 0)
    {
      if ("image" == vsTypeElems[0])
      {
        res.m_type = EResourceType::eImage;
      }
      else if ("video" == vsTypeElems[0])
      {
        res.m_type = EResourceType::eMovie;
      }
    }
  }
  it = imgObj.find(c_sJSONElementViewUrl);
  if (imgObj.end() != it)
  {
    res.m_sPath = SResourcePath(QUrl(it.value().toString()));
  }
  it = imgObj.find(c_sJSONElementTags);
  if (imgObj.end() != it)
  {
    if (it.value().isArray())
    {
      static QStringList vsInterestedTags = {
        c_sJSONElementTagArtist, c_sJSONElementTagPrompter, c_sJSONElementTagEditor, c_sJSONElementTagCreator
      };
      QJsonArray arr = it.value().toArray();
      for (qint32 i = 0; arr.size() > i; ++i)
      {
        QString sTagFull = arr.at(i).toString();
        for (const QString& sTag : qAsConst(vsInterestedTags))
        {
          if (sTagFull.contains(sTag))
          {
            auto vsParts = sTagFull.split(":");
            if (vsParts.size() > 1)
            {
              vTags.push_back({vsParts[0], sTagFull, QString()});
              res.m_vsResourceTags.insert(sTagFull);
            }
            break;
          }
        }
      }
    }
  }

  AddResourceImpl(static_cast<QUrl>(res.m_sPath), bDownloadAndAddAsFile, ERequestType::eImage, res, vTags);
}

//----------------------------------------------------------------------------------------
//
void CPhilomenaRemoteResourceAdder::HandleResponseImage(const SRequestItem& item, const QByteArray& arr)
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
void CPhilomenaRemoteResourceAdder::NetworkReplyErrorImpl(QNetworkReply::NetworkError code)
{
  if (c_errBlocked == code)
  {
    m_iAllowedRequests = 0;
    m_lastTimePointReset = std::chrono::steady_clock::now();
    m_resetTime = c_requestBlockTime;
  }
  else if (c_errMiddleWareChallange == code)
  {
    m_iAllowedRequests = 0;
    m_lastTimePointReset = std::chrono::steady_clock::now();
    m_resetTime = c_requestRefreshInterval;
  }
}
