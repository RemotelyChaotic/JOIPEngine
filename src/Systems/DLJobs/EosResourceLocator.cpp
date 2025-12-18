#include "EosResourceLocator.h"

#include "Systems/Database/Project.h"

#include <QJsonArray>
#include <QJsonObject>

namespace
{
  const QString c_sDownloadPath = "https://media.milovana.com/timg/";

  const QString c_sPagesKeyWord = "pages";
  const QString c_sLocatorKeyWord = "locator";

  const QString c_sStartPageKeyWord = "start";

  const QString c_sGalleryKeyWord = "galleries";
  const QString c_sGalleryNameKeyWord = "name";
  const QString c_sGalleryImagesKeyWord = "images";
  const QString c_sImageIdKeyWord = "id";
  const QString c_sImagewidthKeyWord = "width";
  const QString c_sImageHeightKeyWord = "height";
  const QString c_sImageHashKeyWord = "hash";
  const QString c_sImageTypeKeyWord = "type";

  const QString c_sModulesKeyWord = "modules";
  const QString c_sModulesAudioKeyWord = "audio";
  const QString c_sModulesStorageKeyWord = "storage";
  const QString c_sModulesNotificationKeyWord = "notification";
  const QString c_sModulesNixKeyWord = "nyx";

  const QString c_sInitKeyWord = "init";
  const QString c_sFilesKeyWord = "files";

  const QString c_sMatcherRemotePrefix = "^https*:\\/\\/";
  const QString c_sMatcherGallery = "^gallery:([^/]+)\\/(.*)$";
  const QString c_sMatcherFile = "^file:(.*)$";
    const QString c_sMatcherFileExtension = "\\.([^.]+)$";
    const QString c_sMatcherFileIsRandom = "\\*";
  const QString c_sMatcherImageData = "^data:image\\/.+";
  const QString c_sMatcherOEOSResource = "^(file:|gallery:).*\\+\\(\\|(oeos|oeos-video):(.+)\\)$";

  const std::map<QString, std::pair<QString, EResourceType>> c_sExtensionMap = {
    {"mp3", {"audio/mpeg", EResourceType::eSound}},
    {"jpg", {"image/jpeg", EResourceType::eImage}},
  };
  const std::map<QString, EResourceType> c_sTypeMap = {
    {"audio/mpeg", EResourceType::eSound},
    {"image/jpeg", EResourceType::eImage},
  };

  const QString c_sSchemeGallery = "gallery";
  const QString c_sSchemeFile = "file";
  const QString c_sSchemeRemote = "https";

  std::vector<QJsonValue> FindKeyRecursive(const QString& key, const QJsonValue& value)
  {
    std::vector<QJsonValue> retVal;
    if (value.isObject())
    {
      const QJsonObject obj = value.toObject();
      if (obj.contains(key))
      { return {obj.value(key)}; } // return 'early' if object contains key

      for (const auto& value : obj)
      {
        std::vector<QJsonValue> recurse =
            FindKeyRecursive(key, value);
        if (recurse.size() > 0)
        {
          retVal.insert(retVal.end(), recurse.begin(), recurse.end());
        }
      }
    }
    else if (value.isArray())
    {
      for (const auto& value : value.toArray())
      {
        std::vector<QJsonValue> recurse = FindKeyRecursive(key, value);
        if (recurse.size() > 0)
        {
          retVal.insert(retVal.end(), recurse.begin(), recurse.end());
        }
      }
    }

    return retVal;          // base case: a null value
  }
}

//----------------------------------------------------------------------------------------
//
CEosResourceLocator::CEosResourceLocator(const QJsonDocument& script,
                                         const std::vector<QString>& vsSupportedHosts,
                                         const QString& sFIX_POLLUTION) :
  m_script(script),
  m_vsSupportedHosts(vsSupportedHosts),
  m_sFIX_POLLUTION(sFIX_POLLUTION.isEmpty() ? "" : ("?"+sFIX_POLLUTION)),
  m_iIdCounter(0)
{
}

CEosResourceLocator::~CEosResourceLocator()
{
}

