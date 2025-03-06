#include "PhysFsFileEngine.h"
#include <physfs.h>
#include <QDir>
#include <QDebug>
#include <QIODevice>
#include <QString>

const QString CPhysFsFileEngineHandler::c_sScheme = "pfs:/";
const CPhysFsFileEngine::FileFlags CPhysFsFileEngine::allFileFlags = FileFlags(FileFlag::FileInfoAll);


CPhysFsFileEngineIterator::CPhysFsFileEngineIterator(const QString& sPath, QDir::Filters filters, const QStringList& vsNameFilters) :
  QAbstractFileEngineIterator(filters, vsNameFilters),
  vsEntries(CPhysFsFileEngine::entryListStatic(sPath, filters, vsNameFilters)),
  iCurent(0)
{
  nextInfo = QFileInfo(vsEntries[iCurent]);
}
CPhysFsFileEngineIterator::~CPhysFsFileEngineIterator()
{
}

QString CPhysFsFileEngineIterator::next()
{
  if (!hasNext())
  {
    return QString();
  }

  advance();
  return currentFilePath();
}

bool CPhysFsFileEngineIterator::hasNext() const
{
  return iCurent < vsEntries.size();
}

QString CPhysFsFileEngineIterator::currentFileName() const
{
  return currentInfo.fileName();
}

QFileInfo CPhysFsFileEngineIterator::currentFileInfo() const
{
  return currentInfo;
}

void CPhysFsFileEngineIterator::advance()
{
  currentInfo = nextInfo;
  if (hasNext())
  {
    ++iCurent;
    if (vsEntries.size() > iCurent)
    {
      nextInfo = QFileInfo(vsEntries[iCurent]);
    }
    else
    {
      nextInfo = QFileInfo();
    }
  }
}

//----------------------------------------------------------------------------------------
//
CPhysFsFileEngine::CPhysFsFileEngine(const QString& sFilename) :
  m_pHandler(nullptr),
  m_flags(static_cast<FileFlag>(0))
{
  setFileName(sFilename);
}

CPhysFsFileEngine::~CPhysFsFileEngine()
{
  close();
}

bool CPhysFsFileEngine::open(QIODevice::OpenMode openMode)
{
  close();

  if (openMode & QIODevice::WriteOnly)
  {
    m_pHandler = PHYSFS_openWrite(m_sFilename.toUtf8().constData());
    m_flags = QAbstractFileEngine::WriteOwnerPerm | QAbstractFileEngine::WriteUserPerm | QAbstractFileEngine::FileType;
  }

  else if (openMode & QIODevice::ReadOnly)
  {
    m_pHandler = PHYSFS_openRead(m_sFilename.toUtf8().constData());
  }

  else if (openMode & QIODevice::Append)
  {
    m_pHandler = PHYSFS_openAppend(m_sFilename.toUtf8().constData());
  }

  else
  {
    qWarning() << QString(QT_TR_NOOP("Bad file open mode: %1")).arg(openMode);
  }

  if (!m_pHandler)
  {
    qWarning() << QString(QT_TR_NOOP("Failed to open %1, reason: %2"))
      .arg(m_sFilename.toUtf8().constData(), PHYSFS_getLastError());
    return false;
  }

  return true;
}

bool CPhysFsFileEngine::close()
{
  if (isOpened())
  {
    int iResult = PHYSFS_close(m_pHandler);
    m_pHandler = nullptr;
    return iResult != 0;
  }

  return true;
}

bool CPhysFsFileEngine::flush()
{
  return PHYSFS_flush(m_pHandler) != 0;
}

qint64 CPhysFsFileEngine::size() const
{
  return m_iSize;
}

qint64 CPhysFsFileEngine::pos() const
{
  return PHYSFS_tell(m_pHandler);
}

bool CPhysFsFileEngine::seek(qint64 iPos)
{
  return PHYSFS_seek(m_pHandler, static_cast<size_t>(iPos)) != 0;
}

bool CPhysFsFileEngine::isSequential() const
{
  return false;
}

bool CPhysFsFileEngine::remove()
{
  return PHYSFS_delete(m_sFilename.toUtf8().constData()) != 0;
}

bool CPhysFsFileEngine::mkdir(const QString& sDirName, bool bCreateParentDirectories) const
{
  Q_UNUSED(bCreateParentDirectories)
  return PHYSFS_mkdir(sDirName.toUtf8().constData()) != 0;
}

bool CPhysFsFileEngine::rmdir(const QString& sDirName, bool sRecurseParentDirectories) const
{
  Q_UNUSED(sRecurseParentDirectories)
  return PHYSFS_delete(sDirName.toUtf8().constData()) != 0;
}

