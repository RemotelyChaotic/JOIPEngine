#ifndef SCRIPTDBWRAPPERS_H
#define SCRIPTDBWRAPPERS_H

#include "Systems/DialogueTree.h"
#include "Systems/Database/Kink.h"
#include "Systems/Lockable.h"
#include "Systems/Database/Project.h"
#include "Systems/Database/Resource.h"
#include "Systems/Database/SaveData.h"
#include "Systems/Database/Scene.h"
#include <QObject>
#include <variant>

class CResourceScriptWrapperReadOnly;
class CSaveDataWrapperReadOnly;
class CSceneScriptWrapperReadOnly;
class QJSEngine;
namespace QtLua {
  class State;
}

using tEngineType = std::variant<QJSEngine*, QtLua::State*>;

//----------------------------------------------------------------------------------------
//
class CProjectScriptWrapperReadOnly : public QObject, public CLockable
{
  Q_OBJECT
  Q_DISABLE_COPY(CProjectScriptWrapperReadOnly)
  CProjectScriptWrapperReadOnly() = delete;
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
  Q_PROPERTY(qint32         metCmdMode        READ getMetCmdMode        CONSTANT)
  Q_PROPERTY(bool           canStartAtAnyScene READ getCanStartAtAnyScene CONSTANT)
  Q_PROPERTY(bool           isUsingWeb        READ isUsingWeb           CONSTANT)
  Q_PROPERTY(bool           isUsingCodecs     READ isUsingCodecs        CONSTANT)
  Q_PROPERTY(bool           isBundled         READ isBundled            CONSTANT)
  Q_PROPERTY(bool           isReadOnly        READ isReadOnly           CONSTANT)
  Q_PROPERTY(bool           isLoaded          READ isLoaded             CONSTANT)
  Q_PROPERTY(qint32         dlState           READ getDlState           CONSTANT)
  Q_PROPERTY(QString        font              READ getFont              CONSTANT)
  Q_PROPERTY(QString        userData          READ getUserData          CONSTANT)


public:
  enum DownLoadState
  {
    Unstarted         = EDownLoadState::eUnstarted,
    DownloadRunning   = EDownLoadState::eDownloadRunning,
    Finished          = EDownLoadState::eFinished
  };
  Q_ENUM(EDownLoadState)

  enum ToyMetronomeCommandMode
  {
    Vibrate         = EToyMetronomeCommandModeFlag::eVibrate,
    Linear          = EToyMetronomeCommandModeFlag::eLinear,
    Rotate          = EToyMetronomeCommandModeFlag::eRotate,

    Default         = EToyMetronomeCommandModeFlag::eDefault,
  };
  Q_ENUM(ToyMetronomeCommandMode)

  explicit CProjectScriptWrapperReadOnly(tEngineType pEngine, const std::shared_ptr<SProject>& spProject);
  ~CProjectScriptWrapperReadOnly() override;

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
  qint32 getMetCmdMode();
  bool getCanStartAtAnyScene();
  bool isUsingWeb();
  bool isUsingCodecs();
  bool isBundled();
  bool isReadOnly();
  bool isLoaded();
  qint32 getDlState();
  QString getFont();
  QString getUserData();

  Q_INVOKABLE qint32 numKinks();
  Q_INVOKABLE QStringList kinks();
  Q_INVOKABLE QVariant kink(const QString& sName);

  Q_INVOKABLE qint32 numScenes();
  Q_INVOKABLE QStringList scenes();
  Q_INVOKABLE QVariant scene(const QString& sName);
  Q_INVOKABLE QVariant scene(qint32 iIndex);

  Q_INVOKABLE qint32 numResources();
  Q_INVOKABLE QStringList resources();
  Q_INVOKABLE QVariant resource(const QString& sValue);
  Q_INVOKABLE QVariant resource(qint32 iIndex);

  Q_INVOKABLE qint32 numTags();
  Q_INVOKABLE QStringList tags();
  Q_INVOKABLE QVariant tag(const QString& sValue);
  Q_INVOKABLE QVariant tag(qint32 iIndex);

  Q_INVOKABLE qint32 numAchievements();
  Q_INVOKABLE QStringList achievements();
  Q_INVOKABLE QVariant achievement(const QString& sValue);
  Q_INVOKABLE QVariant achievement(qint32 iIndex);