//----------------------------------------------------------------------------------------
//
bool CEosResourceLocator::LocateAllResources(QString* psError)
{
  QJsonObject root = m_script.object();

  //auto initIt = root.find(c_sInitKeyWord); // optional
  auto pagesIt = root.find(c_sPagesKeyWord);
  auto galeryIt = root.find(c_sGalleryKeyWord);
  auto modulesIt = root.find(c_sModulesKeyWord);
  auto filesIt = root.find(c_sFilesKeyWord); // optional
  if (root.end() == pagesIt || root.end() == galeryIt || root.end() == modulesIt)
  {
    if (nullptr != psError) { *psError = "JSON does not caontain some required modes."; }
    return false;
  }

  if (!pagesIt.value().isObject())
  {
    if (nullptr != psError) { *psError = QString("\"%1\" node is not an object.").arg(c_sPagesKeyWord); }
    return false;
  }
  if (!galeryIt.value().isObject())
  {
    if (nullptr != psError) { *psError = QString("\"%1\" node is not an object.").arg(c_sGalleryKeyWord); }
    return false;
  }
  if (!modulesIt.value().isObject())
  {
    if (nullptr != psError) { *psError = QString("\"%1\" node is not an object.").arg(c_sModulesKeyWord); }
    return false;
  }

  QJsonObject pagesObj = pagesIt.value().toObject();
  QJsonObject galeryObj = galeryIt.value().toObject();
  QJsonObject modulesObj = modulesIt.value().toObject();

  QJsonArray startPageArr;
  auto startIt = pagesObj.find(c_sStartPageKeyWord);
  if (pagesObj.end() != startIt && startIt.value().isArray())
  {
    startPageArr = startIt.value().toArray();
  }

  // gather resources from galeries
  for (auto it = galeryObj.constBegin(); galeryObj.constEnd() != it; ++it)
  {
    QJsonValue galeryObjVal = it.value();
    const QString sGaleryKey = it.key();
    auto& currentGallery = m_resourceMap[sGaleryKey];
    currentGallery.insert({});
    if (galeryObjVal.isObject())
    {
      QJsonObject galeryObj = galeryObjVal.toObject();
      // get galery name
      auto nameIt = galeryObj.find(c_sGalleryNameKeyWord);
      QString sGaleryName;
      if (galeryObj.end() != nameIt)
      {
        sGaleryName = nameIt.value().toString();
      }
      // get galery resources
      auto imagesIt = galeryObj.find(c_sGalleryImagesKeyWord);
      if (galeryObj.end() != imagesIt)
      {
        if (imagesIt->isArray())
        {
          QJsonArray arrImages = imagesIt->toArray();
          for (const QJsonValue& valImage : arrImages)
          {
            if (valImage.isObject())
            {
              QJsonObject objImage = valImage.toObject();
              auto itHash = objImage.find(c_sImageHashKeyWord);
              if (objImage.end() != itHash)
              {
                std::shared_ptr<SEosResourceData> spImg = std::make_shared<SEosResourceData>();
                spImg->m_data.m_type = EResourceType::eImage;
                spImg->m_sHash = itHash.value().toString();
                auto it = objImage.find(c_sImageIdKeyWord);
                if (objImage.end() != it)
                {
                  if (it.value().isString())
                  {
                    spImg->m_data.m_sName = sGaleryKey + it.value().toString();
                  }
                  else
                  {
                    spImg->m_data.m_sName = sGaleryKey + QString::number(it.value().toInt());
                  }
                }
                it = objImage.find(c_sImagewidthKeyWord);
                if (objImage.end() != it)
                {
                  spImg->m_iWidth = it.value().toInt();
                }
                it = objImage.find(c_sImageHeightKeyWord);
                if (objImage.end() != it)
                {
                  spImg->m_iHeight = it.value().toInt();
                }
                spImg->m_sLocatorType = c_sSchemeGallery;
                spImg->m_data.m_sPath = ":/" + ToValidProjectName(sGaleryName) + "/" + spImg->m_data.m_sName;
                currentGallery.insert({spImg->m_data.m_sName, spImg});
              }
            }
          }
        }
      }
    }
  }

  // get file list
  tResourceMap files;
  if (root.end() != filesIt)
  {
    QJsonObject filesObj = filesIt.value().toObject();
    for (auto it = filesObj.constBegin(); filesObj.constEnd() != it; ++it)
    {
      QJsonValue fileObjVal = it.value();
      const QString sFileKey = it.key();
      if (fileObjVal.isObject())
      {
        QJsonObject objFile = fileObjVal.toObject();
        std::shared_ptr<SEosResourceData> spImg = std::make_shared<SEosResourceData>();
        spImg->m_data.m_type = EResourceType::eImage;
        spImg->m_data.m_sName = sFileKey;
        auto it = objFile.find(c_sImageHashKeyWord);
        if (objFile.end() != it)
        {
          spImg->m_sHash = it.value().toString();
        }
        it = objFile.find(c_sImageIdKeyWord);
        if (objFile.end() != it)
        {
          /*
          if (it.value().isString())
          {
            spImg->m_data.m_sName = it.value().toString();
          }
          else
          {
            spImg->m_data.m_sName = QString::number(it.value().toInt());
          }
          */
        }
        it = objFile.find(c_sImageTypeKeyWord);
        if (objFile.end() != it)
        {
          const QString sType = it.value().toString();
          if (auto itType = c_sTypeMap.find(sType); c_sTypeMap.end() != itType)
          {
            spImg->m_data.m_type = itType->second;
          }
        }
        spImg->m_sLocatorType = c_sSchemeFile;
        spImg->m_data.m_sPath = ":/_files/" + spImg->m_data.m_sName;
        files.insert({sFileKey, spImg});
      }
    }
  }

  // find resources in the pages in case of remote resources
  QRegExp rxMatcherImageData(c_sMatcherImageData);
  QRegExp rxOEOS(c_sMatcherOEOSResource);
  std::vector<QJsonValue> valsStart;
  for (QJsonValueRef obj : startPageArr)
  {
    if (obj.isObject())
    {
      std::vector<QJsonValue> valueTemp = FindKeyRecursive(c_sLocatorKeyWord, obj.toObject());
      valsStart.insert(valsStart.end(), valueTemp.begin(), valueTemp.end());
    }
  }
  std::vector<QJsonValue> vals = FindKeyRecursive(c_sLocatorKeyWord, pagesObj);

  // iterate over start page at first
  QStringList vsErrors;
  for (const QJsonValue& value : valsStart)
  {
    QString sError;
    const QString sResource = value.toString();
    if (LookupRemoteLink(sResource, &sError))
    {
      continue;
    }
    if (!sError.isEmpty()) { vsErrors.push_back(sError); sError = QString(); }
    if (LookupGaleryImage(m_resourceMap, sResource, &sError))
    {
      continue;
    }
    if (!sError.isEmpty()) { vsErrors.push_back(sError); sError = QString(); }
    if (LookupFile(files, sResource, &sError))
    {
      continue;
    }
    if (!sError.isEmpty()) { vsErrors.push_back(sError); sError = QString(); }
  }

  // now iterate over everything
  for (const QJsonValue& value : vals)
  {
    QString sError;
    const QString sResource = value.toString();
    if (LookupRemoteLink(sResource, &sError))
    {
      continue;
    }
    if (!sError.isEmpty()) { vsErrors.push_back(sError); sError = QString(); }
    if (LookupGaleryImage(m_resourceMap, sResource, &sError))
    {
      continue;
    }
    if (!sError.isEmpty()) { vsErrors.push_back(sError); sError = QString(); }
    if (LookupFile(files, sResource, &sError))
    {
      continue;
    }
    if (!sError.isEmpty()) { vsErrors.push_back(sError); sError = QString(); }
  }

  // insert a "gallery" for files
  if (files.size() > 0)
  {
    m_resourceMap["_files"].insert(files.begin(), files.end());
  }

  // errors
  if (nullptr != psError && vsErrors.size() > 0)
  {
    *psError = vsErrors.join(", ");
    return true;
  }

  return true;
}

