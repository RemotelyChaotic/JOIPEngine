#ifndef PROJECT_H
#define PROJECT_H

#include "ISerializable.h"
#include "Resource.h"
#include "Scene.h"
#include "SVersion.h"
#include <QObject>
#include <QPointer>
#include <QReadWriteLock>
#include <QSharedPointer>
#include <map>
#include <memory>

class QScriptEngine;

struct SProject : public ISerializable, std::enable_shared_from_this<SProject>
{
  SProject();
  SProject(const SProject& other);
  ~SProject() override;

  mutable QReadWriteLock    m_rwLock;
  qint32                    m_iId;
  SVersion                  m_iVersion;
  SVersion                  m_iTargetVersion;
  QString                   m_sName;
  QString                   m_sFolderName;
  QString                   m_sDescribtion;
  QString                   m_sTitleCard;
  QString                   m_sMap;
  QString                   m_sSceneModel;
  QString                   m_sPlayerLayout;
  bool                      m_bUsesWeb;
  bool                      m_bNeedsCodecs;
  bool                      m_bBundled;
  bool                      m_bLoaded;
  QStringList               m_vsKinks;
  tvspScene                 m_vspScenes;
  tspResourceMap            m_spResourcesMap;

  QJsonObject ToJsonObject() override;
  void FromJsonObject(const QJsonObject& json) override;

  std::shared_ptr<SProject> GetPtr()
  {
    return shared_from_this();
  }
};

//----------------------------------------------------------------------------------------
//
class CProject : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(CProject)
  CProject() {}
  Q_PROPERTY(qint32  id                READ getId                )
  Q_PROPERTY(qint32  version           READ getVersion           )
  Q_PROPERTY(QString versionText       READ getVersionText       )
  Q_PROPERTY(qint32  targetVersion     READ getTargetVersion     )
  Q_PROPERTY(QString targetVersionText READ getTargetVersionText )
  Q_PROPERTY(QString name              READ getName              )
  Q_PROPERTY(QString folderName        READ getFolderName        )
  Q_PROPERTY(QString describtion       READ getDescribtion       )
  Q_PROPERTY(QString titleCard         READ getTitleCard         )
  Q_PROPERTY(QString map               READ getMap               )
  Q_PROPERTY(QString sceneModel        READ getSceneModel        )
  Q_PROPERTY(QString playerLayout      READ getPlayerLayout      )
  Q_PROPERTY(QStringList kinks         READ getKinks             )
  Q_PROPERTY(bool    isUsingWeb        READ isUsingWeb           )
  Q_PROPERTY(bool    isUsingCodecs     READ isUsingCodecs        )
  Q_PROPERTY(bool    isBundled         READ isBundled            )
  Q_PROPERTY(bool    isLoaded          READ isLoaded             )

public:
  explicit CProject(QJSEngine* pEngine, const std::shared_ptr<SProject>& spProject);
  ~CProject();

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
  QStringList getKinks();
  bool isUsingWeb();
  bool isUsingCodecs();
  bool isBundled();
  bool isLoaded();

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
  std::map<QString, QPointer<CScene>>    m_vpLoadedScenes;
  std::map<QString, QPointer<CResource>> m_vpLoadedResources;
};

//----------------------------------------------------------------------------------------
//
typedef std::shared_ptr<SProject> tspProject;
typedef std::vector<tspProject>   tvspProject;

Q_DECLARE_METATYPE(CProject*)
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