  std::shared_ptr<SProject> Data() { return m_spData; }

protected:
  virtual CResourceScriptWrapperReadOnly* CreateReosurceWrapper(const std::shared_ptr<SResource>& spResource) const;
  virtual CSaveDataWrapperReadOnly* CreateSaveDataWrapper(const std::shared_ptr<SSaveData>& spSaveData) const;
  virtual CSceneScriptWrapperReadOnly* CreateSceneWrapper(const std::shared_ptr<SScene>& spScene) const;

  std::shared_ptr<SProject>              m_spData;
  tEngineType                            m_pEngine;
};

//----------------------------------------------------------------------------------------
//
class CProjectScriptWrapperReadWrite : public CProjectScriptWrapperReadOnly
{
  Q_OBJECT
  Q_DISABLE_COPY(CProjectScriptWrapperReadWrite)
  CProjectScriptWrapperReadWrite() = delete;
  Q_PROPERTY(qint32         version           READ getVersion           WRITE setVersion)
  Q_PROPERTY(QString        versionText       READ getVersionText       WRITE setVersionText)
  Q_PROPERTY(qint32         targetVersion     READ getTargetVersion     WRITE setTargetVersion)
  Q_PROPERTY(QString        targetVersionText READ getTargetVersionText WRITE setTargetVersionText)
  Q_PROPERTY(QString        describtion       READ getDescribtion       WRITE setDescribtion)
  Q_PROPERTY(QString        titleCard         READ getTitleCard         WRITE setTitleCard)
  Q_PROPERTY(QString        map               READ getMap               WRITE setMap)
  Q_PROPERTY(QString        sceneModel        READ getSceneModel        WRITE setSceneModel)
  Q_PROPERTY(QString        playerLayout      READ getPlayerLayout      WRITE setPlayerLayout)
  Q_PROPERTY(qint32         numberOfSoundEmitters READ getNumberOfSoundEmitters WRITE setNumberOfSoundEmitters)
  Q_PROPERTY(qint32         metCmdMode        READ getMetCmdMode        WRITE setMetCmdMode)
  Q_PROPERTY(QString        font              READ getFont              WRITE setFont)
  Q_PROPERTY(QString        userData          READ getUserData          WRITE setUserData)

public:
  explicit CProjectScriptWrapperReadWrite(tEngineType pEngine, const std::shared_ptr<SProject>& spProject);
  ~CProjectScriptWrapperReadWrite() override;

  void setVersion(qint32 iValue);
  void setVersionText(const QString& sValue);
  void setTargetVersion(qint32 iValue);
  void setTargetVersionText(const QString& sValue);
  void setDescribtion(const QString& sValue);
  void setTitleCard(const QString& sValue);
  void setMap(const QString& sValue);
  void setSceneModel(const QString& sValue);
  void setPlayerLayout(const QString& sValue);
  void setNumberOfSoundEmitters(qint32 iValue);
  void setMetCmdMode(qint32 iValue);
  void setFont(const QString& sValue);
  void setUserData(const QString& sValue);

protected:
  CResourceScriptWrapperReadOnly* CreateReosurceWrapper(const std::shared_ptr<SResource>& spResource) const override;
  CSaveDataWrapperReadOnly* CreateSaveDataWrapper(const std::shared_ptr<SSaveData>& spSaveData) const override;
  CSceneScriptWrapperReadOnly* CreateSceneWrapper(const std::shared_ptr<SScene>& spScene) const override;
};

//----------------------------------------------------------------------------------------
//
class CResourceScriptWrapperReadOnly : public QObject, public CLockable
{
  Q_OBJECT
  Q_DISABLE_COPY(CResourceScriptWrapperReadOnly)
  CResourceScriptWrapperReadOnly() = delete;
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

  explicit CResourceScriptWrapperReadOnly(tEngineType pEngine, const std::shared_ptr<SResource>& spResource);
  ~CResourceScriptWrapperReadOnly() override;

  bool isAnimatedImpl();
  bool isLocalPath();
  QString getName();
  QUrl getPath();
  QUrl getSource();
  ResourceType getType();
  QString getResourceBundle();

  Q_INVOKABLE bool load();
  Q_INVOKABLE QVariant project();

  Q_INVOKABLE qint32 numTags();
  Q_INVOKABLE QStringList tags();
  Q_INVOKABLE QVariant tag(const QString& sValue);
  Q_INVOKABLE QVariant tag(qint32 iIndex);

