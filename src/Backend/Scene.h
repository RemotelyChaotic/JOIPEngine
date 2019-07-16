#ifndef SCENE_H
#define SCENE_H

#include "Resource.h"
#include <QObject>
#include <QMutex>
#include <QScriptValue>
#include <QSharedPointer>
#include <memory>

class CProject;
class QScriptEngine;
struct SProject;
typedef QSharedPointer<CProject> tspProjectRef;

struct SScene
{
  SScene();
  SScene(const SScene& other);

  std::shared_ptr<SProject> m_spParent;
  mutable QMutex            m_mutex;
  qint32                    m_iId;
  QString                   m_sName;
  QString                   m_sScript;
  tvsResourceRefs           m_vsResourceRefs;
};

//----------------------------------------------------------------------------------------
//
class CScene : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(CScene)
  CScene() {}
  Q_PROPERTY(qint32  id        READ Id          WRITE SetId)
  Q_PROPERTY(QString name      READ Name        WRITE SetName)
  Q_PROPERTY(QString script    READ Script      WRITE SetScript)

public:
  explicit CScene(const std::shared_ptr<SScene>& spScene);
  ~CScene();

  void SetId(qint32 iValue);
  qint32 Id();

  void SetName(const QString& sValue);
  QString Name();

  void SetScript(const QString& sValue);
  QString Script();

  Q_INVOKABLE void AddResource(const QString& sValue);
  Q_INVOKABLE void ClearResources();
  Q_INVOKABLE qint32 NumResources();
  Q_INVOKABLE void RemoveResource(const QString& sValue);
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