bool CPhysFsFileEngine::caseSensitive() const
{
  return true;
}

bool CPhysFsFileEngine::isRelativePath() const
{
  return true;
}

QStringList CPhysFsFileEngine::entryListStatic(const QString& sPath, QDir::Filters filters, const QStringList& vsFilterNames)
{
  // TODO: support QDir filters
  Q_UNUSED(filters)

  QString file;
  QStringList result;
  QByteArray baPath = sPath.toLocal8Bit();
  char **files = PHYSFS_enumerateFiles(baPath.constData());

  PHYSFS_Stat stat;
  for (char **i = files; *i != nullptr; i++)
  {
    file = QString::fromUtf8(*i);
    if (QDir::match(vsFilterNames, file))
    {
      if (filters.testFlag(QDir::Filter::NoSymLinks) && PHYSFS_isSymbolicLink(*i)) { continue; }
      if (filters.testFlag(QDir::Filter::Dirs) && !PHYSFS_isDirectory(*i)) { continue; }
      if (filters.testFlag(QDir::Filter::Files) && PHYSFS_isDirectory(*i)) { continue; }

      PHYSFS_stat(*i, &stat);
      if (filters.testFlag(QDir::Filter::Writable) && stat.readonly) { continue; }

      result << CPhysFsFileEngineHandler::c_sScheme + sPath + "/" + file;
    }
  }

  PHYSFS_freeList(files);

  // Dot and DotDot are not supported by PhysFs
  //if (filters.testFlag(QDir::Filter::NoDot)) { result.removeAll("."); }
  //if (filters.testFlag(QDir::Filter::NoDotDot)) { result.removeAll(".."); }
  return result;
}

QStringList CPhysFsFileEngine::entryList(QDir::Filters filters, const QStringList& vsFilterNames) const
{
  return entryListStatic(m_sFilename, filters, vsFilterNames);
}

CPhysFsFileEngine::FileFlags CPhysFsFileEngine::fileFlags(FileFlags type) const
{
  return m_flags & type;
}

QString CPhysFsFileEngine::fileName(FileName file) const
{
  if (file == CPhysFsFileEngine::AbsolutePathName)
  {
    return PHYSFS_getWriteDir();
  }

  return CPhysFsFileEngineHandler::c_sScheme + m_sFilename;
}

QDateTime CPhysFsFileEngine::fileTime(FileTime time) const
{
  switch (time)
  {
    case CPhysFsFileEngine::ModificationTime: // fallthrough
    default:
        return m_datetime;
  }
}

void CPhysFsFileEngine::setFileName(const QString& sFile)
{
  QString sFileName = sFile;
  if (sFile.startsWith(CPhysFsFileEngineHandler::c_sScheme))
  {
    sFileName = "/" + sFile.right(sFile.size() - CPhysFsFileEngineHandler::c_sScheme.size());
  }

  m_sFilename = sFileName;
  PHYSFS_Stat stat;
  if (PHYSFS_stat(m_sFilename.toUtf8().constData(), &stat) != 0)
  {
    m_iSize = stat.filesize;
    m_datetime = QDateTime::fromTime_t(static_cast<quint32>(stat.modtime));
    m_flags |= CPhysFsFileEngine::ExistsFlag;

    switch (stat.filetype)
    {
      case PHYSFS_FILETYPE_REGULAR:
          m_flags |= CPhysFsFileEngine::FileType;
          break;

      case PHYSFS_FILETYPE_DIRECTORY:
          m_flags |= CPhysFsFileEngine::DirectoryType;
          break;

      case PHYSFS_FILETYPE_SYMLINK:
          m_flags |= CPhysFsFileEngine::LinkType;
          break;

      case PHYSFS_FILETYPE_OTHER: // fallthrough
      default: qWarning() << QString("Unsupported PhysFs filetype detected: %1").arg(stat.filetype);
    }
  }
}

bool CPhysFsFileEngine::atEnd() const
{
  return PHYSFS_eof(m_pHandler) != 0;
}

CPhysFsFileEngine::Iterator* CPhysFsFileEngine::beginEntryList(QDir::Filters filters,
                                                               const QStringList &filterNames)
{
  return new CPhysFsFileEngine::Iterator(m_sFilename, filters, filterNames);
}

CPhysFsFileEngine::Iterator*CPhysFsFileEngine::endEntryList()
{
  return nullptr;
}

