/*-------------------------------------------------------------------------------------*/
/*! Logic in this file is mostly a C++ conversion of fapnips openeos software, which can
 *  be found  here:
 *
 * https://github.com/fapnip/openeos
 *
 * Functions which have been directly converted retained their original names.
 **//*----------------------------------------------------------------------------------*/

#include "EosDownloadJob.h"
#include "Application.h"
#include "Settings.h"
#include "ui_EosDownloadJobWidget.h"

#include "RCC/rcc.h"
#include "Systems/DatabaseManager.h"
#include "Systems/EOS/EosHelpers.h"
#include "Systems/Project.h"
#include "Systems/Resource.h"
#include "Systems/ResourceBundle.h"
#include "Systems/PhysFs/PhysFsFileEngine.h"
#include "Utils/RaiiFunctionCaller.h"

#include <nlohmann/json-schema.hpp>

#include <QBuffer>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QDomDocument>
#include <QEventLoop>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPointer>
#include <QRegExp>
#include <QThread>
#include <QUrl>
#include <chrono>

Q_DECLARE_METATYPE(std::shared_ptr<Ui::CEosDownloadJobWidget>)

DECLARE_DOWNLOADJOB(CEosDownloadJob, "EOS/hostcfg", {"milovana.com"})
//DECLARE_JOBSETTINGS_WIDGETPROVIDER(CEosDownloadJobWidgetProvider, CEosDownloadJob)

namespace
{
  const QUrl c_sGetEOSScript = QUrl("https://milovana.com/webteases/geteosscript.php");
  const QUrl c_sGetEOSTease = QUrl("https://milovana.com/webteases/showtease.php");
  const QString c_FIX_POLLUTION = /*QString("&")+QUrl::toPercentEncoding("___oeos:milovana.com")*/ "";

  const QString c_sDataTitle = "data-title";
  const QString c_sDataTitleAlt = "<title>(.*)</title>";
  const QString c_sDataTitleAlt2 = "<span class=\"TopBar_tease__1kRgf\">(.*)</span>";
  const QString c_sDataTitleAlt3 = "<h1>(.*)</h1>";
  const QString c_sDataAuthor = "data-author";
  const QString c_sDataAuthorAlt = "<button class=\"TopBar_authorButton__Nrip5\">(.*)</button>";
  const QString c_sDataAuthorAlt2 = "<h2>(.*)</h2>";
  const QString c_sDataAuthorId = "data-author-id";
  const QString c_sDataTeaseId = "data-tease-id";
  const QString c_sDataKey = "data-key";

  const qint32 c_iResourceLibraryFormatVersion = 3;
  const qint32 c_iMaxResourceBlobSize = 1'000'000'000; // max resource blob size is 1GB to reduce memory usage
  const QString c_sScriptResourceName = "script";
  const QString c_sResourceBlobName = "resource";

  const QString c_sSceneModelName = "SceneModel";

  // default trusted media hosts
  const std::vector<QString> c_vsSupportedHosts = {
    "thumbs[0-9]*\\.*gfycat\\.com\\/.+",
    "thumbs[0-9]*\\.*redgifs\\.com\\/.+",
    "w*[0-9]*\\.*mboxdrive\\.com\\/.+",
    "media[0-9]*\\.vocaroo\\.com\\/.+",
    "iili\\.io\\/.+",
    "i\\.ibb\\.co\\/.",
    "media\\.milovana\\.com\\/.+"
  };

  void InstructionSetFormatChecker(const std::string& format, const std::string& value)
  {
    Q_UNUSED(value)
    if (format == "something")
    {
      //if (!check_value_for_something(value))
      //  throw std::invalid_argument("value is not a good something");
    }
    else
    {
      //throw std::logic_error("Don't know how to validate " + format);
    }
  }

  //--------------------------------------------------------------------------------------
  //
  /* json-parse the people - with custom error handler */
  class CustomErrorHandler : public nlohmann::json_schema::basic_error_handler
  {
  public:
    CustomErrorHandler(QString* pError) : m_pError(pError)
    {
    }

    void error(const nlohmann::json_pointer<nlohmann::basic_json<>> &pointer,
               const nlohmann::json &instance,
               const std::string &message) override
    {
        nlohmann::json_schema::basic_error_handler::error(pointer, instance, message);
        if (nullptr != m_pError)
        {
          *m_pError =
              QString("Validation of schema failed:") + QString::fromStdString(pointer.to_string()) +
                      ":" + QString::fromStdString(message);
        }
    }

    QString* m_pError;
  };


