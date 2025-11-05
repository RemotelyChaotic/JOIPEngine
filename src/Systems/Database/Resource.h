#ifndef RESOURCE_H
#define RESOURCE_H

#include "ISerializable.h"
#include "Systems/DatabaseInterface/ResourceData.h"
#include <enum.h>
#include <QJSEngine>
#include <QObject>
#include <QPointer>
#include <QReadWriteLock>
#include <QSharedPointer>
#include <memory>
#include <set>

class CProjectScriptWrapper;
class QJSEngine;
struct SProject;
typedef std::shared_ptr<SProject>      tspProject;

namespace joip_resource
{
  const QString c_sProjectFileName  = "JOIPData.json";
  const QString c_sSceneModelFile   = "SceneModel.flow";
  const QString c_sDialogueFileType   = "joipdlg";
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
typedef std::shared_ptr<SResource>      tspResource;
typedef std::vector<tspResource>        tvspResource;
typedef std::set<QString>               tvsResourceRefs;
typedef std::map<QString, tspResource>  tspResourceMap;

Q_DECLARE_METATYPE(tspResource)

//----------------------------------------------------------------------------------------
//
bool IsLocalFile(const QUrl& url);
QString PhysicalResourcePath(const tspResource& spResource);
QString PhysicalResourcePath(const QUrl& url, const tspProject& spProject,
                             const QString& sResourceBundle = QString(), const QString& sResourceName = QString());
QString ResourceUrlToAbsolutePath(const tspResource& spResource, const QString& sResourceScheme = ":");
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
  static QStringList LayoutFormats();
  static QStringList OtherFormats();
  static QStringList ScriptFormats();
  static QStringList SequenceFormat();
  static QStringList VideoFormats();

  static QString JoinedFormatsForFilePicker();
};

#endif // RESOURCE_H
