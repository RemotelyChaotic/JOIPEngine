#ifndef SCENE_H
#define SCENE_H

#include "ISerializable.h"
#include "Resource.h"
#include <QObject>
#include <QReadWriteLock>
#include <QScriptValue>
#include <QSharedPointer>
#include <memory>

class CProject;
class QScriptEngine;
struct SProject;
typedef QSharedPointer<CProject> tspProjectRef;

struct SScene : public ISerializable
{
  SScene();
  SScene(const SScene& other);
  ~SScene() override;

  std::shared_ptr<SProject> m_spParent;
  mutable QReadWriteLock    m_rwLock;
  qint32                    m_iId;
  QString                   m_sName;
  QString                   m_sScript;
  tvsResourceRefs           m_vsResourceRefs;

  QJsonObject ToJsonObject() override;
  void FromJsonObject(const QJsonObject& json) override;
};

//----------------------------------------------------------------------------------------
//
class CScene : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(CScene)
  CScene() {}
  Q_PROPERTY(qint32  id        READ Id    )
  Q_PROPERTY(QString name      READ Name  )
  Q_PROPERTY(QString script    READ Script)

public:
  explicit CScene(const std::shared_ptr<SScene>& spScene);
  ~CScene();

  qint32 Id();
  QString Name();
  QString Script();

  Q_INVOKABLE qint32 NumResources();
  Q_INVOKABLE tspResourceRef Resource(const QString& sValue);

  Q_INVOKABLE tspProjectRef Project();

private:
  std::shared_ptr<SScene>    m_spData;
};

//----------------------------------------------------------------------------------------
//
typedef std::shared_ptr<SScene> tspScene;
typedef QSharedPointer<CScene>  tspSceneRef;
typedef std::vector<tspScene>   tvspScene;

Q_DECLARE_METATYPE(CScene*)
Q_DECLARE_METATYPE(tspScene)
Q_DECLARE_METATYPE(tspSceneRef)

// qScriptRegisterMetaType(&engine, SceneToScriptValue, SceneFromScriptValue);
QScriptValue SceneToScriptValue(QScriptEngine* pEngine, CScene* const& pIn);
void SceneFromScriptValue(const QScriptValue& object, CScene*& pOut);

#endif // SCENE_H