  //--------------------------------------------------------------------------------------
  //
  std::chrono::system_clock::duration TimeSinceEpoch()
  {
    using namespace std::chrono;
    auto currentTime = system_clock::now();
    return currentTime.time_since_epoch();
  }

  //--------------------------------------------------------------------------------------
  //
  /*
  bool IsMinutesDividableBy3()
  {
    std::chrono::minutes m =
        std::chrono::duration_cast<std::chrono::minutes>(TimeSinceEpoch());
    qint32 iNumNinutes = m.count();
    return (iNumNinutes % 3) == 0;
  }
  */
}

//----------------------------------------------------------------------------------------
//
CEosDownloadJobWidgetProvider::CEosDownloadJobWidgetProvider() :
  IDownloadJobWidgetProvider()
{}
CEosDownloadJobWidgetProvider::~CEosDownloadJobWidgetProvider()
{}

//----------------------------------------------------------------------------------------
//
QWidget* CEosDownloadJobWidgetProvider::operator()() const
{
  QWidget* pWidget = new QWidget();
  auto spWidgetUi = std::make_shared<Ui::CEosDownloadJobWidget>();
  spWidgetUi->setupUi(pWidget);
  pWidget->setProperty("UI", QVariant::fromValue<std::shared_ptr<Ui::CEosDownloadJobWidget>>(spWidgetUi));
  return pWidget;
}

//----------------------------------------------------------------------------------------
//
CEosDownloadJob::CEosDownloadJob(QObject* pParent) :
  QObject(pParent),
  IDownloadJob(),
  m_spNetworkAccessManager(nullptr),
  m_spResourceLib(nullptr),
  m_spProject(nullptr),
  m_iProgress(0),
  m_sName("EOS"),
  m_sError("No error."),
  m_iProjId(-1),
  m_iFileBlobCounter(0)
{
}

CEosDownloadJob::~CEosDownloadJob()
{
}

//----------------------------------------------------------------------------------------
//
QString CEosDownloadJob::Error()
{
  return m_sError;
}

//----------------------------------------------------------------------------------------
//
bool CEosDownloadJob::Finished()
{
  return true;
}

//----------------------------------------------------------------------------------------
//
QString CEosDownloadJob::JobName() const
{
  return m_sName;
}

//----------------------------------------------------------------------------------------
//
qint32 CEosDownloadJob::Progress() const
{
  return m_iProgress;
}