  std::shared_ptr<SResource> Data() { return m_spData; }

protected:
  virtual CProjectScriptWrapperReadOnly* CreateProjectWrapper(const std::shared_ptr<SProject>& spProject) const;

  std::shared_ptr<SResource>    m_spData;
  tEngineType                   m_pEngine;
};

//----------------------------------------------------------------------------------------
//
class CResourceScriptWrapperReadWrite : public CResourceScriptWrapperReadOnly
{
  Q_OBJECT
  Q_DISABLE_COPY(CResourceScriptWrapperReadWrite)
  CResourceScriptWrapperReadWrite() = delete;

public:
  explicit CResourceScriptWrapperReadWrite(tEngineType pEngine, const std::shared_ptr<SResource>& spResource);
  ~CResourceScriptWrapperReadWrite() override;

  Q_INVOKABLE void addTag(const QString& sName, const QString& sCategory, const QString& sDescription);
  Q_INVOKABLE void removeTag(const QString& sValue);
  Q_INVOKABLE void removeTag(qint32 iIndex);

protected:
  CProjectScriptWrapperReadOnly* CreateProjectWrapper(const std::shared_ptr<SProject>& spProject) const override;
};

//----------------------------------------------------------------------------------------
//
class CSceneScriptWrapperReadOnly : public QObject, public CLockable
{
  Q_OBJECT
  Q_DISABLE_COPY(CSceneScriptWrapperReadOnly)
  CSceneScriptWrapperReadOnly() = delete;
  Q_PROPERTY(qint32  id               READ getId     CONSTANT)
  Q_PROPERTY(QString name             READ getName   CONSTANT)
  Q_PROPERTY(QString script           READ getScript CONSTANT)
  Q_PROPERTY(QString sceneLayout      READ getSceneLayout CONSTANT)
  Q_PROPERTY(bool    canStartHere     READ getCanStartHere CONSTANT)
  Q_PROPERTY(QString titleCard        READ getTitleCard CONSTANT)

public:
  explicit CSceneScriptWrapperReadOnly(tEngineType pEngine, const std::shared_ptr<SScene>& spScene);
  ~CSceneScriptWrapperReadOnly() override;

  qint32 getId();
  QString getName();
  QString getScript();
  QString getSceneLayout();
  bool getCanStartHere();
  QString getTitleCard();

  Q_INVOKABLE qint32 numResources();
  Q_INVOKABLE QStringList resources();
  Q_INVOKABLE QVariant resource(const QString& sValue);

  Q_INVOKABLE QVariant project();

  std::shared_ptr<SScene> Data() { return m_spData; }

protected:
  virtual CProjectScriptWrapperReadOnly* CreateProjectWrapper(const std::shared_ptr<SProject>& spProject) const;
  virtual CResourceScriptWrapperReadOnly* CreateReosurceWrapper(const std::shared_ptr<SResource>& spResource) const;

  std::shared_ptr<SScene>                m_spData;
  tEngineType                            m_pEngine;
};

//----------------------------------------------------------------------------------------
//
class CSceneScriptWrapperReadWrite : public CSceneScriptWrapperReadOnly
{
  Q_OBJECT
  Q_DISABLE_COPY(CSceneScriptWrapperReadWrite)
  CSceneScriptWrapperReadWrite() = delete;
  Q_PROPERTY(QString script           READ getScript WRITE setScript)
  Q_PROPERTY(QString sceneLayout      READ getSceneLayout WRITE setSceneLayout)
  Q_PROPERTY(QString titleCard        READ getTitleCard WRITE setTitleCard)

public:
  explicit CSceneScriptWrapperReadWrite(tEngineType pEngine, const std::shared_ptr<SScene>& spScene);
  ~CSceneScriptWrapperReadWrite() override;

  void setScript(const QString& sValue);
  void setSceneLayout(const QString& sValue);
  void setTitleCard(const QString& sValue);

  Q_INVOKABLE void addResource(const QString& sName);
  Q_INVOKABLE void removeResource(const QString& sValue);

protected:
  CProjectScriptWrapperReadOnly* CreateProjectWrapper(const std::shared_ptr<SProject>& spProject) const override;
  CResourceScriptWrapperReadOnly* CreateReosurceWrapper(const std::shared_ptr<SResource>& spResource) const override;
};