//----------------------------------------------------------------------------------------
//
QByteArray CEosResourceLocator::DownloadResource(
    std::shared_ptr<SEosResourceData> spResource,
    std::function<QByteArray(const QUrl&, QString*)> fnFetch,
    QString* psError)
{
  QByteArray arr;
  QUrl dowloadUrl;
  if (spResource->m_sLocatorType == c_sSchemeGallery)
  {
    dowloadUrl =
        c_sDownloadPath + "tb_xl/" + spResource->m_sHash + ".jpg" + m_sFIX_POLLUTION;
  }
  else if (spResource->m_sLocatorType == c_sSchemeFile)
  {
    auto it =
        std::find_if(c_sExtensionMap.begin(), c_sExtensionMap.end(),
                     [&spResource](const std::pair<QString, std::pair<QString, EResourceType>>& pair) {
      return pair.second.second == spResource->m_data.m_type;
    });
    if (c_sExtensionMap.end() == it)
    {
      if (nullptr != psError) { *psError = "Tried to download unknown file type"; }
      return arr;
    }
    dowloadUrl =
        c_sDownloadPath + spResource->m_sHash + "." + it->first + m_sFIX_POLLUTION;
  }
  else if (spResource->m_sLocatorType == c_sSchemeRemote)
  {
    dowloadUrl = static_cast<QUrl>(spResource->m_data.m_sPath);
  }

  arr = fnFetch(dowloadUrl, psError);
  return arr;
}

