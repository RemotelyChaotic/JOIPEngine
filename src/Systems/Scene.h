#ifndef SCENE_H
#define SCENE_H

#include "ISerializable.h"
#include "Lockable.h"
#include "DatabaseInterface/SceneData.h"
#include <QJSEngine>
#include <QObject>
#include <QPointer>
#include <QSharedPointer>
#include <memory>
#include <set>

class CProjectScriptWrapper;
class CResourceScriptWrapper;
class QJSEngine;
struct SProject;
typedef std::set<QString>        tvsResourceRefs;

struct SScene : public ISerializable, public SSceneData
{
  SScene();
  SScene(const SScene& other);
  ~SScene() override;

  mutable QReadWriteLock    m_rwLock;
  std::shared_ptr<SProject> m_spParent;
  tvsResourceRefs           m_vsResourceRefs;

  QJsonObject ToJsonObject() override;
  void FromJsonObject(const QJsonObject& json) override;
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

public:
  explicit CSceneScriptWrapper(QJSEngine* pEngine, const std::shared_ptr<SScene>& spScene);
  ~CSceneScriptWrapper();

  qint32 getId();
  QString getName();
  QString getScript();

  Q_INVOKABLE qint32 numResources();
  Q_INVOKABLE QStringList resources();
  Q_INVOKABLE QJSValue resource(const QString& sValue);

  Q_INVOKABLE QJSValue project();

  std::shared_ptr<SScene> Data() { return m_spData; }

private:
  std::shared_ptr<SScene>                m_spData;
  QJSEngine*                             m_pEngine;
};

//----------------------------------------------------------------------------------------
//
typedef std::shared_ptr<SScene> tspScene;
typedef std::vector<tspScene>   tvspScene;

Q_DECLARE_METATYPE(CSceneScriptWrapper*)
Q_DECLARE_METATYPE(tspScene)

#endif // SCENE_H
