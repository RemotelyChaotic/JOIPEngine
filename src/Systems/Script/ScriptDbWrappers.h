#ifndef SCRIPTDBWRAPPERS_H
#define SCRIPTDBWRAPPERS_H

#include "Systems/DialogTree.h"
#include "Systems/Kink.h"
#include "Systems/Lockable.h"
#include "Systems/Project.h"
#include "Systems/Resource.h"
#include "Systems/SaveData.h"
#include "Systems/Scene.h"
#include <QObject>
#include <variant>

class QJSEngine;
namespace QtLua {
  class State;
}

using tEngineType = std::variant<QJSEngine*, QtLua::State*>;

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
  Q_PROPERTY(qint32         metCmdMode        READ getMetCmdMode        CONSTANT)
  Q_PROPERTY(bool           isUsingWeb        READ isUsingWeb           CONSTANT)
  Q_PROPERTY(bool           isUsingCodecs     READ isUsingCodecs        CONSTANT)
  Q_PROPERTY(bool           isBundled         READ isBundled            CONSTANT)
  Q_PROPERTY(bool           isReadOnly        READ isReadOnly           CONSTANT)
  Q_PROPERTY(bool           isLoaded          READ isLoaded             CONSTANT)
  Q_PROPERTY(DownLoadState  dlState           READ getDlState           CONSTANT)
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

  explicit CProjectScriptWrapper(tEngineType pEngine, const std::shared_ptr<SProject>& spProject);
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
  qint32 getMetCmdMode();
  bool isUsingWeb();
  bool isUsingCodecs();
  bool isBundled();
  bool isReadOnly();
  bool isLoaded();
  DownLoadState getDlState();
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

private:
  std::shared_ptr<SProject>              m_spData;
  tEngineType                            m_pEngine;
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

  explicit CResourceScriptWrapper(tEngineType pEngine, const std::shared_ptr<SResource>& spResource);
  ~CResourceScriptWrapper();

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

private:
  std::shared_ptr<SResource>    m_spData;
  tEngineType                   m_pEngine;
};


//----------------------------------------------------------------------------------------
//
class CSceneScriptWrapper : public QObject, public CLockable
{
  Q_OBJECT
  Q_DISABLE_COPY(CSceneScriptWrapper)
  CSceneScriptWrapper() = delete;
  Q_PROPERTY(qint32  id               READ getId     CONSTANT)
  Q_PROPERTY(QString name             READ getName   CONSTANT)
  Q_PROPERTY(QString script           READ getScript CONSTANT)
  Q_PROPERTY(QString sceneLayout      READ getSceneLayout CONSTANT)

public:
  explicit CSceneScriptWrapper(tEngineType pEngine, const std::shared_ptr<SScene>& spScene);
  ~CSceneScriptWrapper();

  qint32 getId();
  QString getName();
  QString getScript();
  QString getSceneLayout();

  Q_INVOKABLE qint32 numResources();
  Q_INVOKABLE QStringList resources();
  Q_INVOKABLE QVariant resource(const QString& sValue);

  Q_INVOKABLE QVariant project();

  std::shared_ptr<SScene> Data() { return m_spData; }

private:
  std::shared_ptr<SScene>                m_spData;
  tEngineType                            m_pEngine;
};

//----------------------------------------------------------------------------------------
//
class CKinkWrapper : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(CKinkWrapper)
  CKinkWrapper() {}
  Q_PROPERTY(qint32   idForOrdering              READ getIdForOrdering     CONSTANT)
  Q_PROPERTY(QString  type                       READ getType              CONSTANT)
  Q_PROPERTY(QString  name                       READ getName              CONSTANT)
  Q_PROPERTY(QString  describtion                READ getDescribtion       CONSTANT)

public:
  explicit CKinkWrapper(tEngineType pEngine, const std::shared_ptr<SKink>& spKink);
  ~CKinkWrapper();

  qint32 getIdForOrdering();
  QString getType();
  QString getName();
  QString getDescribtion();

  Q_INVOKABLE QColor color();

  std::shared_ptr<SKink> Data() { return m_spData; }

private:
  std::shared_ptr<SKink>              m_spData;
  tEngineType                         m_pEngine;
};

//----------------------------------------------------------------------------------------
//
class CTagWrapper : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(CTagWrapper)
  CTagWrapper() {}
  Q_PROPERTY(QString  type                       READ getType              CONSTANT)
  Q_PROPERTY(QString  name                       READ getName              CONSTANT)
  Q_PROPERTY(QString  describtion                READ getDescribtion       CONSTANT)

