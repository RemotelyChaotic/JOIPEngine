#include "EditorExportJob.h"
#include "Application.h"

#include "Systems/Database/ResourceBundle.h"
#include "Systems/DatabaseManager.h"
#include "Systems/PhysFs/PhysFsFileEngine.h"
#include "Systems/Database/Project.h"

#include <rcc.h>
#include <rcc_options.h>
#include <rcc_runner.h>

#undef VERSION

#include <zip.h>
#include <zipint.h>

#undef close

#include <QDebug>
#include <QDir>

#include <iostream>
#include <fstream>

namespace
{
  const char c_sTempRCCFileName[] = "JOIPEngineExport.qrc";
  const char c_sTemporaryRccFileProperty[] = "RccFile";
  const char c_sTemporaryOutFileProperty[] = "OutFile";
}

CEditorExportJob::CEditorExportJob(QObject* pParent) :
    IEditorJob(pParent),
    m_spProject(nullptr),
    m_iProgress(0),
    m_sName("Export"),
    m_sError("No error."),
    m_bFinished(false)
{
  qRegisterMetaType<QProcess::ProcessState>();
  qRegisterMetaType<QProcess::ExitStatus>();
}
CEditorExportJob::~CEditorExportJob()
{
  Stop();
}

//----------------------------------------------------------------------------------------
//
QString CEditorExportJob::Error() const
{
  return m_sError;
}

//----------------------------------------------------------------------------------------
//
bool CEditorExportJob::Finished() const
{
  return m_bFinished;
}

//----------------------------------------------------------------------------------------
//
bool CEditorExportJob::HasError() const
{
  return m_bHasError;
}

//----------------------------------------------------------------------------------------
//
qint32 CEditorExportJob::Id() const
{
  return m_iId;
}

//----------------------------------------------------------------------------------------
//
QString CEditorExportJob::JobName() const
{
  return m_sName;
}

//----------------------------------------------------------------------------------------
//
QString CEditorExportJob::JobType() const
{
  return editor_job::c_sExport;
}

//----------------------------------------------------------------------------------------
//
qint32 CEditorExportJob::Progress() const
{
  return m_iProgress;
}

//----------------------------------------------------------------------------------------
//
QString CEditorExportJob::ReturnValue() const
{
  return m_sReturnValue;
}

//----------------------------------------------------------------------------------------
//
bool CEditorExportJob::Run(const QVariantList& args)
{
  m_bHasError = false;
  m_bFinished = false;
  m_iProgress = 0;

  assert(3 == args.size());
  if (3 != args.size())
  {
    m_sError = QString("1 argument was expected, got %1.").arg(args.size());
    m_bHasError = true;
    m_bFinished = true;
    return false;
  }

  if (auto spDbManager = CApplication::Instance()->System<CDatabaseManager>().lock())
  {
    m_iId = args[0].toInt();
    const QString sProject = args[1].toString();
    EExportFormat format = static_cast<EExportFormat>(args[2].toInt());

    emit SignalStarted(m_iId);

    m_spProject = spDbManager->FindProject(sProject);
    if (nullptr == m_spProject)
    {
      m_sError = QString("Could not find Project \"%1\".").arg(sProject);
      m_bHasError = true;
      m_bFinished = true;
      return false;
    }

    emit SignalProgressChanged(m_iId, Progress());

    const QString sName = PhysicalProjectName(m_spProject);
    const QString sFolder = CApplication::Instance()->Settings()->ContentFolder() + "/" + sName;

    switch (format)
    {
      case EExportFormat::eArchive:
        return RunZipExport(sName, sFolder);
      case EExportFormat::eBinary:
        return RunBinaryExport(sName, sFolder);
    }

    m_sError = QString("Invalid Export option: %1.").arg(static_cast<qint32>(format));
    m_bHasError = true;
    m_bFinished = true;
    return false;
  }

  m_sError = QString("Database manager was destroyed.");
  m_bHasError = true;
  m_bFinished = true;
  return false;
}

