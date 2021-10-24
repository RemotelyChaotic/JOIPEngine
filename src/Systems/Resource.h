#ifndef RESOURCE_H
#define RESOURCE_H

#include "ISerializable.h"
#include "DatabaseInterface/ResourceData.h"
#include "Lockable.h"
#include <enum.h>
#include <QJSEngine>
#include <QObject>
#include <QPointer>
#include <QSharedPointer>
#include <memory>
#include <set>

class CProjectScriptWrapper;
class QJSEngine;
struct SProject;

namespace joip_resource {
  const QString c_sProjectFileName  = "JOIPData.json";
  const QString c_sSceneModelFile   = "SceneModel.flow";
  const QString c_sPlayerLayoutFile = "Player.layout";
}

//----------------------------------------------------------------------------------------
//
struct SResource : public ISerializable, public std::enable_shared_from_this<SResource>,
                   public SResourceData
{
  explicit SResource(EResourceType type = EResourceType::eOther);
  SResource(const SResource& other);
  ~SResource() override;

  mutable QReadWriteLock    m_rwLock;
  std::shared_ptr<SProject> m_spParent = nullptr;

  // internal variable/not serialized: currently only used for fonts
  qint32                    m_iLoadedId = -1;

  QJsonObject ToJsonObject() override;
  void FromJsonObject(const QJsonObject& json) override;

  std::shared_ptr<SResource> GetPtr()
  {
    return shared_from_this();
  }
};

//----------------------------------------------------------------------------------------
//
class CResourceScriptWrapper : public QObject, public CLockable
{
  Q_OBJECT
  Q_DISABLE_COPY(CResourceScriptWrapper)
  CResourceScriptWrapper() = delete;
  Q_PROPERTY(bool          isAnimated     READ isAnimatedImpl CONSTANT)
  Q_PROPERTY(bool          isLocal        READ isLocalPath CONSTANT)
  Q_PROPERTY(QString       name           READ getName CONSTANT)
  Q_PROPERTY(QUrl          path           READ getPath CONSTANT)
  Q_PROPERTY(QUrl          source         READ getSource CONSTANT)
  Q_PROPERTY(ResourceType  type           READ getType CONSTANT)
  Q_PROPERTY(QString       resourceBundle READ getResourceBundle CONSTANT)

public:
  enum ResourceType {
    Image    = EResourceType::eImage,
    Movie    = EResourceType::eMovie,
    Sound    = EResourceType::eSound,
    Other    = EResourceType::eOther,
    Script   = EResourceType::eScript,
    Database = EResourceType::eDatabase,
    Font     = EResourceType::eFont
  };
  Q_ENUM(ResourceType)

  enum ResourceLoadState {
    Null = 0,
    Loading,
    Loaded,
    Error,
  };
  Q_ENUM(ResourceLoadState)

  explicit CResourceScriptWrapper(QJSEngine* pEngine, const std::shared_ptr<SResource>& spResource);
  ~CResourceScriptWrapper();

  bool isAnimatedImpl();
  bool isLocalPath();
  QString getName();
  QUrl getPath();
  QUrl getSource();
  ResourceType getType();
  QString getResourceBundle();

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

Q_DECLARE_METATYPE(CResourceScriptWrapper*)
Q_DECLARE_METATYPE(tspResource)

//----------------------------------------------------------------------------------------
//
bool IsLocalFile(const QUrl& url);
QString ResourceUrlToAbsolutePath(const tspResource& spResource);
QString ResourceUrlToRelativePath(const tspResource& spResource);
QUrl ResourceUrlFromLocalFile(const QString& sPath);

//----------------------------------------------------------------------------------------
//
struct SResourceFormats
{
  static QStringList AudioFormats();
  static QStringList ArchiveFormats();
  static QStringList DatabaseFormats();
  static QStringList FontFormats();
  static QStringList ImageFormats();
  static QStringList OtherFormats();
  static QStringList ScriptFormats();
  static QStringList VideoFormats();

  static QString JoinedFormatsForFilePicker();
};

#endif // RESOURCE_H