//----------------------------------------------------------------------------------------
//
bool CEosDownloadJob::Run(const QVariantList& args)
{
  assert(1 == args.size());
  if (1 != args.size())
  {
    m_sError = QString("1 argument was expected, got %1.").arg(args.size());
    return false;
  }

  const QUrl dlUrl = args[0].toUrl();

  m_iProgress = 0;
  emit SignalStarted(m_iProjId);
  emit SignalProgressChanged(m_iProjId, Progress());

  // only allow downloads every 3 minutes to not strain eos servers too much
  /*
  while (!IsMinutesDividableBy3())
  {
    thread()->sleep(10);
  }
  */

  m_spNetworkAccessManager.reset(new QNetworkAccessManager());
  CRaiiFunctionCaller resetCaller([this](){
    m_spNetworkAccessManager.reset();
  });
  std::shared_ptr<CDatabaseManager> spDbManager =
    CApplication::Instance()->System<CDatabaseManager>().lock();
  if (nullptr == spDbManager)
  {
    m_sError = QString("Internal Error: Database Manager was not found.");
    return false;
  }

  // create and get new project
  tvfnActionsProject vfnActions = {
    [this](const tspProject& spProjectCallback) {
      spProjectCallback->m_dlState = EDownLoadState::eDownloadRunning;
      m_spProject = spProjectCallback;
    }
  };

#if defined(Q_OS_ANDROID)
  // TODO:
  const QString sBaseProjectPath = CApplication::Instance()->Settings()->ContentFolder();
#else
  const QString sBaseProjectPath = CApplication::Instance()->Settings()->ContentFolder();
#endif

  m_iProjId = spDbManager->AddProject(QDir(sBaseProjectPath + "/TBD"), 1, false, true, vfnActions);
  spDbManager->PrepareNewProject(m_iProjId);
  spDbManager->SerializeProject(m_iProjId, true);
  if (0 > m_iProjId)
  {
    m_sError = "Could not create new project.";
    return false;
  }

  // offline test
  //QString sTeaseId = "41639";
  //QFile testFile("testScript.json");
  //testFile.open(QIODevice::ReadOnly);
  //QByteArray arr = testFile.readAll();
  //ValidateJson(arr, &m_sError);
  //QJsonDocument jsonScript = QJsonDocument::fromJson(arr);

  // Get Script
  QString sTeaseId;
  QJsonDocument jsonScript;
  QString sError;
  if (!RequestRemoteScript(dlUrl, sTeaseId, jsonScript, &sError))
  {
    if (sError.isEmpty())
    {
      m_sError = QString("Could not parse tease script.\nOnly EOS teases are supported.");
    }
    else
    {
      m_sError = sError;
    }
    return false;
  }

  ABORT_CHECK(m_iProjId)

  // Get Metatate from html file
  SScriptMetaData metaData;
  //QString sError;
  if (!RequestRemoteScriptMetadata(sTeaseId, metaData, &sError))
  {
    if (sError.isEmpty())
    {
      m_sError = QString("Could not parse tease script.\nOnly EOS teases are supported.");
    }
    else
    {
      m_sError = sError;
    }
    return false;
  }

  // offline test
  //QFile testFile2("testHtml.html");
  //testFile2.open(QIODevice::ReadOnly);
  //arr = testFile2.readAll();
  //ParseHtml(arr, metaData, &m_sError);

  ABORT_CHECK(m_iProjId)

  // complete project details
  const QString sKey = eos::c_sKey + eos::GetEOSK();
  if (nullptr != m_spProject)
  {
    QWriteLocker locker(&m_spProject->m_rwLock);
    m_spProject->m_sDescribtion =
      "<h1>" + metaData.m_sTitle + "</h1><br>" +
      "Created by: <i>" + metaData.m_sAuthor + "</b><br>" +
      "Downloaded from Milovana on " + QDateTime::currentDateTime().toString() + "";
    QCryptographicHash hash(QCryptographicHash::Algorithm::Sha256);
    hash.addData(QString(metaData.m_sTitle + sKey).toUtf8());
    m_spProject->m_sUserData = QString::fromUtf8(hash.result().toBase64());
  }
  spDbManager->RenameProject(m_iProjId, ToValidProjectName(metaData.m_sTitle));
  spDbManager->SerializeProject(m_iProjId, true);

  // emit another progress update to let the displays update
  emit SignalProgressChanged(m_iProjId, Progress());

  ABORT_CHECK(m_iProjId)

  QBuffer errorBuffer;
  errorBuffer.open(QIODevice::ReadOnly);
  m_spResourceLib  = std::make_shared<RCCResourceLibrary>(c_iResourceLibraryFormatVersion);
  m_spResourceLib->setCompressionAlgorithm(RCCResourceLibrary::CompressionAlgorithm::None);
  m_spResourceLib->setFormat(RCCResourceLibrary::Binary);
  m_spResourceLib->readFiles(false, errorBuffer);

  ABORT_CHECK(m_iProjId)

  // locate all resources in the script
  CEosResourceLocator locator(jsonScript, c_vsSupportedHosts, c_FIX_POLLUTION);
  if (!locator.LocateAllResources(&sError))
  {
    m_sError = (m_sError.isEmpty() ? "" : "; ") + sError;
    return false;
  }
  else if (!sError.isEmpty())
  {
    m_sError += (m_sError.isEmpty() ? "" : "; ") + sError;
    qDebug() << sError;
  }

  ABORT_CHECK(m_iProjId)

  sError = QString();
  CEosPagesToScenesTransformer pagesTransformer(jsonScript);
  if (!pagesTransformer.CollectScenes(&sError))
  {
    m_sError = (m_sError.isEmpty() ? "" : "; ") + sError;
    return false;
  }
  else if (!sError.isEmpty())
  {
    m_sError += (m_sError.isEmpty() ? "" : "; ") + sError;
    qDebug() << sError;
  }

  ABORT_CHECK(m_iProjId)

  if (!locator.m_spTitleImg.isEmpty())
  {
    if (nullptr != m_spProject)
    {
      QWriteLocker locker(&m_spProject->m_rwLock);
      m_spProject->m_sTitleCard = locator.m_spTitleImg;
    }
    spDbManager->SerializeProject(m_iProjId, true);
  }

  ABORT_CHECK(m_iProjId)

  // get number of total progress
  qint32 iMaxProgress = 0;
  for (const auto& itGallery : locator.m_resourceMap)
  {
    iMaxProgress += static_cast<qint32>(itGallery.second.size());
  }
  iMaxProgress += static_cast<qint32>(pagesTransformer.m_vPages.size());


  sError = QString();
  if (!CreateResourceFiles(locator.m_resourceMap,
                           iMaxProgress,
                           errorBuffer, locator,
                           &sError))
  {
    m_sError += (m_sError.isEmpty() ? "" : "; ") + sError;
    return false;
  }

  ABORT_CHECK(m_iProjId)

  sError = QString();
  if (!CreateScriptFiles(jsonScript,
                         pagesTransformer.m_vPages,
                         iMaxProgress,
                         errorBuffer, pagesTransformer,
                         &sError))
  {
    m_sError += (m_sError.isEmpty() ? "" : "; ") + sError;
    return false;
  }

  ABORT_CHECK(m_iProjId)

  // set downloadstate to finished
  m_spProject->m_rwLock.lockForWrite();
  m_spProject->m_dlState = EDownLoadState::eFinished;
  m_spProject->m_rwLock.unlock();

  // save the project
  spDbManager->SerializeProject(m_iProjId, true);

  emit SignalFinished(m_iProjId);

  return true;
}