public:
  explicit CTagWrapper(tEngineType pEngine, const std::shared_ptr<STag>& spTag);
  ~CTagWrapper();

  QString getType();
  QString getName();
  QString getDescribtion();

  Q_INVOKABLE QColor color();

  Q_INVOKABLE qint32 numResources();
  Q_INVOKABLE QStringList resources();

  std::shared_ptr<STag> Data() { return m_spData; }

private:
  std::shared_ptr<STag>               m_spData;
  tEngineType                         m_pEngine;
};

//----------------------------------------------------------------------------------------
//
class CDialogueWrapperBase : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(CDialogueWrapperBase)
  CDialogueWrapperBase() = delete;
  Q_PROPERTY(QString  resource                READ getResource       CONSTANT)
  Q_PROPERTY(QString  name                    READ getName           CONSTANT)

public:
  explicit CDialogueWrapperBase(tEngineType pEngine, const std::shared_ptr<CDialogueNode>& spData);
  ~CDialogueWrapperBase();

  QString getResource() const;
  QString getName() const;

  std::shared_ptr<CDialogueNode> DataBase() { return m_spData; }

protected:
  tEngineType                         m_pEngine;

private:
  std::shared_ptr<CDialogueNode>        m_spData;
};

//----------------------------------------------------------------------------------------
//
class CDialogueWrapper : public CDialogueWrapperBase
{
  Q_OBJECT
  Q_DISABLE_COPY(CDialogueWrapper)
  CDialogueWrapper() = delete;
  Q_PROPERTY(QString  hasCondition            READ getHasCondition   CONSTANT)

public:
  explicit CDialogueWrapper(tEngineType pEngine, const std::shared_ptr<CDialogueNodeDialogue>& spData);
  ~CDialogueWrapper();

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

private:
  std::shared_ptr<CDialogueNodeDialogue>  m_spData;
};

//----------------------------------------------------------------------------------------
//
class CDialogueDataWrapper : public CDialogueWrapperBase
{
  Q_OBJECT
  Q_DISABLE_COPY(CDialogueDataWrapper)
  CDialogueDataWrapper() = delete;
  Q_PROPERTY(QString  condition               READ getCondition      CONSTANT)
  Q_PROPERTY(QString  string                  READ getString         CONSTANT)
  Q_PROPERTY(QString  soundResource           READ getSoundResource  CONSTANT)
  Q_PROPERTY(qint64   waitTimeMs              READ getWaitTime       CONSTANT)
  Q_PROPERTY(bool     skipable                READ getSkipable       CONSTANT)

public:
  explicit CDialogueDataWrapper(tEngineType pEngine, const std::shared_ptr<CDialogueData>& spData);
  ~CDialogueDataWrapper();

  QString getCondition() const;
  QString getString() const;
  QString getSoundResource() const;
  qint64 getWaitTime() const;
  bool getSkipable() const;

  std::shared_ptr<CDialogueData> Data() { return m_spData; }

private:
  std::shared_ptr<CDialogueData>        m_spData;
};

//----------------------------------------------------------------------------------------
//
class CSaveDataWrapper : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(CSaveDataWrapper)
  CSaveDataWrapper() = delete;
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

  explicit CSaveDataWrapper(tEngineType pEngine, const std::shared_ptr<SSaveData>& spData);
  ~CSaveDataWrapper();

  QString getName() const;
  QString getDescribtion() const;
  SaveDataType getType() const;
  QString getResource() const;
  QVariant getData() const;

  const std::shared_ptr<SSaveData>& Data() { return m_spData; }

private:
  std::shared_ptr<SSaveData>          m_spData;
  tEngineType                         m_pEngine;
};

//----------------------------------------------------------------------------------------
//
Q_DECLARE_METATYPE(CResourceScriptWrapper*)
Q_DECLARE_METATYPE(CProjectScriptWrapper*)
Q_DECLARE_METATYPE(CSceneScriptWrapper*)
Q_DECLARE_METATYPE(CKinkWrapper*)
Q_DECLARE_METATYPE(CTagWrapper*)
Q_DECLARE_METATYPE(CDialogueWrapperBase*)
Q_DECLARE_METATYPE(CDialogueWrapper*)
Q_DECLARE_METATYPE(CDialogueDataWrapper*)
Q_DECLARE_METATYPE(CSaveDataWrapper*)

#endif // SCRIPTDBWRAPPERS_H
