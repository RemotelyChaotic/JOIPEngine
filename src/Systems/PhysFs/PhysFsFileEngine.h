#ifndef PhysFsFileEngine_h
#define PhysFsFileEngine_h

#include <private/qabstractfileengine_p.h>
#include <QDateTime>
#include <QFileInfo>

struct PHYSFS_File;

class CPhysFsFileEngineIterator : public QAbstractFileEngineIterator
{
public:
  CPhysFsFileEngineIterator(const QString& sPath, QDir::Filters filters, const QStringList& vsNameFilters);
  ~CPhysFsFileEngineIterator() override;

  QString next() override;
  bool hasNext() const override;

  QString currentFileName() const override;
  QFileInfo currentFileInfo() const override;

private:
  void advance();
  QStringList vsEntries;
  mutable QFileInfo currentInfo;
  mutable QFileInfo nextInfo;
  mutable qint32 iCurent;
};

//----------------------------------------------------------------------------------------
//
class CPhysFsFileEngine : public QAbstractFileEngine
{
public:
  CPhysFsFileEngine(const QString& filename);
  ~CPhysFsFileEngine() override;

  bool open(QIODevice::OpenMode openMode) override;
  bool close() override;
  bool flush() override;
  qint64 size() const override;
  qint64 pos() const override;
  bool seek(qint64 pos) override;
  bool isSequential() const override;
  bool remove() override;
  bool mkdir(const QString& sDirName, bool bCreateParentDirectories) const override;
  bool rmdir(const QString& sDirName, bool bRecurseParentDirectories) const override;
  bool caseSensitive() const override;
  bool isRelativePath() const override;
  static QStringList entryListStatic(const QString& sPath, QDir::Filters filters, const QStringList& vsFilterNames);
  QStringList entryList(QDir::Filters filters, const QStringList& vsFilterNames) const override;
  FileFlags fileFlags(FileFlags type = allFileFlags) const override;
  QString fileName(FileName file = DefaultName) const override;
  QDateTime fileTime(FileTime time) const override;
  void setFileName(const QString& sFile) override;
  bool atEnd() const;

  typedef CPhysFsFileEngineIterator Iterator;
  Iterator *beginEntryList(QDir::Filters filters, const QStringList &filterNames) override;
  Iterator *endEntryList() override;

  qint64 read(char* data, qint64 iMaxlen) override;
  qint64 readLine(char* data, qint64 iMaxlen) override;
  qint64 write(const char* data, qint64 iLen) override;

  bool isOpened() const;

  QFile::FileError error() const;
  static QString errorString();

  bool supportsExtension(Extension extension) const override;

  static bool init(const QString& sPath);
  static bool deInit();
  static bool mount(const char* sNewDir, const char* sMountPoint, bool bAppendToPath = true);
  static bool unmount(const char* sOldDir);
  static bool setWriteDir(const char* sNewDir);

private:
  PHYSFS_File* m_pHandler;
  qint64 m_iSize;
  FileFlags m_flags;
  QString m_sFilename;
  QDateTime m_datetime;

  const static FileFlags allFileFlags;
};

//----------------------------------------------------------------------------------------
//
class CPhysFsFileEngineHandler : public QAbstractFileEngineHandler
{
public:
  QAbstractFileEngine* create(const QString &filename) const override;

  static QStringList SupportedFileTypes();
  static const QString c_sScheme;
};

#endif // PhysFsFileEngine_h