//----------------------------------------------------------------------------------------
//
void CEosDownloadJob::AbortImpl()
{
  m_sError = QString("Download stopped.");
  m_spNetworkAccessManager.reset();

  std::shared_ptr<CDatabaseManager> spDbManager =
    CApplication::Instance()->System<CDatabaseManager>().lock();
  if (nullptr != spDbManager && -1 != m_iProjId)
  {
    spDbManager->SerializeProject(m_iProjId);
  }

  // write the resources collected so far
  WriteResourceBlob(m_spProject, QString("==="), m_iFileBlobCounter);
}

//----------------------------------------------------------------------------------------
//
bool CEosDownloadJob::CreateResourceFiles(const CEosResourceLocator::tGaleryData& resourceMap,
                                          qint32 iMaxProgress,
                                          QBuffer& errorBuffer,
                                          CEosResourceLocator& locator,
                                          QString* psError)
{
  // already checked for nullptr and must be != 0 at this point
  std::shared_ptr<CDatabaseManager> spDbManager =
    CApplication::Instance()->System<CDatabaseManager>().lock();

  QString sResource;
  tspResource spResource;
  qint32 iCollectiveSize = 0;
  if (0 < iMaxProgress)
  {
    for (const auto& itGallery : resourceMap)
    {
      m_iFileBlobCounter = 0;
      QString sFileBlob =  itGallery.first +
          "." + QString::number(m_iFileBlobCounter) +
          "." + joip_resource::c_sResourceBundleSuffix;
      if (!spDbManager->AddResourceArchive(
            m_spProject, QUrl(CPhysFsFileEngineHandler::c_sScheme + sFileBlob)))
      {
        if (nullptr != psError)
        {
          *psError += ((!psError->isEmpty()) ? "; " : "") +
              QString("Could not create resource bundle.").arg(sFileBlob);
        }
        return false;
      }

      for (const auto& itFile : itGallery.second)
      {
        QString sError;
        QByteArray arr =
          locator.DownloadResource(itFile.second,
                                   std::bind(&CEosDownloadJob::Fetch, this,
                                             std::placeholders::_1, std::placeholders::_2),
                                   &sError);
        ABORT_CHECK(m_iProjId)

        // filesize would be too large, create new file
        if (c_iMaxResourceBlobSize < iCollectiveSize + arr.size())
        {
          if (!WriteResourceBlob(m_spProject, sFileBlob, m_iFileBlobCounter)) { return false; }
          sFileBlob =  itGallery.first +
              "." + QString::number(m_iFileBlobCounter) +
              "." + joip_resource::c_sResourceBundleSuffix;
          if (!spDbManager->AddResourceArchive(m_spProject,
                                          QUrl(CPhysFsFileEngineHandler::c_sScheme + sFileBlob)))
          { sError = QString("Could not create resource bundle.").arg(sFileBlob); return false; }

          ResetResourceLibrary(errorBuffer);
          iCollectiveSize = 0;
        }
        // add file to resources
        {
          iCollectiveSize += arr.size();

          const QString sName = itFile.second->m_data.m_sName;
          QUrl urlCopy = itFile.second->m_data.m_sPath;
          const QString sPath = urlCopy.toString();
          RCCFileInfo fileInfo(sName, QFileInfo(sPath), QLocale::C,
                               QLocale::AnyCountry, RCCFileInfo::NoFlags);
          fileInfo.m_prefilledContent = arr;
          m_spResourceLib->addFile(sName, fileInfo);
          sResource = spDbManager->AddResource(m_spProject, urlCopy,
                                               itFile.second->m_data.m_type, sName);
          spResource = spDbManager->FindResourceInProject(m_spProject, sResource);
          if (nullptr == spResource) { sError = QString("Resource error."); return false; }

          QWriteLocker resourceLocker(&spResource->m_rwLock);
          spResource->m_sResourceBundle = QFileInfo(sFileBlob).completeBaseName();
        }

        ABORT_CHECK(m_iProjId)

        // offline test:
        //thread()->sleep(1);

        ++m_iProgressCounter;
        m_iProgress = static_cast<qint32>(100.0 * static_cast<double>(m_iProgressCounter) / iMaxProgress);
        emit SignalProgressChanged(m_iProjId, Progress());

        // wait a bit to not overload eos servers
        thread()->sleep(1);

        // add error to List
        if (sError.isEmpty())
        {
          if (nullptr != psError) { *psError += ((!psError->isEmpty()) ? "; " : "") + sError; }
        }
      }

      if (!WriteResourceBlob(m_spProject, sFileBlob, m_iFileBlobCounter)) { return false; }
      ResetResourceLibrary(errorBuffer);
      iCollectiveSize = 0;
    }
  }

  return true;
}

