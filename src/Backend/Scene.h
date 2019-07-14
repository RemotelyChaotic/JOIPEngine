#ifndef SCENE_H
#define SCENE_H

#include "Resource.h"
#include <QObject>
#include <QMutex>
#include <QScriptValue>
#include <QSharedPointer>
#include <memory>

class QScriptEngine;

struct SScene
{
  SScene() = default;
  SScene(const SScene& other) :
    m_iId(other.m_iId),
    m_sName(other.m_sName),
    m_sScript(other.m_sScript)
  {}

  qint32                    m_iId;
  QString                   m_sName;
  QString                   m_sScript;
  tspResourceMap            m_resources;
};

//----------------------------------------------------------------------------------------
//
class CScene : public QObject
{
  Q_OBJECT
  Q_PROPERTY(qint32  id        READ Id          WRITE SetId)
  Q_PROPERTY(QString name      READ Name        WRITE SetName)
  Q_PROPERTY(QString script    READ Script      WRITE SetScript)

public:
  CScene();
  explicit CScene(const CScene& other);
  ~CScene();

  void SetId(qint32 iValue);
  qint32 Id();

  void SetName(const QString& sValue);
  QString Name();

  void SetScript(const QString& sValue);
  QString Script();

  Q_INVOKABLE void AddResource(const tspResource& spValue);
  Q_INVOKABLE void ClearResources();
  Q_INVOKABLE qint32 NumResources();
  Q_INVOKABLE void RemoveResource(const QString& sValue);
  Q_INVOKABLE tspResource Resource(const QString& sValue);

  SScene Data();

private:
  mutable QMutex      m_mutex;
  SScene              m_data;
};

//----------------------------------------------------------------------------------------
//
typedef QSharedPointer<CScene> tspScene;
typedef std::vector<tspScene>  tvspScene;

Q_DECLARE_METATYPE(CScene*)
Q_DECLARE_METATYPE(tspScene)

// qScriptRegisterMetaType(&engine, SceneToScriptValue, SceneFromScriptValue);
QScriptValue SceneToScriptValue(QScriptEngine* pEngine, CScene* const& pIn);
void SceneFromScriptValue(const QScriptValue& object, CScene*& pOut);

#endif // SCENE_H