//----------------------------------------------------------------------------------------
//
class CKinkWrapperReadOnly : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(CKinkWrapperReadOnly)
  CKinkWrapperReadOnly() {}
  Q_PROPERTY(qint32   idForOrdering              READ getIdForOrdering     CONSTANT)
  Q_PROPERTY(QString  type                       READ getType              CONSTANT)
  Q_PROPERTY(QString  name                       READ getName              CONSTANT)
  Q_PROPERTY(QString  describtion                READ getDescribtion       CONSTANT)

public:
  explicit CKinkWrapperReadOnly(tEngineType pEngine, const std::shared_ptr<SKink>& spKink);
  ~CKinkWrapperReadOnly();

  qint32 getIdForOrdering();
  QString getType();
  QString getName();
  QString getDescribtion();

  Q_INVOKABLE QColor color();

  std::shared_ptr<SKink> Data() { return m_spData; }

protected:
  std::shared_ptr<SKink>              m_spData;
  tEngineType                         m_pEngine;
};

//----------------------------------------------------------------------------------------
//
class CTagWrapperReadOnly : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(CTagWrapperReadOnly)
  CTagWrapperReadOnly() {}
  Q_PROPERTY(QString  type                       READ getType              CONSTANT)
  Q_PROPERTY(QString  name                       READ getName              CONSTANT)
  Q_PROPERTY(QString  describtion                READ getDescribtion       CONSTANT)

public:
  explicit CTagWrapperReadOnly(tEngineType pEngine, const std::shared_ptr<STag>& spTag);
  ~CTagWrapperReadOnly();

  QString getType();
  QString getName();
  QString getDescribtion();

  Q_INVOKABLE QColor color();

  Q_INVOKABLE qint32 numResources();
  Q_INVOKABLE QStringList resources();

  std::shared_ptr<STag> Data() { return m_spData; }

protected:
  std::shared_ptr<STag>               m_spData;
  tEngineType                         m_pEngine;
};

//----------------------------------------------------------------------------------------
//
class CDialogueWrapperBaseReadOnly : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(CDialogueWrapperBaseReadOnly)
  CDialogueWrapperBaseReadOnly() = delete;
  Q_PROPERTY(QString  resource                READ getResource       CONSTANT)
  Q_PROPERTY(QString  name                    READ getName           CONSTANT)

public:
  explicit CDialogueWrapperBaseReadOnly(tEngineType pEngine, const std::shared_ptr<CDialogueNode>& spData);
  ~CDialogueWrapperBaseReadOnly();

  QString getResource() const;
  QString getName() const;

  std::shared_ptr<CDialogueNode> DataBase() { return m_spData; }

protected:
  tEngineType                         m_pEngine;
  std::shared_ptr<CDialogueNode>        m_spData;
};

//----------------------------------------------------------------------------------------
//
class CDialogueWrapperReadOnly : public CDialogueWrapperBaseReadOnly
{
  Q_OBJECT
  Q_DISABLE_COPY(CDialogueWrapperReadOnly)
  CDialogueWrapperReadOnly() = delete;
  Q_PROPERTY(QString  hasCondition            READ getHasCondition   CONSTANT)

public:
  explicit CDialogueWrapperReadOnly(tEngineType pEngine, const std::shared_ptr<CDialogueNodeDialogue>& spData);
  ~CDialogueWrapperReadOnly();

  bool getHasCondition() const;

  Q_INVOKABLE qint32 numDialogueData();
  Q_INVOKABLE QStringList dialogueDataList();
  Q_INVOKABLE QVariant dialogueData(const QString& sValue);
  Q_INVOKABLE QVariant dialogueData(qint32 iIndex);

  Q_INVOKABLE qint32 numTags();
  Q_INVOKABLE QStringList tags();
  Q_INVOKABLE QVariant tag(const QString& sValue);
  Q_INVOKABLE QVariant tag(qint32 iIndex);

  std::shared_ptr<CDialogueNodeDialogue> Data() { return m_spData; }

protected:
  std::shared_ptr<CDialogueNodeDialogue>  m_spData;
};

//----------------------------------------------------------------------------------------
//
class CDialogueDataWrapperReadOnly : public CDialogueWrapperBaseReadOnly
{
  Q_OBJECT
  Q_DISABLE_COPY(CDialogueDataWrapperReadOnly)
  CDialogueDataWrapperReadOnly() = delete;
  Q_PROPERTY(QString  condition               READ getCondition      CONSTANT)
  Q_PROPERTY(QString  string                  READ getString         CONSTANT)
  Q_PROPERTY(QString  soundResource           READ getSoundResource  CONSTANT)
  Q_PROPERTY(qint64   waitTimeMs              READ getWaitTime       CONSTANT)
  Q_PROPERTY(bool     skipable                READ getSkipable       CONSTANT)

public:
  explicit CDialogueDataWrapperReadOnly(tEngineType pEngine, const std::shared_ptr<CDialogueData>& spData);
  ~CDialogueDataWrapperReadOnly();