//----------------------------------------------------------------------------------------
//
bool CEosDownloadJob::CreateScriptFiles(const QJsonDocument& entireScript,
                                        const std::vector<CEosPagesToScenesTransformer::SPageScene>& vScenes,
                                        qint32 iMaxProgress,
                                        QBuffer& errorBuffer,
                                        CEosPagesToScenesTransformer& sceneTransformer,
                                        QString* psError)
{
  // already checked for nullptr and must be != 0 at this point
  std::shared_ptr<CDatabaseManager> spDbManager =
    CApplication::Instance()->System<CDatabaseManager>().lock();

  QString sError;
  QString sResource;
  tspResource spResource;

  QString sFileBlob = c_sSceneModelName + "." + joip_resource::c_sResourceBundleSuffix;
  if (!spDbManager->AddResourceArchive(m_spProject,
                        QUrl(CPhysFsFileEngineHandler::c_sScheme + sFileBlob)))
  { sError = QString("Could not create resource bundle.").arg(sFileBlob); return false; }


  // write entire script into file
  RCCFileInfo fileInfoScript(QString(c_sScriptResourceName + c_sScriptFormat),
                             QFileInfo(c_sScriptResourceName + c_sScriptFormat),
                             QLocale::C, QLocale::AnyCountry, RCCFileInfo::NoFlags);
  fileInfoScript.m_prefilledContent = entireScript.toJson();
  m_spResourceLib->addFile(QString(":/" + c_sScriptResourceName + c_sScriptFormat), fileInfoScript);
  sResource =
      spDbManager->AddResource(m_spProject, QUrl("qrc:" + c_sScriptResourceName + c_sScriptFormat),
                               EResourceType::eScript, c_sScriptResourceName);
  spResource =
      spDbManager->FindResourceInProject(m_spProject, sResource);
  {
    QWriteLocker resourceLocker(&spResource->m_rwLock);
    spResource->m_sResourceBundle = QFileInfo(sFileBlob).completeBaseName();
  }


  // write all pages into own file
  for (const auto& scenePage : vScenes)
  {
    tspScene spScene =
        sceneTransformer.AddPageToScene(m_iProjId, m_spProject, scenePage);
    if (nullptr == spScene) { sError = QString("Scene error."); return false; }

    const QString sName = scenePage.m_sName;
    const QString sPath = c_sSceneModelName;
    RCCFileInfo fileInfo(QString(sName), QFileInfo(":/" + sPath + "/" + sName), QLocale::C,
                         QLocale::AnyCountry, RCCFileInfo::NoFlags);
    fileInfo.m_prefilledContent = scenePage.m_pageScript.toJson();
    m_spResourceLib->addFile(sName, fileInfo);
    sResource = spDbManager->AddResource(m_spProject, QUrl("qrc:" + sPath + "/" + sName + c_sScriptFormat),
                                         EResourceType::eScript, sName);
    spResource = spDbManager->FindResourceInProject(m_spProject, sResource);
    if (nullptr == spResource) { sError = QString("Resource error."); return false; }

    {
      QWriteLocker sceneLocker(&spScene->m_rwLock);
      spScene->m_sScript = sResource;
    }

    {
      QWriteLocker resourceLocker(&spResource->m_rwLock);
      spResource->m_sResourceBundle = QFileInfo(sFileBlob).completeBaseName();
    }

    ABORT_CHECK(m_iProjId)

    // offline test:
    //thread()->sleep(1);

    ++m_iProgressCounter;
    m_iProgress = static_cast<qint32>(100.0 * static_cast<double>(m_iProgressCounter) / iMaxProgress);
    emit SignalProgressChanged(m_iProjId, Progress());

    // add error to List
    if (sError.isEmpty())
    {
      if (nullptr != psError) { *psError += ((!psError->isEmpty()) ? "; " : "") + sError; }
    }
  }

  ABORT_CHECK(m_iProjId)

  // write .flow file
  QByteArray arr = sceneTransformer.CompileScenes();

  const QString sName = joip_resource::c_sSceneModelFile;
  RCCFileInfo fileInfo(QString(sName), QFileInfo(":/" + sName), QLocale::C,
                       QLocale::AnyCountry, RCCFileInfo::NoFlags);
  fileInfo.m_prefilledContent = arr;
  m_spResourceLib->addFile(sName, fileInfo);
  sResource = spDbManager->AddResource(m_spProject, QUrl("qrc:" + sName),
                                       EResourceType::eOther, sName);
  spResource = spDbManager->FindResourceInProject(m_spProject, sResource);
  if (nullptr == spResource) { sError = QString("Resource error."); return false; }

  QWriteLocker resourceLocker(&spResource->m_rwLock);
  spResource->m_sResourceBundle = QFileInfo(sFileBlob).completeBaseName();


  if (nullptr != m_spProject)
  {
    QWriteLocker locker(&m_spProject->m_rwLock);
    m_spProject->m_sSceneModel = sName;
  }
  spDbManager->SerializeProject(m_iProjId, true);

  if (!WriteResourceBlob(m_spProject, sFileBlob, m_iFileBlobCounter)) { return false; }
  ResetResourceLibrary(errorBuffer);

  return true;
}

