#include "Unzipper.h"

#undef VERSION

#include <zip.h>
#include <zipint.h>

#undef close

#include <QApplication>
#include <QDir>
#include <QObject>
#include <QFileInfo>

#include <future>
#include <thread>

//----------------------------------------------------------------------------------------
//
namespace
{
  bool CreatePath(const QString& sDir, const QString& sPath)
  {
    QDir dir(sDir);
    return dir.mkpath(sPath);
  }

  bool CreateFile(const QString& sDir, const QString& sFile, const QByteArray& data)
  {
    QFile file(sDir + "/" + sFile);
    if (!file.open(QIODevice::ReadWrite | QIODevice::Truncate))
    {
      return false;
    }

    file.write(data);
    return true;
  }
}

//----------------------------------------------------------------------------------------
//
namespace zipper
{
  bool Unzip(const QString& sPathSource,
             std::function<void(const QString&)> fnMsg,
             std::function<void(qint32,qint32)> fnProgress,
             QString* psErr)
  {
    std::packaged_task<bool()> task([sPathSource, fnMsg, fnProgress, psErr]() -> bool
    {
      QFileInfo sourceInfo(sPathSource);
      if (!sourceInfo.exists())
      {
        *psErr = QObject::tr("Zip file does not exist").arg(sPathSource);
        return false;
      }

      const QString sFolder = sourceInfo.absolutePath();

      int iErr;
      zip_t* pArch = zip_open(QDir::fromNativeSeparators(sPathSource).toStdString().c_str(),
                              ZIP_RDONLY, &iErr);
      if (nullptr == pArch)
      {
        zip_error_t error;
        zip_error_init_with_code(&error, iErr);
        *psErr = zip_error_strerror(&error);
        zip_error_fini(&error);
        return false;
      }

      zip_stat_t sb;
      qint64 iNumEntries = zip_get_num_entries(pArch, 0);
      for (qint64 i = 0; i < iNumEntries; i++)
      {
        fnMsg(QObject::tr("Extracting %1/%2...").arg(i).arg(iNumEntries));
        fnProgress(i, iNumEntries);

        if (zip_stat_index(pArch, i, 0, &sb) == 0)
        {
          const QString sFile(sb.name);
          if (sFile.back() == '/')
          {
            if (!CreatePath(sFolder, sFile))
            {
              *psErr = QObject::tr("Could not create %1").arg(sFile);
              zip_close(pArch);
              return false;
            }
          }
          else
          {
            zip_file_t* pZf = zip_fopen_index(pArch, i, 0);
            if (nullptr == pZf)
            {
              *psErr = QObject::tr("Could not open compressed file %1").arg(sFile);
              zip_close(pArch);
              return false;
            }

            QByteArray arr(sb.size, '\0');
            qint64 iRead = zip_fread(pZf, arr.data(), sb.size);
            if (iRead != sb.size)
            {
              *psErr = QObject::tr("Could not read compressed file %1").arg(sFile);
              zip_fclose(pZf);
              zip_close(pArch);
              return false;
            }

            zip_fclose(pZf);

            if (!CreateFile(sFolder, sFile, arr))
            {
              *psErr = QObject::tr("Could not create %1").arg(sFile);
              zip_close(pArch);
              return false;
            }
          }
        }
        else
        {
          *psErr = QObject::tr("Internal error: zip_stat_index");
          zip_close(pArch);
          return false;
        }
      }

      if (zip_close(pArch) == -1)
      {
        *psErr = QObject::tr("Can't close zip archive");
        return false;
      }

      return true;
    });

    std::future<bool> future = task.get_future();
    std::thread t(std::move(task));

    while (future.wait_for(std::chrono::seconds(0)) != std::future_status::ready)
    {
      qApp->processEvents();
    }

    t.join();
    return future.get();
  }
}
