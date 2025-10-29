#include "ScriptCacheFileEngine.h"

const QString CScriptCacheFileEngineHandler::c_sScheme = "smemfile:/";

//----------------------------------------------------------------------------------------
//
namespace
{
  std::map<qint32, std::map<QString, QString>>& GetCache()
  {
    static std::map<qint32, std::map<QString, QString>> cache;
    return cache;
  }
}

//----------------------------------------------------------------------------------------
//
CScriptCacheFileEngine::CScriptCacheFileEngine(const QString& sFilename) :
  m_pBuff(nullptr),
  m_flags(static_cast<FileFlag>(0))
{
  setFileName(sFilename);
}
CScriptCacheFileEngine::~CScriptCacheFileEngine()
{
  close();
}

//----------------------------------------------------------------------------------------
//
bool CScriptCacheFileEngine::open(QIODevice::OpenMode openMode)
{
  close();

  if (openMode & QIODevice::ReadOnly)
  {
    QStringList vsElems = m_sFilename.split("/");
    if (vsElems.size() != 2)
    {
      return false;
    }

    bool bOk = false;
    qint32 iProjId = vsElems[0].toInt(&bOk);
    if (!bOk || -1 == iProjId)
    {
      return false;
    }

    auto it = GetCache().find(iProjId);
    if (GetCache().end() == it)
    {
      return false;
    }

    auto itScr = it->second.find(vsElems[1]);
    if (it->second.end() == itScr)
    {
      return false;
    }

    m_sFileContent = itScr->second.toUtf8();
    m_pBuff = new QBuffer(&m_sFileContent);

    return m_pBuff->open(QIODevice::ReadOnly);
  }
  return false;
}

//----------------------------------------------------------------------------------------
//
bool CScriptCacheFileEngine::close()
{
  if (nullptr != m_pBuff)
  {
    if (m_pBuff->isOpen())
    {
      m_pBuff->close();
    }
    delete m_pBuff;
    m_pBuff = nullptr;
  }
  m_sFileContent.clear();
  return true;
}

//----------------------------------------------------------------------------------------
//
qint64 CScriptCacheFileEngine::size() const
{
  return m_sFileContent.size();
}

//----------------------------------------------------------------------------------------
//
qint64 CScriptCacheFileEngine::pos() const
{
  return nullptr != m_pBuff ? m_pBuff->pos() : -1;
}

//----------------------------------------------------------------------------------------
//
bool CScriptCacheFileEngine::seek(qint64 pos)
{
  return nullptr != m_pBuff ? m_pBuff->seek(pos) : false;
}

//----------------------------------------------------------------------------------------
//
bool CScriptCacheFileEngine::caseSensitive() const
{
  return false;
}

//----------------------------------------------------------------------------------------
//
bool CScriptCacheFileEngine::isRelativePath() const
{
  return true;
}

//----------------------------------------------------------------------------------------
//
CScriptCacheFileEngine::FileFlags CScriptCacheFileEngine::fileFlags(FileFlags type) const
{
  return type & m_flags;
}

//----------------------------------------------------------------------------------------
//
QString CScriptCacheFileEngine::fileName(FileName file) const
{
  Q_UNUSED(file)
  return CScriptCacheFileEngineHandler::c_sScheme + m_sFilename;
}

//----------------------------------------------------------------------------------------
//
QDateTime CScriptCacheFileEngine::fileTime(FileTime time) const
{
  switch (time)
  {
    case CScriptCacheFileEngine::ModificationTime: // fallthrough
    default:
      return m_datetime;
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptCacheFileEngine::setFileName(const QString& sFile)
{
  QString sFileName = sFile;
  if (sFile.startsWith(CScriptCacheFileEngineHandler::c_sScheme))
  {
    sFileName = sFile.right(sFile.size() - CScriptCacheFileEngineHandler::c_sScheme.size());
  }

  m_sFilename = sFileName;
}

//----------------------------------------------------------------------------------------
//
bool CScriptCacheFileEngine::atEnd() const
{
  return nullptr != m_pBuff ? m_pBuff->atEnd() : true;
}

//----------------------------------------------------------------------------------------
//
qint64 CScriptCacheFileEngine::read(char* data, qint64 iMaxlen)
{
  return nullptr != m_pBuff ? m_pBuff->read(data, iMaxlen) : 0;
}

//----------------------------------------------------------------------------------------
//
qint64 CScriptCacheFileEngine::readLine(char* data, qint64 iMaxlen)
{
  return nullptr != m_pBuff ? m_pBuff->read(data, iMaxlen) : 0;
}

//----------------------------------------------------------------------------------------
//
bool CScriptCacheFileEngine::supportsExtension(Extension extension) const
{
  return extension == CScriptCacheFileEngine::AtEndExtension;
}

//----------------------------------------------------------------------------------------
//
QAbstractFileEngine* CScriptCacheFileEngineHandler::create(const QString &filename) const
{
  if (filename.startsWith(c_sScheme))
  {
    return new CScriptCacheFileEngine(filename);
  }
  // handling for QUrl(file:///smemfile:/...).toLocalFile() because sometimes we don't have a choice
  if (filename.startsWith("/" + c_sScheme))
  {
    return new CScriptCacheFileEngine(filename.mid(1));
  }

  return nullptr;
}

//----------------------------------------------------------------------------------------
//
void CScriptCacheFileEngineHandler::ClearFiles(qint32 iProjId)
{
  auto it = GetCache().find(iProjId);
  if (GetCache().end() != it)
  {
    it->second.clear();
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptCacheFileEngineHandler::RegisterFile(qint32 iProjId, const QString& sName,
                                                 const QString& sContent)
{
  GetCache()[iProjId][sName] = sContent;
}