//----------------------------------------------------------------------------------------
//
QUrl CEosDownloadJob::EncodeForCorsProxy(const QUrl& url, const QString& sQuery)
{
  QUrl urlCopy = url;
  urlCopy.setQuery(sQuery);
  return urlCopy;
}

//----------------------------------------------------------------------------------------
//
QByteArray CEosDownloadJob::Fetch(const QUrl& url, QString* psError)
{
  QByteArray arr;
  QEventLoop loop;
  QPointer<QNetworkReply> pReply =
      m_spNetworkAccessManager->get(QNetworkRequest(url));
  connect(pReply, &QNetworkReply::finished,
          this, [pReply, &loop, &arr, &psError](){
    if(nullptr != pReply)
    {
      QUrl url = pReply->url();
      arr = pReply->readAll();
      bool bOk = QMetaObject::invokeMethod(&loop, "quit", Qt::QueuedConnection);
      assert(bOk); Q_UNUSED(bOk)
    }
    else
    {
      if (nullptr != psError) { *psError = "NetworkReply object was destroyed too early."; }
    }
  });
  connect(pReply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
          this, [pReply, &loop, &psError](QNetworkReply::NetworkError error){
    Q_UNUSED(error)
    if (nullptr != pReply)
    {
      if (nullptr != psError) { *psError =
            tr(QT_TR_NOOP("Error fetching remote resource: %1"))
                          .arg(pReply->errorString()); }
      bool bOk = QMetaObject::invokeMethod(&loop, "quit", Qt::QueuedConnection);
      assert(bOk); Q_UNUSED(bOk)
    }
    else
    {
      if (nullptr != psError) { *psError = "NetworkReply object was destroyed too early."; }
    }
  });

  loop.exec();

  if (nullptr != pReply) { delete pReply; }
  else { return QByteArray(); }
  return arr;
}

