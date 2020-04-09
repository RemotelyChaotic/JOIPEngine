#ifndef RESOURCE_H
#define RESOURCE_H

#include "ISerializable.h"
#include <enum.h>
#include <QJSEngine>
#include <QObject>
#include <QReadWriteLock>
#include <QSharedPointer>
#include <QUrl>
#include <memory>
#include <set>

BETTER_ENUM(EResourceType, qint32,
            eImage = 0,
            eMovie = 1,
            eSound = 2,
            eOther = 3);

class CProject;
class QScriptEngine;
struct SProject;

//----------------------------------------------------------------------------------------
//
struct SResource : public ISerializable
{
  explicit SResource(EResourceType type = EResourceType::eOther);
  SResource(const SResource& other);
  ~SResource() override;

  std::shared_ptr<SProject> m_spParent;
  mutable QReadWriteLock    m_rwLock;
  QString                   m_sName;
  QUrl                      m_sPath;
  EResourceType             m_type;

  QJsonObject ToJsonObject() override;
  void FromJsonObject(const QJsonObject& json) override;
};

//----------------------------------------------------------------------------------------
//
class CResource : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(CResource)
  CResource() {}
  Q_PROPERTY(QString name      READ getName)
  Q_PROPERTY(QString path      READ getPath)
  Q_PROPERTY(qint32  type      READ getType)

public:
  explicit CResource(QJSEngine* pEngine, const std::shared_ptr<SResource>& spResource);
  ~CResource();

  QString getName();
  QString getPath();
  qint32 getType();

  Q_INVOKABLE QJSValue project();

  std::shared_ptr<SResource> Data() { return m_spData; }

private:
  std::shared_ptr<SResource>m_spData;
  QJSEngine*                m_pEngine;
};

//----------------------------------------------------------------------------------------
//
typedef std::shared_ptr<SResource>      tspResource;
typedef std::vector<tspResource>        tvspResource;
typedef std::set<QString>               tvsResourceRefs;
typedef std::map<QString, tspResource>  tspResourceMap;

Q_DECLARE_METATYPE(CResource*)
Q_DECLARE_METATYPE(tspResource)

//----------------------------------------------------------------------------------------
//
QStringList AudioFormats();
QStringList ImageFormats();
QStringList OtherFormats();
QString ResourceUrlToAbsolutePath(const QUrl& url, const QString& sProjectFolder);
QStringList VideoFormats();

#endif // RESOURCE_H
