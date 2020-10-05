#include "PhysFsFileEngine.h"
#include "PhysFsQtAVIntegration.h"
#include <QtAV/MediaIO.h>
#include <QtAV/private/factory.h>
#include <QtAV/private/MediaIO_p.h>
#include <QtAV/private/mkid.h>
#include <QDebug>
#include <QFile>
#include <QIODevice>

using namespace QtAV;

static const char* c_sSchemeBase = "pfs";

/******************************************************************************
  Based on the private QtAV-QFile implementation see QtAV\src\io\QIODeviceIO.cpp
******************************************************************************/
class QFileIOPhysFsPrivate : public MediaIOPrivate
{
public:
    QFileIOPhysFsPrivate()
        : MediaIOPrivate()
        , dev(0)
    {}
    ~QFileIOPhysFsPrivate() {
        if (file.isOpen())
            file.close();
    }

    QIODevice *dev;
    QFile file;
};

//----------------------------------------------------------------------------------------
//
static const char kQFileName[] = "QFilePhysFS";
class QFileIOPhysFs Q_DECL_FINAL: public MediaIO
{
  Q_OBJECT
  Q_PROPERTY(QIODevice* device READ device WRITE setDevice NOTIFY deviceChanged)
  DPTR_DECLARE_PRIVATE(QFileIOPhysFs)

public:
  QFileIOPhysFs() : MediaIO(*new QFileIOPhysFsPrivate())
  {
    setDevice(&d_func().file);
  }
  virtual QString name() const Q_DECL_OVERRIDE { return QLatin1String(kQFileName);}
  void setDevice(QIODevice *dev)
  {
    DPTR_D(QFileIOPhysFs);
    if (d.dev == dev)
        return;
    d.dev = dev;
    emit deviceChanged();
  }

  QIODevice* device() const
  {
    return d_func().dev;
  }

  bool isSeekable() const Q_DECL_OVERRIDE
  {
    DPTR_D(const QFileIOPhysFs);
    return d.dev && !d.dev->isSequential();
  }

  bool isWritable() const Q_DECL_OVERRIDE
  {
    DPTR_D(const QFileIOPhysFs);
    return d.dev && d.dev->isWritable();
  }

  qint64 read(char *data, qint64 maxSize) Q_DECL_OVERRIDE
  {
    DPTR_D(QFileIOPhysFs);
    if (!d.dev)
        return 0;
    return d.dev->read(data, maxSize);
  }

  qint64 write(const char *data, qint64 maxSize) Q_DECL_OVERRIDE
  {
    DPTR_D(QFileIOPhysFs);
    if (!d.dev)
        return 0;
    return d.dev->write(data, maxSize);
  }

  bool seek(qint64 offset, int from) Q_DECL_OVERRIDE
  {
    DPTR_D(QFileIOPhysFs);
    if (!d.dev)
        return false;
    if (from == SEEK_END) {
        offset = d.dev->size() - offset;
    } else if (from == SEEK_CUR) {
        offset = d.dev->pos() + offset;
    }
    return d.dev->seek(offset);
  }

  qint64 position() const Q_DECL_OVERRIDE
  {
    DPTR_D(const QFileIOPhysFs);
    if (!d.dev)
        return 0;
    return d.dev->pos();
  }

  qint64 size() const Q_DECL_OVERRIDE
  {
    DPTR_D(const QFileIOPhysFs);
    if (!d.dev)
        return 0;
    return d.dev->size(); // sequential device returns bytesAvailable()
  }

  const QStringList& protocols() const Q_DECL_OVERRIDE
  {
    static QStringList p = QStringList() << c_sSchemeBase;
    return p;
  }
signals:
  void deviceChanged();

protected:
  void onUrlChanged() Q_DECL_OVERRIDE
  {
    DPTR_D(QFileIOPhysFs);
    if (d.file.isOpen())
        d.file.close();
    QString path(url());
    if (!path.startsWith(c_sSchemeBase + QLatin1String(":"))) {
        return;
    }
    d.file.setFileName(path);
    if (path.isEmpty())
        return;
    if (!d.dev->open(QIODevice::ReadOnly))
        qWarning() << "Failed to open [" << d.file.fileName() << "]: " << d.file.errorString();
  }

protected:
  QFileIOPhysFs(QFileIOPhysFsPrivate &d) : MediaIO(d) {}
};

typedef QFileIOPhysFs MediaIOQFilePhysFs;
static const MediaIOId MediaIOId_QFilePhysFs = mkid::id32base36_3<'P','F','S'>::value;
FACTORY_REGISTER(MediaIO, QFilePhysFs, kQFileName)

//----------------------------------------------------------------------------------------
//
void QtAV::RegisterPhysFsFileHandler()
{
  RegisterMediaIOQFilePhysFs_Man();
}

#include "PhysFsQtAVIntegration.moc"