//----------------------------------------------------------------------------------------
//
bool CEosDownloadJob::ParseHtml(const QByteArray& arr, SScriptMetaData& data, QString* psError)
{
  Q_UNUSED(psError)
  // we won't parse the html document, as it might be not well formated
  // instead we just regexp out the info we need
  /*
  QDomDocument html;
  bool bOk = html.setContent(arr, false, psError);
  if (!bOk)
  {
    return false;
  }
  */

  QRegExp nodesTitle = QRegExp(c_sDataTitle + "=\"(.*)\"");
  QRegExp nodesAuthor = QRegExp(c_sDataAuthor + "=\"(.*)\"");
  QRegExp nodesAuthorId = QRegExp(c_sDataAuthorId + "=\"(.*)\"");
  QRegExp nodesTeaseId = QRegExp(c_sDataTeaseId + "=\"(.*)\"");
  QRegExp nodesDataKey = QRegExp(c_sDataKey + "=\"(.*)\"");

  QString sStr = QString::fromUtf8(arr);
  auto fnMatch = [&sStr](const QRegExp& node, QString& sSaveIn) {
    qint32 iPos = 0;
    while ((iPos = node.indexIn(sStr, iPos)) != -1)
    {
        sSaveIn = node.cap(1);
        qint32 iEnd = sSaveIn.indexOf('\"');
        if (-1 != iEnd)
        {
          sSaveIn = sSaveIn.mid(0, iEnd);
        }
        iPos += node.matchedLength();
    }
  };

  fnMatch(nodesTitle, data.m_sTitle);
  fnMatch(nodesAuthor, data.m_sAuthor);
  fnMatch(nodesAuthorId, data.m_sAuthorId);
  fnMatch(nodesTeaseId, data.m_sTeaseId);
  fnMatch(nodesDataKey, data.m_sDataKey);

  if (data.m_sTitle.isEmpty())
  {
    fnMatch(QRegExp(c_sDataTitleAlt), data.m_sTitle);
    if (data.m_sTitle.isEmpty())
    {
      fnMatch(QRegExp(c_sDataTitleAlt2), data.m_sTitle);
      if (data.m_sTitle.isEmpty())
      {
        fnMatch(QRegExp(c_sDataTitleAlt3), data.m_sTitle);
      }
    }
  }
  if (data.m_sAuthor.isEmpty())
  {
    fnMatch(QRegExp(c_sDataAuthorAlt), data.m_sAuthor);
    if (data.m_sAuthor.isEmpty())
    {
      fnMatch(QRegExp(c_sDataAuthorAlt2), data.m_sAuthor);
      data.m_sAuthor.replace("by ", "");
    }
  }

  return true;
}

//----------------------------------------------------------------------------------------
//
QString CEosDownloadJob::ParseTeaseURI(const QUrl& url)
{
  QString sTeaseId;
  QRegExp rx("id=([0-9a-z]+.*)");
  qint32 iPos = 0;
  while ((iPos = rx.indexIn(url.query(), iPos)) != -1)
  {
    sTeaseId = rx.cap(1);
    iPos += rx.matchedLength();
  }
  return sTeaseId;
}

//----------------------------------------------------------------------------------------
//
void CEosDownloadJob::ResetResourceLibrary(QBuffer& errorBuffer)
{
  errorBuffer.reset();
  m_spResourceLib.reset(new RCCResourceLibrary(c_iResourceLibraryFormatVersion));
  m_spResourceLib->setCompressionAlgorithm(RCCResourceLibrary::CompressionAlgorithm::None);
  m_spResourceLib->setFormat(RCCResourceLibrary::Binary);
  m_spResourceLib->readFiles(false, errorBuffer);
}

//----------------------------------------------------------------------------------------
//
bool CEosDownloadJob::RequestRemoteScript(const QUrl& url, QString& sTeaseId,
                                          QJsonDocument& jsonScript,
                                          QString* psError)
{
  qint32 iMinutes = 0;
  {
    using namespace std::chrono;
    minutes min = duration_cast<minutes>(TimeSinceEpoch());
    iMinutes = min.count();
  }

  sTeaseId = ParseTeaseURI(url);
  if (sTeaseId.isEmpty())
  {
    if (nullptr != psError) { *psError = "Could not parse tease id."; }
    return false;
  }

  const QUrl sFinalrequest = EncodeForCorsProxy(c_sGetEOSScript,
                                QString("id=%1%2%3")
                                .arg(sTeaseId)
                                .arg(c_FIX_POLLUTION)
                                .arg("&cacheable&_nc=" + QString::number(iMinutes)));
  if (sFinalrequest.isEmpty())
  {
    if (nullptr != psError) { *psError = "Could not create request."; }
    return false;
  }

  QPointer<CEosDownloadJob> pThis(this);
  QByteArray arr = Fetch(sFinalrequest, psError);
  if (nullptr == pThis) { return false; }
  if (arr.isEmpty())
  {
    if (nullptr != psError) { *psError = "Fetched resource was empty."; }
    return false;
  }

  // test write to file
  //QFile file("testScript.json");
  //file.open(QIODevice::WriteOnly);
  //file.write(arr);

  if (!ValidateJson(arr, psError))
  {
    return false;
  }

  jsonScript = QJsonDocument::fromJson(arr);
  return jsonScript.isObject();
}

