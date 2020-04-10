#ifndef PROJECT_H
#define PROJECT_H

#include "ISerializable.h"
#include "Resource.h"
#include "Scene.h"
#include "SVersion.h"
#include <QObject>
#include <QReadWriteLock>
#include <QSharedPointer>
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
  QString                   m_sTitleCard;
  QString                   m_sMap;
  QString                   m_sSceneModel;
  bool                      m_bUsesWeb;
  bool                      m_bNeedsCodecs;
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
  Q_PROPERTY(qint32  id            READ getId            )
  Q_PROPERTY(qint32  version       READ getVersion       )
  Q_PROPERTY(qint32  targetVersion READ getTargetVersion)
  Q_PROPERTY(QString name          READ getName          )
  Q_PROPERTY(QString folderName    READ getFolderName    )
  Q_PROPERTY(QString titleCard     READ getTitleCard     )
  Q_PROPERTY(QString map           READ getMap           )
  Q_PROPERTY(QStringList kinks     READ getKinks         )
  Q_PROPERTY(bool    isUsingWeb    READ isUsingWeb       )
  Q_PROPERTY(bool    isUsingCodecs READ isUsingCodecs    )

public:
  explicit CProject(QJSEngine* pEngine, const std::shared_ptr<SProject>& spProject);
  ~CProject();

  qint32 getId();
  qint32 getVersion();
  qint32 getTargetVersion();
  QString getName();
  QString getFolderName();
  QString getTitleCard();
  QString getMap();
  QStringList getKinks();
  bool isUsingWeb();
  bool isUsingCodecs();

  Q_INVOKABLE qint32 numScenes();
  Q_INVOKABLE QJSValue scene(qint32 iIndex);

  Q_INVOKABLE qint32 numResources();
  Q_INVOKABLE QJSValue resource(const QString& sValue);

  std::shared_ptr<SProject> Data() { return m_spData; }

private:
  std::shared_ptr<SProject>  m_spData;
  QJSEngine*                 m_pEngine;
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