//----------------------------------------------------------------------------------------
//
bool CEditorExportJob::RunBinaryExport(const QString& sName, const QString& sFolder)
{
  emit SignalJobMessage(m_iId, JobType(), tr("Collecting Resources."));

  QFile rccFile(CPhysFsFileEngineHandler::c_sScheme + c_sTempRCCFileName);
  if (!rccFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
  {
    m_sError =  tr("Could not write temporary resource file '%1':\n%2")
                   .arg(rccFile.fileName()).arg(rccFile.errorString());
    m_bHasError = true;
    m_bFinished = true;
    return false;
  }

  QTextStream outStream(&rccFile);
  outStream.setCodec("UTF-8");
  outStream << "<!DOCTYPE RCC><RCC version=\"1.0\">" << "<qresource prefix=\"/\">";
  m_spProject->m_rwLock.lockForRead();
  for (auto spResourcePair : m_spProject->m_baseData.m_spResourcesMap)
  {
    const QString sOutPath = ResourceUrlToAbsolutePath(spResourcePair.second);
    spResourcePair.second->m_rwLock.lockForRead();
    if (IsLocalFile(spResourcePair.second->m_sPath))
    {
      outStream << QString("<file alias=\"%2\">%1</file>")
                       .arg(sOutPath)
                       .arg(spResourcePair.second->m_sName);
    }
    spResourcePair.second->m_rwLock.unlock();
  }
  m_spProject->m_rwLock.unlock();
  outStream << QString("<file alias=\"%2\">%1</file>")
                   .arg(joip_resource::c_sProjectFileName)
                   .arg(joip_resource::c_sProjectFileName);
  outStream << "</qresource>" << "</RCC>";

  rccFile.close();

  QString sRccFileName = (sFolder + "/" + c_sTempRCCFileName);
  QString sOutFileName = (sFolder + "/" + sName + ".proj");
  QRcc::SRCCOptions opts {
      sOutFileName,
      std::nullopt,
      std::nullopt,
      std::nullopt,
      std::nullopt,
      std::nullopt,
      true,
      std::nullopt,
      std::nullopt,
      false, true, false, false,
      std::nullopt,
      false,
      std::nullopt,
      QStringList() << sRccFileName
  };

  emit SignalJobMessage(m_iId, JobType(), tr("Packing into: %1").arg(sOutFileName));

  qint32 iRet = QRcc::runRcc(opts);
  if (0 != iRet)
  {
    m_sError = tr("Could not write project file %1:\n%2")
                   .arg(sOutFileName).arg("");
    m_bHasError = true;
    m_bFinished = true;
    return false;
  }

  if (!rccFile.remove())
  {
    m_sError += tr("Could not remove temporary qrc file '%1'.")
                    .arg(rccFile.fileName());
    m_bHasError = true;
    m_bFinished = true;
    return false;
  }

  if (!QFile(sOutFileName).exists())
  {
    m_sError += tr("Could not create exported project File.");
    m_bHasError = true;
    m_bFinished = true;
    return false;
  }

  m_sReturnValue = QString("Exported to: %2").arg(sOutFileName);
  m_bHasError = false;
  m_bFinished = true;
  return true;
}

//----------------------------------------------------------------------------------------
//
bool CEditorExportJob::RunZipExport(const QString& sName, const QString& sFolder)
{
  const QString sZipFile = sFolder + QDir::separator() + sName + ".zip";

  int iErr;
  zip_t* pOutArch = zip_open(QDir::fromNativeSeparators(sZipFile).toStdString().c_str(),
                            ZIP_CREATE | ZIP_TRUNCATE, &iErr);
  if (nullptr == pOutArch)
  {
    zip_error_t error;
    zip_error_init_with_code(&error, iErr);
    m_sError = tr("Could not create Archive %1:\n%2")
        .arg(sZipFile).arg(zip_error_strerror(&error));
    m_bHasError = true;
    m_bFinished = true;
    zip_error_fini(&error);
    return false;
  }

  emit SignalJobMessage(m_iId, JobType(), tr("Collecting Resources."));

  // first collect all resources
  std::vector<SExportFile> vArchives;
  std::vector<SExportFile> vFiles;
  {
    QReadLocker projLocker(&m_spProject->m_rwLock);
    for (const auto& [sName, spResource] : m_spProject->m_baseData.m_spResourcesMap)
    {
      QReadLocker resLocker(&spResource->m_rwLock);
      // only include local files
      if (IsLocalFile(spResource->m_sPath))
      {
        if (spResource->m_sResourceBundle.isEmpty())
        {
          auto it = std::find_if(vFiles.begin(), vFiles.end(),
                                 [spR = spResource](const SExportFile& file) {
            return file.m_sName == spR->m_sName;
          });
          if (vFiles.end() == it)
          {
            QString sPath = PhysicalResourcePath(spResource);
            vFiles.push_back({sName, sPath});
          }
        }
        else
        {
          auto spBundle = m_spProject->m_baseData.m_spResourceBundleMap[spResource->m_sResourceBundle];
          QReadLocker bundleLocker(&spBundle->m_rwLock);
          auto it = std::find_if(vArchives.begin(), vArchives.end(),
                                 [&spBundle](const SExportFile& file) {
            return file.m_sName == spBundle->m_sName;
          });
          if (vArchives.end() == it && IsLocalFile(spBundle->m_sPath))
          {
            QUrl urlCopy(spBundle->m_sPath);
            urlCopy.setScheme(QString());
            QString sBasePath = PhysicalProjectPath(spBundle->m_spParent);
            vArchives.push_back({spBundle->m_sName,
                                 sBasePath + "/" + QUrl().resolved(urlCopy).toString()});
          }
        }
      }
    }

    vFiles.push_back({joip_resource::c_sProjectFileName,
                      sFolder + QDir::separator() + joip_resource::c_sProjectFileName});
  }

  // now write all files into archive
  std::vector<SExportFile> vTotal;
  vTotal.reserve(vArchives.size() + vFiles.size());
  vTotal.insert(vTotal.begin(), vArchives.begin(), vArchives.end());
  vTotal.insert(vTotal.begin(), vFiles.begin(), vFiles.end());

  size_t iTotal = vTotal.size();
  size_t iCounter = 0;
  for (const SExportFile& file : vTotal)
  {
    emit SignalJobMessage(m_iId, JobType(), tr("Archiving: %1").arg(file.m_sName));

    QString sFilePath = QDir::fromNativeSeparators(file.m_sPath);
    zip_source_t* pSource = zip_source_file(pOutArch,
                                            sFilePath.toStdString().c_str(),
                                            0, -1);
    if (nullptr != pSource)
    {
      QString sFileInArch = sFilePath.replace(sFolder + "/", "");
      zip_int64_t iFile =
        zip_file_add(pOutArch,
                     sFileInArch.toStdString().c_str(), pSource,
                     ZIP_FL_OVERWRITE | ZIP_FL_ENC_UTF_8);
      if (0 > iFile)
      {
        m_sError = tr("Could add file to Archive %1:\n%2")
            .arg(sZipFile).arg(zip_error_strerror(&pOutArch->error));
        m_bHasError = true;
        m_bFinished = true;

        zip_discard(pOutArch);

        return false;
      }
    }
    else
    {
      m_sError = tr("Could not read source file %1:\n%2")
          .arg(sFilePath).arg(zip_error_strerror(&pOutArch->error));
      m_bHasError = true;
      m_bFinished = true;

      zip_discard(pOutArch);

      return false;
    }

    m_iProgress = static_cast<qint32>(iCounter * 100 / iTotal);
    emit SignalProgressChanged(m_iId, Progress());
  }

  // try to write archive
  emit SignalJobMessage(m_iId, JobType(), tr("Writing archive."));
  iErr = zip_close(pOutArch);
  if (0 > iErr)
  {
    m_sError = tr("Could not create Archive %1:\n%2")
        .arg(sZipFile).arg(zip_error_strerror(&pOutArch->error));
    m_bHasError = true;
    m_bFinished = true;

    zip_discard(pOutArch);

    return false;
  }

  m_sReturnValue = QString("Exported to: %2").arg(sZipFile);
  m_bFinished = true;
  return true;
}

//----------------------------------------------------------------------------------------
//
void CEditorExportJob::Stop()
{
  IRunnableJob::Stop();
  AbortImpl();
}

//----------------------------------------------------------------------------------------
//
void CEditorExportJob::AbortImpl()
{
}
