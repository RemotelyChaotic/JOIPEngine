#ifndef RESOURCE_H
#define RESOURCE_H

#include "ISerializable.h"
#include <enum.h>
#include <QJSEngine>
#include <QObject>
#include <QPointer>
#include <QReadWriteLock>
#include <QSharedPointer>
#include <QUrl>
#include <memory>
#include <set>

BETTER_ENUM(EResourceType, qint32,
            eImage      = 0,
            eMovie      = 1,
            eSound      = 2,
            eOther      = 3,
            eScript     = 4,
            eDatabase   = 5);

class CProject;
class QScriptEngine;
struct SProject;

namespace joip_resource {
  const QString c_sProjectFileName  = "JOIPData.json";
  const QString c_sSceneModelFile   = "SceneModel.flow";
  const QString c_sPlayerLayoutFile = "Player.layout";
}

//----------------------------------------------------------------------------------------
//
struct SResource : public ISerializable, public std::enable_shared_from_this<SResource>
{
  explicit SResource(EResourceType type = EResourceType::eOther);
  SResource(const SResource& other);
  ~SResource() override;

  std::shared_ptr<SProject> m_spParent;
  mutable QReadWriteLock    m_rwLock;
  QString                   m_sName;
  QUrl                      m_sPath;
  QUrl                      m_sSource;
  EResourceType             m_type;

  QJsonObject ToJsonObject() override;
  void FromJsonObject(const QJsonObject& json) override;

  std::shared_ptr<SResource> GetPtr()
  {
    return shared_from_this();
  }
};

//----------------------------------------------------------------------------------------
//
class CResource : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(CResource)
  CResource() {}
  Q_PROPERTY(bool          isAnimated READ isAnimatedImpl CONSTANT)
  Q_PROPERTY(bool          isLocal    READ isLocalPath CONSTANT)
  Q_PROPERTY(QString       name       READ getName CONSTANT)
  Q_PROPERTY(QUrl          path       READ getPath CONSTANT)
  Q_PROPERTY(QUrl          source     READ getSource CONSTANT)
  Q_PROPERTY(ResourceType  type       READ getType CONSTANT)

public:
  enum ResourceType {
    Image    = EResourceType::eImage,
    Movie    = EResourceType::eMovie,
    Sound    = EResourceType::eSound,
    Other    = EResourceType::eOther,
    Script   = EResourceType::eScript,
    Database = EResourceType::eDatabase
  };
  Q_ENUM(ResourceType)

  enum ResourceLoadState {
    Null = 0,
    Loading,
    Loaded,
    Error,
  };
  Q_ENUM(ResourceLoadState)

  explicit CResource(QJSEngine* pEngine, const std::shared_ptr<SResource>& spResource);
  ~CResource();

  bool isAnimatedImpl();
  bool isLocalPath();
  QString getName();
  QUrl getPath();
  QUrl getSource();
  ResourceType getType();

  Q_INVOKABLE QJSValue project();

  std::shared_ptr<SResource> Data() { return m_spData; }

private:
  std::shared_ptr<SResource>    m_spData;
  QJSEngine*                    m_pEngine;
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
QStringList DatabaseFormats();
QStringList ImageFormats();
QStringList OtherFormats();
QString ResourceUrlToAbsolutePath(const tspResource& spResource);
QStringList ScriptFormats();
QStringList VideoFormats();

#endif // RESOURCE_H
