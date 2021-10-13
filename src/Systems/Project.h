#ifndef PROJECT_H
#define PROJECT_H

#include "ISerializable.h"
#include "DatabaseInterface/ProjectData.h"
#include "Enums.h"
#include "Kink.h"
#include "Lockable.h"
#include "Resource.h"
#include "Scene.h"
#include <QObject>
#include <QPointer>
#include <QSharedPointer>
#include <map>
#include <memory>

class QJSEngine;

struct SProject : public ISerializable, std::enable_shared_from_this<SProject>,
                  public SProjectData
{
  SProject();
  SProject(const SProject& other);
  ~SProject() override;

  mutable QReadWriteLock    m_rwLock;
  QStringList               m_vsKinks;
  tvspScene                 m_vspScenes;
  tspResourceMap            m_spResourcesMap;
  QStringList               m_vsMountPoints;          // Archives need to be mounted via physFS to access content
                                                      // WARNING: root is NOT included in this list

  QJsonObject ToJsonObject() override;
  void FromJsonObject(const QJsonObject& json) override;

  std::shared_ptr<SProject> GetPtr()
  {
    return shared_from_this();
  }
};

//----------------------------------------------------------------------------------------
//
class CProjectScriptWrapper : public QObject, public CLockable
{
  Q_OBJECT
  Q_DISABLE_COPY(CProjectScriptWrapper)
  CProjectScriptWrapper() = delete;
  Q_PROPERTY(qint32         id                READ getId                CONSTANT)
  Q_PROPERTY(qint32         version           READ getVersion           CONSTANT)
  Q_PROPERTY(QString        versionText       READ getVersionText       CONSTANT)
  Q_PROPERTY(qint32         targetVersion     READ getTargetVersion     CONSTANT)
  Q_PROPERTY(QString        targetVersionText READ getTargetVersionText CONSTANT)
  Q_PROPERTY(QString        name              READ getName              CONSTANT)
  Q_PROPERTY(QString        folderName        READ getFolderName        CONSTANT)
  Q_PROPERTY(QString        describtion       READ getDescribtion       CONSTANT)
  Q_PROPERTY(QString        titleCard         READ getTitleCard         CONSTANT)
  Q_PROPERTY(QString        map               READ getMap               CONSTANT)
  Q_PROPERTY(QString        sceneModel        READ getSceneModel        CONSTANT)
  Q_PROPERTY(QString        playerLayout      READ getPlayerLayout      CONSTANT)
  Q_PROPERTY(qint32         numberOfSoundEmitters READ getNumberOfSoundEmitters CONSTANT)
  Q_PROPERTY(bool           isUsingWeb        READ isUsingWeb           CONSTANT)
  Q_PROPERTY(bool           isUsingCodecs     READ isUsingCodecs        CONSTANT)
  Q_PROPERTY(bool           isBundled         READ isBundled            CONSTANT)
  Q_PROPERTY(bool           isReadOnly        READ isReadOnly           CONSTANT)
  Q_PROPERTY(bool           isLoaded          READ isLoaded             CONSTANT)
  Q_PROPERTY(DownLoadState  dlState           READ getDlState           CONSTANT)
  Q_PROPERTY(QString        font              READ getFont              CONSTANT)

public:
  enum DownLoadState
  {
    Unstarted         = EDownLoadState::eUnstarted,
    DownloadRunning   = EDownLoadState::eDownloadRunning,
    Finished          = EDownLoadState::eFinished
  };
  Q_ENUM(EDownLoadState)

  explicit CProjectScriptWrapper(QJSEngine* pEngine, const std::shared_ptr<SProject>& spProject);
  ~CProjectScriptWrapper();

  qint32 getId();
  qint32 getVersion();
  QString getVersionText();
  qint32 getTargetVersion();
  QString getTargetVersionText();
  QString getName();
  QString getFolderName();
  QString getDescribtion();
  QString getTitleCard();
  QString getMap();
  QString getSceneModel();
  QString getPlayerLayout();
  qint32 getNumberOfSoundEmitters();
  bool isUsingWeb();
  bool isUsingCodecs();
  bool isBundled();
  bool isReadOnly();
  bool isLoaded();
  DownLoadState getDlState();
  QString getFont();

  Q_INVOKABLE qint32 numKinks();
  Q_INVOKABLE QStringList kinks();
  Q_INVOKABLE QJSValue kink(const QString& sName);

  Q_INVOKABLE qint32 numScenes();
  Q_INVOKABLE QStringList scenes();
  Q_INVOKABLE QJSValue scene(const QString& sName);
  Q_INVOKABLE QJSValue scene(qint32 iIndex);

  Q_INVOKABLE qint32 numResources();
  Q_INVOKABLE QStringList resources();
  Q_INVOKABLE QJSValue resource(const QString& sValue);
  Q_INVOKABLE QJSValue resource(qint32 iIndex);

  std::shared_ptr<SProject> Data() { return m_spData; }

private:
  std::shared_ptr<SProject>              m_spData;
  QJSEngine*                             m_pEngine;
};

//----------------------------------------------------------------------------------------
//
typedef std::shared_ptr<SProject> tspProject;
typedef std::vector<tspProject>   tvspProject;

Q_DECLARE_METATYPE(CProjectScriptWrapper*)
Q_DECLARE_METATYPE(tspProject)

//----------------------------------------------------------------------------------------
//
QString PhysicalProjectName(const tspProject& spProject);

//----------------------------------------------------------------------------------------
//
bool ProjectNameCheck(const QString& sProjectName, QString* sErrorText = nullptr);

//----------------------------------------------------------------------------------------
//
QString ToValidProjectName(const QString& sProjectName);

#endif // PROJECT_H