//----------------------------------------------------------------------------------------
//
bool CEosResourceLocator::LookupRemoteLink(const QString& sResource, QString* psError)
{
  static QRegExp rxRemote(c_sMatcherRemotePrefix);
  qint32 iPos = 0;

  if ((iPos = rxRemote.indexIn(sResource, iPos)) == -1)
  {
    return false;
  }

  bool bHasAllowedMatch = false;
  for (const QString& sHost : m_vsSupportedHosts)
  {
    iPos = 0;
    QRegExp rxHost(sHost);
    if ((iPos = rxRemote.indexIn(sResource, iPos)) != -1)
    {
      bHasAllowedMatch = true;
      break;
    }
  }

  if (!bHasAllowedMatch)
  {
    if (nullptr != psError)
    { *psError = "Resource host not in whitelist."; }
    return false;
  }

  std::shared_ptr<SEosResourceData> spImg = std::make_shared<SEosResourceData>();
  spImg->m_data.m_type = EResourceType::eImage;
  spImg->m_sHash = m_iIdCounter++;
  spImg->m_data.m_sName = QUrl(sResource).fileName();
  spImg->m_iWidth = -1;
  spImg->m_iHeight = -1;
  spImg->m_data.m_sPath = sResource;
  spImg->m_data.m_sSource = sResource;
  spImg->m_sLocatorType = c_sSchemeRemote;
  m_resourceMap["_remote"].insert({sResource, spImg});

  // set name
  if (m_spTitleImg.isEmpty())
  {
    m_spTitleImg = spImg->m_data.m_sName;
  }

  return true;
}

//----------------------------------------------------------------------------------------
//
bool CEosResourceLocator::LookupEOSResource(const QString& sResource, QString* psError)
{
  Q_UNUSED(sResource)
  Q_UNUSED(psError)
  return false;
}

//----------------------------------------------------------------------------------------
//
bool CEosResourceLocator::LookupGaleryImage(const tGaleryData& gallieries,
                                            const QString& sResource,
                                            QString* psError)
{
  static QRegExp rxGallery(c_sMatcherGallery);
  qint32 iPos = 0;

  QString sGallery;
  QString sId;
  if ((iPos = rxGallery.indexIn(sResource, iPos)) != -1)
  {
    sGallery = rxGallery.cap(1);
    sId = rxGallery.cap(2);
    iPos += rxGallery.matchedLength();
  }
  else
  {
    return false;
  }

  auto itGallery = gallieries.find(sGallery);
  if (gallieries.end() == itGallery)
  {
    if (nullptr != psError) { *psError = QString("Unknown gallery %1.").arg(sGallery); }
    return false;
  }

  if ("*" == sId)
  {
    return true;
  }

  auto itId = std::find_if(itGallery->second.begin(), itGallery->second.end(),
                           [&](const std::pair<QString /*sImgHash*/,
                                              std::shared_ptr<SEosResourceData>>& pair) {
    const QString sName = sGallery + sId;
    return pair.second->m_data.m_sName == sName;
  });
  if (itGallery->second.end() == itId)
  {
    if (nullptr != psError)
    {
      *psError = QString("Unknown image ID in gallery %1: %2.")
          .arg(sGallery).arg(sId);
    }
    return false;
  }

  // set name
  if (m_spTitleImg.isEmpty())
  {
    m_spTitleImg = itId->second->m_data.m_sName;
  }

  return true;
}

//----------------------------------------------------------------------------------------
//
bool CEosResourceLocator::LookupFile(const tResourceMap& files,
                                     const QString& sResource, QString* psError)
{
  static QRegExp rxFile(c_sMatcherFile);
  static QRegExp rxFileExt(c_sMatcherFileExtension);
  static QRegExp rxFileIsRandom(c_sMatcherFileIsRandom);

  qint32 iPos = 0;

  if ((iPos = rxFile.indexIn(sResource, iPos)) == -1)
  {
    return false;
  }

  bool bIsRandom = false;
  QString sExtension;
  iPos = 0;
  if ((iPos = rxFileExt.indexIn(sResource, iPos)) != -1)
  {
    sExtension = rxFileExt.cap(1);
  }
  iPos = 0;
  if ((iPos = rxFileIsRandom.indexIn(sResource, iPos)) != -1)
  {
    bIsRandom = true;
  }

  if (bIsRandom)
  {
    return true;
  }

  auto itExt = c_sExtensionMap.find(sExtension);
  if (c_sExtensionMap.end() == itExt)
  {
    if (nullptr != psError)
    {
      *psError = QString("Unknown file extension %1.").arg(sExtension);
    }
    return false;
  }

  const QString sFile = sResource.mid(5); // remove 'file:'
  auto itFile = files.find(sFile);
  if (files.end() == itFile)
  {
    if (nullptr != psError)
    {
      *psError = QString("Unknown file %1.").arg(sFile);
    }
    return false;
  }

  if (itExt->second.second != itFile->second->m_data.m_type)
  {
    *psError = QString("Unknown file type %1.").arg(itExt->second.first);
    return false;
  }

  // Usually this doesn't contain images, so we won't check
  /*
  if (m_spTitleImg.isEmpty())
  {
    m_spTitleImg = itFile->second->m_data.m_sName;
  }*/

  return true;
}
