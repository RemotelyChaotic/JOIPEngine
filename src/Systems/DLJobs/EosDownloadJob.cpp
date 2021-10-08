/*-------------------------------------------------------------------------------------*/
/*! Logic in this file is mostly a C++ conversion of fapnips openeos software, which can
 *  be found  here:
 *
 * https://github.com/fapnip/openeos
 *
 * Functions which have been directly converted retained their original names.
 **//*----------------------------------------------------------------------------------*/

#include "EosDownloadJob.h"
#include "ui_EosDownloadJobWidget.h"
#include <QEventLoop>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPointer>
#include <QRegExp>
#include <QThread>
#include <QUrl>
#include <chrono>

Q_DECLARE_METATYPE(std::shared_ptr<Ui::CEosDownloadJobWidget>)

namespace
{
  const QUrl c_sGetEOSScript = QUrl("https://milovana.com/webteases/geteosscript.php");
  const QString c_FIX_POLLUTION = /*QString("&")+QUrl::toPercentEncoding("___oeos:milovana.com")*/ "";

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
  bool IsMinutesDividableBy3()
  {
    std::chrono::minutes m =
        std::chrono::duration_cast<std::chrono::minutes>(TimeSinceEpoch());
    qint32 iNumNinutes = m.count();
    return (iNumNinutes % 3) == 0;
  }
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
  m_spNetworkAccessManager(std::make_shared<QNetworkAccessManager>()),
  m_iProgress(0)
{
}

CEosDownloadJob::~CEosDownloadJob()
{

}

//----------------------------------------------------------------------------------------
//
bool CEosDownloadJob::Finished()
{
  return true;
}

//----------------------------------------------------------------------------------------
//
qint32 CEosDownloadJob::Progress()
{
  return m_iProgress;
}

//----------------------------------------------------------------------------------------
//
void CEosDownloadJob::Run(const QVariantList& args)
{
  assert(1 == args.size());
  if (1 != args.size()) { return; }

  const QUrl dlUrl = args[0].toUrl();

  // only allow downloads every 3 minutes to not strain eos servers too much
  //while (!IsMinutesDividableBy3())
  //{
  //  thread()->sleep(10);
  //}

  m_iProgress = 0;
  emit SignalStarted();
  emit SignalProgressChanged(Progress());

  QString sTeaseId;
  QJsonDocument jsonScript;
  if (!RequestRemoteScript(dlUrl, sTeaseId, jsonScript))
  {
    return;
  }

  ABORT_CHECK

  emit SignalFinished();
}

//----------------------------------------------------------------------------------------
//
void CEosDownloadJob::AbortImpl()
{

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
QByteArray CEosDownloadJob::Fetch(const QUrl& url)
{
  QByteArray arr;
  QEventLoop loop;
  QPointer<QNetworkReply> pReply =
      m_spNetworkAccessManager->get(QNetworkRequest(url));
  connect(pReply, &QNetworkReply::finished,
          this, [pReply, &loop, &arr](){
    if(nullptr != pReply)
    {
      QUrl url = pReply->url();
      arr = pReply->readAll();
      bool bOk = QMetaObject::invokeMethod(&loop, "quit", Qt::QueuedConnection);
      assert(bOk); Q_UNUSED(bOk)
    }
    else
    {
      qCritical() << "QNetworkReply object was destroyed too early.";
    }
  });
  connect(pReply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
          this, [pReply, &loop](QNetworkReply::NetworkError error){
    Q_UNUSED(error)
    if (nullptr != pReply)
    {
      qWarning() << tr(QT_TR_NOOP("Error fetching remote resource: %1"))
                    .arg(pReply->errorString());
      bool bOk = QMetaObject::invokeMethod(&loop, "quit", Qt::QueuedConnection);
      assert(bOk); Q_UNUSED(bOk)
    }
    else
    {
      qCritical() << "QNetworkReply object was destroyed too early.";
    }
  });

  loop.exec();

  if (nullptr != pReply) { delete pReply; }
  else { return QByteArray(); }
  return arr;
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
bool CEosDownloadJob::RequestRemoteScript(const QUrl& url, QString& sTeaseId,
                                          QJsonDocument& jsonScript)
{
  qint32 iMinutes = 0;
  {
    using namespace std::chrono;
    minutes min = duration_cast<minutes>(TimeSinceEpoch());
    iMinutes = min.count();
  }

  sTeaseId = ParseTeaseURI(url);
  if (sTeaseId.isEmpty()) { return false; }

  const QUrl sFinalrequest = EncodeForCorsProxy(c_sGetEOSScript,
                                QString("id=%1%2%3")
                                .arg(sTeaseId)
                                .arg(c_FIX_POLLUTION)
                                .arg("&cacheable&_nc=" + QString::number(iMinutes)));
  if (sFinalrequest.isEmpty()) { return false; }

  QPointer<CEosDownloadJob> pThis(this);
  QByteArray arr = Fetch(sFinalrequest);
  if (nullptr == pThis) { return false; }
  if (arr.isEmpty()) { return false; }

  jsonScript = QJsonDocument::fromJson(arr);
  return jsonScript.isObject();
}
