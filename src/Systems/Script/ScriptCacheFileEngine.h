#ifndef CSCRIPTCACHEFILEENGINE_H
#define CSCRIPTCACHEFILEENGINE_H

#include <private/qabstractfileengine_p.h>

#include <QBuffer>
#include <QDateTime>
#include <QFileInfo>

class CScriptCacheFileEngine : public QAbstractFileEngine
{
public:
  CScriptCacheFileEngine(const QString& filename);
  ~CScriptCacheFileEngine() override;

  bool open(QIODevice::OpenMode openMode) override;
  bool close() override;
  qint64 size() const override;
  qint64 pos() const override;
  bool seek(qint64 pos) override;
  bool caseSensitive() const override;
  bool isRelativePath() const override;
  FileFlags fileFlags(FileFlags type = FileInfoAll) const override;
  QString fileName(FileName file = DefaultName) const override;
  QDateTime fileTime(FileTime time) const override;
  void setFileName(const QString& sFile) override;
  bool atEnd() const;

  qint64 read(char* data, qint64 iMaxlen) override;
  qint64 readLine(char* data, qint64 iMaxlen) override;

  bool supportsExtension(Extension extension) const override;

private:
  QByteArray m_sFileContent;
  QBuffer* m_pBuff;

  FileFlags m_flags;
  QString m_sFilename;
  QDateTime m_datetime;
};

//----------------------------------------------------------------------------------------
//
class CScriptCacheFileEngineHandler : public QAbstractFileEngineHandler
{
public:
  QAbstractFileEngine* create(const QString &filename) const override;

  static void ClearFiles(qint32 iProjId);
  static void RegisterFile(qint32 iProjId, const QString& sName, const QString& sContent);

  static const QString c_sScheme;
};

#endif // CSCRIPTCACHEFILEENGINE_H