//----------------------------------------------------------------------------------------
//
bool CEosDownloadJob::RequestRemoteScriptMetadata(
    const QString& sId, SScriptMetaData& data,
    QString* psError)
{
  qint32 iMinutes = 0;
  {
    using namespace std::chrono;
    minutes min = duration_cast<minutes>(TimeSinceEpoch());
    iMinutes = min.count();
  }

  data.m_sTeaseId = sId;
  const QUrl sFinalrequest = EncodeForCorsProxy(c_sGetEOSTease,
                                QString("id=%1%2%3")
                                .arg(sId)
                                .arg(c_FIX_POLLUTION)
                                .arg("&cacheable&_nc=" + QString::number(iMinutes)));
  if (sFinalrequest.isEmpty())
  {
    if (nullptr != psError) { *psError = "Could not create request."; }
    return false;
  }

  QPointer<CEosDownloadJob> pThis(this);
  QByteArray arr = Fetch(sFinalrequest, psError);
  if (nullptr == pThis) { return false; }
  if (arr.isEmpty())
  {
    if (nullptr != psError) { *psError = "Fetched resource was empty."; }
    return false;
  }

  // test write to file
  //QFile file("testHtml.html");
  //file.open(QIODevice::WriteOnly);
  //file.write(arr);

  if (!ParseHtml(arr, data, psError))
  {
    return false;
  }


  return true;
}

//----------------------------------------------------------------------------------------
//
bool CEosDownloadJob::ValidateJson(const QByteArray& arr, QString* psError)
{
  CustomErrorHandler errorHandler(psError);
  nlohmann::json_schema::json_validator validator(nullptr, InstructionSetFormatChecker, nullptr);

  nlohmann::json script;
  try
  {
    script = nlohmann::json::parse(QString::fromUtf8(arr).toStdString());
  }
  catch (const std::exception &e)
  {
    if (nullptr != psError)
     { *psError = QString(QT_TR_NOOP("Loading json failed: %1")).arg(e.what()); }
    return false;
  }

  QFile schemaFile(":/EosSchema.json");
  if (!schemaFile.open(QIODevice::ReadOnly))
  {
    if (nullptr != psError)
    { *psError = QString(QT_TR_NOOP("Failed to open validation schema file.")); }
    return false;
  }

  nlohmann::json schema;
  try
  {
    schema = nlohmann::json::parse(QString::fromUtf8(schemaFile.readAll()).toStdString());
  }
  catch (const std::exception &e)
  {
    if (nullptr != psError)
     { *psError = QString(QT_TR_NOOP("Loading of schema failed: %1"))
                          .arg(e.what()); }
    return false;
  }

  try
  {
    validator.set_root_schema(schema);
  }
  catch (const std::exception &e)
  {
    if (nullptr != psError)
     { *psError = QString(QT_TR_NOOP("Validation of schema failed: %1"))
                          .arg(e.what()); }
    return false;
  }

  bool bOk = true;
  validator.validate(script, errorHandler);
  if (static_cast<bool>(errorHandler))
  {
    bOk = false;
  }

  return bOk;
}

//----------------------------------------------------------------------------------------
//
bool CEosDownloadJob::WriteResourceBlob(const tspProject& spProject,
                                        const QString& sName, qint32& iBlob)
{
  const QString sProjName = PhysicalProjectName(spProject);
  QFile out(CApplication::Instance()->Settings()->ContentFolder() + "/" + sProjName + "/" +
            sName);
  QFile temp;
  QFile errorDevice;

  if (!out.open(QIODevice::WriteOnly))
  { m_sError = QString("Could not open %1 for writing.").arg(out.fileName()); }

  if (nullptr != m_spResourceLib)
  {
    QCryptographicHash hash(QCryptographicHash::Algorithm::Sha256);
    hash.addData(sName.toUtf8());
    const QString sKeyFile = QString::fromUtf8(hash.result().toBase64()).replace(QRegExp("\\=|\\/"),"") + eos::c_sEosKeyFile;
    RCCFileInfo fileInfo(QString(sKeyFile), QFileInfo(":/" + sKeyFile), QLocale::C,
                         QLocale::AnyCountry, RCCFileInfo::NoFlags);
    fileInfo.m_prefilledContent = QString(eos::c_sKey + eos::GetEOSK()).toUtf8();
    m_spResourceLib->addFile(sKeyFile, fileInfo);

    bool success = m_spResourceLib->output(out, temp, errorDevice);
    if (!success)
    {
        // erase the output file if we failed
        out.remove();
        m_sError = QString("Could not write Reosurce file: %1.").arg(out.fileName());
        return false;
    }
    ++iBlob;
    return true;
  }
  else
  {
    m_sError = QString("Could not write Reosurce file: Internal error.");
    return false;
  }
}
