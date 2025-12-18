#include "EditorImageCompressionJob.h"
#include "Application.h"

#include "Systems/DatabaseManager.h"
#include "Systems/Database/Project.h"
#include "Systems/PhysFs/PhysFsFileEngine.h"
#include "Systems/Database/Resource.h"

#include "Utils/RaiiFunctionCaller.h"

#include <QImageReader>

CEditorImageCompressionJob::CEditorImageCompressionJob(QObject* pParent) :
  IEditorJob(pParent),
  m_spProject(nullptr),
  m_iProgress(0),
  m_sName("Export"),
  m_sError("No error."),
  m_bFinished(false)
{
}

CEditorImageCompressionJob::~CEditorImageCompressionJob()
{
}

//----------------------------------------------------------------------------------------
//
QString CEditorImageCompressionJob::Error() const
{
  return m_sError;
}

//----------------------------------------------------------------------------------------
//
bool CEditorImageCompressionJob::Finished() const
{
  return m_bFinished;
}

//----------------------------------------------------------------------------------------
//
bool CEditorImageCompressionJob::HasError() const
{
  return m_bHasError;
}

//----------------------------------------------------------------------------------------
//
qint32 CEditorImageCompressionJob::Id() const
{
  return m_iId;
}

//----------------------------------------------------------------------------------------
//
QString CEditorImageCompressionJob::JobName() const
{
  return m_sName;
}

//----------------------------------------------------------------------------------------
//
QString CEditorImageCompressionJob::JobType() const
{
  return editor_job::c_sCompress;
}

//----------------------------------------------------------------------------------------
//
qint32 CEditorImageCompressionJob::Progress() const
{
  return m_iProgress;
}

//----------------------------------------------------------------------------------------
//
QString CEditorImageCompressionJob::ReturnValue() const
{
  return m_sReturnValue;
}

//----------------------------------------------------------------------------------------
//
#include <thread>
bool CEditorImageCompressionJob::Run(const QVariantList& args)
{
  m_bHasError = false;
  m_bFinished = false;
  m_iProgress = 0;

  CRaiiFunctionCaller finishedCaller([this](){
    m_bFinished = !WasStopped();
  });

  assert(3 == args.size());
  if (3 != args.size())
  {
    m_sError = QString("1 argument was expected, got %1.").arg(args.size());
    m_bHasError = true;
    return false;
  }

  if (auto spDbManager = CApplication::Instance()->System<CDatabaseManager>().lock())
  {
    m_iId = args[0].toInt();
    const QString sProject = args[1].toString();
    qint32 iCompression = args[2].toInt();

    emit SignalStarted(m_iId);

    m_spProject = spDbManager->FindProject(sProject);
    if (nullptr == m_spProject)
    {
      m_sError = QString("Could not find Project \"%1\".").arg(sProject);
      m_bHasError = true;
      return false;
    }

    emit SignalProgressChanged(m_iId, Progress());

    // collect the resources to convert
    std::vector<tspResource> vspResourcesToConvert;
    {
      const QString sGifFormat = "gif";
      const QString sSvgFormat = "svg";
      const QStringList vsCompressedFormats = QStringList() << "jpeg" << "jpg" << "jpe" << "jfif";

      QReadLocker projLocker(&m_spProject->m_rwLock);
      if (!m_spProject->m_bBundled)
      {
        for (const auto& [sName, spResource] : m_spProject->m_baseData.m_spResourcesMap)
        {
          Q_UNUSED(sName)
          QReadLocker resLocker(&spResource->m_rwLock);
          if (spResource->m_sPath.IsLocalFile() && spResource->m_sResourceBundle.isEmpty() &&
              EResourceType::eImage == spResource->m_type._to_integral())
          {
            const QString sSourcePath = spResource->PhysicalResourcePath();
            QImageReader reader(sSourcePath);
            if (reader.canRead())
            {
              const QString sFormat = QString::fromUtf8(QImageReader::imageFormat(sSourcePath));
              if ((!reader.supportsAnimation() || 1 == reader.imageCount()) &&
                  sSvgFormat != sFormat &&
                  sGifFormat != sFormat &&
                  !vsCompressedFormats.contains(sFormat))
              {
                vspResourcesToConvert.push_back(spResource);
              }
            }
          }
        }
      }
    }

    // convert all images
    qint32 iResourceCount = static_cast<qint32>(vspResourcesToConvert.size());
    qint32 iResourceNr = 0;
    for (const tspResource& spResource : vspResourcesToConvert)
    {
      QWriteLocker resLocker(&spResource->m_rwLock);
      emit SignalJobMessage(m_iId, JobType(), spResource->m_sName);

      QString sSourcePath = spResource->PhysicalResourcePath();

      // qimage can not handle PhysFS paths because reasons
      QString sDestPathImage = sSourcePath + ".jpeg";
      QImage img(sSourcePath);
      bool bOk = img.save(sDestPathImage, nullptr, iCompression);
      if (bOk)
      {
        spResource->m_sPath = joip_resource::CreatePathFromAbsolutePath(sDestPathImage, m_spProject);
        QFile oldImg(sSourcePath);
        if (!oldImg.remove())
        {
          QString sMsg = tr("Could not remove old image.\n%1").arg(oldImg.errorString());
          emit SignalJobMessage(m_iId, JobType(), sMsg);
        }
      }
      else
      {
        QString sMsg = tr("Could not convert image to jpeg.");
        emit SignalJobMessage(m_iId, JobType(), sMsg);
      }

      m_iProgress = iResourceNr * 100 / iResourceCount;
      emit SignalProgressChanged(m_iId, Progress());
      emit SignalJobMessage(m_iId, JobType(), spResource->m_sName);
      iResourceNr++;

      if (WasStopped()) { break; }
    }

    return true;
  }

  m_sError = QString("Database manager was destroyed.");
  m_bHasError = true;
  return false;
}

//----------------------------------------------------------------------------------------
//
void CEditorImageCompressionJob::Stop()
{
  IRunnableJob::Stop();
  AbortImpl();
}

//----------------------------------------------------------------------------------------
//
void CEditorImageCompressionJob::AbortImpl()
{

}