qint64 CPhysFsFileEngine::read(char *data, qint64 iMaxlen)
{
  return PHYSFS_read(m_pHandler, data, 1, static_cast<quint32>(iMaxlen));
}

qint64 CPhysFsFileEngine::readLine(char *data, qint64 iMaxlen)
{
  // Not supported, so wse just read as normal
  return read(data, iMaxlen);
}

qint64 CPhysFsFileEngine::write(const char *data, qint64 iLen)
{
  return PHYSFS_write(m_pHandler, data, 1, static_cast<quint32>(iLen));
}

bool CPhysFsFileEngine::isOpened() const
{
  return m_pHandler != nullptr;
}

QFile::FileError CPhysFsFileEngine::error() const
{
  switch(PHYSFS_getLastErrorCode())
  {
    case PHYSFS_ErrorCode::PHYSFS_ERR_OK: return QFile::NoError;
    case PHYSFS_ErrorCode::PHYSFS_ERR_NO_WRITE_DIR: return QFile::WriteError;
    case PHYSFS_ErrorCode::PHYSFS_ERR_OUT_OF_MEMORY: // fallthrough
    case PHYSFS_ErrorCode::PHYSFS_ERR_NOT_INITIALIZED: // fallthrough
    case PHYSFS_ErrorCode::PHYSFS_ERR_NO_SPACE: // fallthrough
    case PHYSFS_ErrorCode::PHYSFS_ERR_CORRUPT: // fallthrough
    case PHYSFS_ErrorCode::PHYSFS_ERR_OS_ERROR: return QFile::FatalError;
    case PHYSFS_ErrorCode::PHYSFS_ERR_NOT_MOUNTED: return QFile::ResourceError;
    case PHYSFS_ErrorCode::PHYSFS_ERR_FILES_STILL_OPEN: return QFile::OpenError;
    case PHYSFS_ErrorCode::PHYSFS_ERR_BAD_FILENAME: return QFile::AbortError;
    case PHYSFS_ErrorCode::PHYSFS_ERR_UNSUPPORTED: // fallthrough
    case PHYSFS_ErrorCode::PHYSFS_ERR_OTHER_ERROR: return QFile::UnspecifiedError;
    case PHYSFS_ErrorCode::PHYSFS_ERR_PAST_EOF: return QFile::PositionError;
    case PHYSFS_ErrorCode::PHYSFS_ERR_PERMISSION: return QFile::PermissionsError;
    default: return QFile::UnspecifiedError;
  }
}

QString CPhysFsFileEngine::errorString()
{
  return PHYSFS_getLastError();
}

bool CPhysFsFileEngine::supportsExtension(Extension extension) const
{
  return extension == CPhysFsFileEngine::AtEndExtension;
}

bool CPhysFsFileEngine::init(const QString& sPath)
{
  bool bRetVal = PHYSFS_init(sPath.toStdString().data()) != 0;
  if (!bRetVal)
  {
    qCritical() << QString(QT_TR_NOOP("Could not init PhysFs in: %1")).arg(sPath);
  }
  return bRetVal;
}

bool CPhysFsFileEngine::deInit()
{
  return PHYSFS_deinit() != 0;
}

bool CPhysFsFileEngine::mount(const char* sNewDir, const char* sMountPoint, bool bAppendToPath)
{
  return PHYSFS_mount(sNewDir, sMountPoint, bAppendToPath) != 0;
}

bool CPhysFsFileEngine::unmount(const char* sOldDir)
{
  return PHYSFS_unmount(sOldDir) != 0;
}

bool CPhysFsFileEngine::setWriteDir(const char* sNewDir)
{
  return PHYSFS_setWriteDir(sNewDir) != 0;
}

//----------------------------------------------------------------------------------------
//
QAbstractFileEngine* CPhysFsFileEngineHandler::create(const QString &filename) const
{
  if (filename.startsWith(c_sScheme))
  {
    return new CPhysFsFileEngine(filename);
  }
  // handling for QUrl(file:///pfs:/...).toLocalFile() because sometimes we don't have a choice
  if (filename.startsWith("/" + c_sScheme))
  {
    return new CPhysFsFileEngine(filename.mid(1));
  }

  return nullptr;
}

QStringList CPhysFsFileEngineHandler::SupportedFileTypes()
{
  static QStringList vsSupportedTypes;
  if (vsSupportedTypes.isEmpty())
  {
    const PHYSFS_ArchiveInfo **i;
    for (i = PHYSFS_supportedArchiveTypes(); *i != NULL; i++)
    {
      vsSupportedTypes << QString::fromUtf8((*i)->extension);
    }
  }
  return vsSupportedTypes;
}