  QString getCondition() const;
  QString getString() const;
  QString getSoundResource() const;
  qint64 getWaitTime() const;
  bool getSkipable() const;

  std::shared_ptr<CDialogueData> Data() { return m_spData; }

protected:
  std::shared_ptr<CDialogueData>        m_spData;
};

//----------------------------------------------------------------------------------------
//
class CSaveDataWrapperReadOnly : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(CSaveDataWrapperReadOnly)
  CSaveDataWrapperReadOnly() = delete;
  Q_PROPERTY(QString      name               READ getName           CONSTANT)
  Q_PROPERTY(QString      describtion        READ getDescribtion    CONSTANT)
  Q_PROPERTY(SaveDataType type               READ getType           CONSTANT)
  Q_PROPERTY(QString      resource           READ getResource       CONSTANT)
  Q_PROPERTY(QVariant     data               READ getData           CONSTANT)

public:
  enum SaveDataType {
    Bool = ESaveDataType::eBool,
    Int = ESaveDataType::eInt,
    Double = ESaveDataType::eDouble,
    String = ESaveDataType::eString,
    Regexp = ESaveDataType::eRegexp,
    Date = ESaveDataType::eDate,
    Url = ESaveDataType::eUrl, // reserved for Qt6
    Array = ESaveDataType::eArray,
    Object = ESaveDataType::eObject,
    Null = ESaveDataType::eNull
  };
  Q_ENUM(SaveDataType)

  explicit CSaveDataWrapperReadOnly(tEngineType pEngine, const std::shared_ptr<SSaveData>& spData);
  ~CSaveDataWrapperReadOnly() override;

  QString getName() const;
  QString getDescribtion() const;
  SaveDataType getType() const;
  QString getResource() const;
  QVariant getData() const;

  const std::shared_ptr<SSaveData>& Data() { return m_spData; }

protected:
  std::shared_ptr<SSaveData>          m_spData;
  tEngineType                         m_pEngine;
};

//----------------------------------------------------------------------------------------
//
class CSaveDataWrapperReadWrite : public CSaveDataWrapperReadOnly
{
  Q_OBJECT
  Q_DISABLE_COPY(CSaveDataWrapperReadWrite)
  CSaveDataWrapperReadWrite() = delete;
  Q_PROPERTY(QString      describtion        READ getDescribtion WRITE setDescribtion)
  Q_PROPERTY(QString      resource           READ getResource    WRITE setResource)
  Q_PROPERTY(QVariant     data               READ getData        WRITE setData)

public:
  explicit CSaveDataWrapperReadWrite(tEngineType pEngine, const std::shared_ptr<SSaveData>& spData);
  ~CSaveDataWrapperReadWrite() override;

  void setDescribtion(const QString& sValue) const;
  void setResource(const QString& sRes) const;
  void setData(const QVariant& var) const;
};

//----------------------------------------------------------------------------------------
//
Q_DECLARE_METATYPE(CResourceScriptWrapperReadOnly*)
Q_DECLARE_METATYPE(CResourceScriptWrapperReadWrite*)
Q_DECLARE_METATYPE(CProjectScriptWrapperReadOnly*)
Q_DECLARE_METATYPE(CProjectScriptWrapperReadWrite*)
Q_DECLARE_METATYPE(CSceneScriptWrapperReadOnly*)
Q_DECLARE_METATYPE(CSceneScriptWrapperReadWrite*)
Q_DECLARE_METATYPE(CKinkWrapperReadOnly*)
Q_DECLARE_METATYPE(CTagWrapperReadOnly*)
Q_DECLARE_METATYPE(CDialogueWrapperBaseReadOnly*)
Q_DECLARE_METATYPE(CDialogueWrapperReadOnly*)
Q_DECLARE_METATYPE(CDialogueDataWrapperReadOnly*)
Q_DECLARE_METATYPE(CSaveDataWrapperReadOnly*)
Q_DECLARE_METATYPE(CSaveDataWrapperReadWrite*)

#endif // SCRIPTDBWRAPPERS_H
