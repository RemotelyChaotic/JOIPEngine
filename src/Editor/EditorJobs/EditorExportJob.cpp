#include "EditorExportJob.h"
#include "Application.h"
#include "EditorJobTypes.h"

#include "Systems/DatabaseManager.h"
#include "Systems/PhysFs/PhysFsFileEngine.h"
#include "Systems/Project.h"

#include <QDebug>

namespace
{
  const char c_sTemporaryRccFileProperty[] = "RccFile";
}

CEditorExportJob::CEditorExportJob(QObject* pParent) :
    QObject(pParent),
    m_spExportProcess(nullptr),
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
bool CEditorExportJob::Run(const QVariantList& args)
{
  m_bHasError = false;
  m_bFinished = false;
  m_iProgress = 0;

  assert(2 == args.size());
  if (2 != args.size())
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
    m_spProject = spDbManager->FindProject(sProject);
    if (nullptr == m_spProject)
    {
      m_sError = QString("Could not find Project \"%1\".").arg(sProject);
      m_bHasError = true;
      m_bFinished = true;
      return false;
    }

    emit SignalStarted(m_iId);
    emit SignalProgressChanged(m_iId, Progress());

    const QString sName = PhysicalProjectName(m_spProject);
    const QString sFolder = CApplication::Instance()->Settings()->ContentFolder() + "/" + sName;

    QFile rccFile(CPhysFsFileEngineHandler::c_sScheme + "JOIPEngineExport.qrc");
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
    for (auto spResourcePair : m_spProject->m_spResourcesMap)
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

    CreateProcess();

    if (m_spExportProcess->state() == QProcess::ProcessState::NotRunning)
    {
      m_spExportProcess->setProperty(c_sTemporaryRccFileProperty, rccFile.fileName());
      m_spExportProcess->setWorkingDirectory(sFolder);
      m_spExportProcess->start("rcc",
                               QStringList() << "--binary" << "--no-compress" << "--verbose"
                                             << rccFile.fileName()
                                             << "--output" << (sFolder + "/" + sName + ".proj"));
    }
    else
    {
      m_sError = tr("Export is allready running.");
      m_bHasError = true;
      m_bFinished = true;
      return false;
    }

    m_bFinished = false;
    return true;
  }

  m_sError = QString("Database manager was destroyed.");
  m_bHasError = true;
  m_bFinished = true;
  return false;
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
  if (m_spExportProcess->state() != QProcess::ProcessState::NotRunning)
  {
    if (!m_spExportProcess->waitForFinished())
    {
      m_spExportProcess->kill();
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorExportJob::CreateProcess()
{
  m_spExportProcess.reset(new QProcess());

  connect(m_spExportProcess.get(), &QProcess::errorOccurred,
          this, &CEditorExportJob::SlotExportErrorOccurred);
  connect(m_spExportProcess.get(), qOverload<int,QProcess::ExitStatus>(&QProcess::finished),
          this, &CEditorExportJob::SlotExportFinished);
  connect(m_spExportProcess.get(), &QProcess::started,
          this, &CEditorExportJob::SlotExportStarted);
  connect(m_spExportProcess.get(), &QProcess::stateChanged,
          this, &CEditorExportJob::SlotExportStateChanged);
  connect(m_spExportProcess.get(), &QProcess::readyReadStandardError,
          this, &CEditorExportJob::SlotReadErrorOut);
  connect(m_spExportProcess.get(), &QProcess::readyReadStandardOutput,
          this, &CEditorExportJob::SlotReadStandardOut);
}

//----------------------------------------------------------------------------------------
//
void CEditorExportJob::SlotExportErrorOccurred(QProcess::ProcessError error)
{
  switch (error)
  {
    case QProcess::ProcessError::Crashed: m_sError = tr("Export process crashed."); break;
    case QProcess::ProcessError::Timedout: m_sError = tr("Export process timed out."); break;
    case QProcess::ProcessError::ReadError: m_sError = tr("Export process read error."); break;
    case QProcess::ProcessError::WriteError: m_sError = tr("Export process write error."); break;
    case QProcess::ProcessError::UnknownError: m_sError = tr("Unknown error in export process."); break;
    case QProcess::ProcessError::FailedToStart: m_sError = tr("Export process failed to start."); break;
    default: break;
  }

  m_bHasError = true;

  emit SignalJobMessage(m_iId, JobType(), m_sError);
  qWarning() << m_spExportProcess->errorString();
}

//----------------------------------------------------------------------------------------
//
void CEditorExportJob::SlotExportFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
  m_bFinished = true;

  if (exitStatus == QProcess::ExitStatus::CrashExit)
  {
    m_sError = tr("Export process crashed with code %1 (%2).")
                  .arg(exitCode).arg(m_spExportProcess->errorString());
    m_bHasError = true;
    emit SignalFinished(m_iId);
    return;
  }

  QFile rccFile(m_spExportProcess->property(c_sTemporaryRccFileProperty).toString());
  if (!rccFile.remove())
  {
    m_sError = tr("Could not remove temporary qrc file '%1'.")
                  .arg(rccFile.fileName());
    m_bHasError = true;
    emit SignalFinished(m_iId);
    return;
  }

  emit SignalFinished(m_iId);
}

//----------------------------------------------------------------------------------------
//
void CEditorExportJob::SlotExportStarted()
{
  m_iProgress = 1;
  emit SignalProgressChanged(m_iId, Progress());
}

//----------------------------------------------------------------------------------------
//
void CEditorExportJob::SlotExportStateChanged(QProcess::ProcessState newState)
{
  QString sMsg;
  switch (newState)
  {
    case QProcess::ProcessState::Running: sMsg = tr("Running export."); break;
    case QProcess::ProcessState::Starting: sMsg = tr("Starting export."); break;
    case QProcess::ProcessState::NotRunning: sMsg = tr("Export finished."); break;
    default: break;
  }
  if (!sMsg.isEmpty())
  {
    emit SignalJobMessage(m_iId, JobType(), sMsg);
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorExportJob::SlotReadErrorOut()
{
  QByteArray arr = m_spExportProcess->readAllStandardError();
  emit SignalJobMessage(m_iId, JobType(), QString::fromUtf8(arr));
}

//----------------------------------------------------------------------------------------
//
void CEditorExportJob::SlotReadStandardOut()
{
  QByteArray arr = m_spExportProcess->readAllStandardOutput();
  emit SignalJobMessage(m_iId, JobType(), QString::fromUtf8(arr));
}
