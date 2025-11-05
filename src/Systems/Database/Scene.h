#ifndef SCENE_H
#define SCENE_H

#include "ISerializable.h"
#include "Systems/DatabaseInterface/SceneData.h"
#include <QPointer>
#include <QReadWriteLock>
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
typedef std::shared_ptr<SScene> tspScene;
typedef std::vector<tspScene>   tvspScene;

Q_DECLARE_METATYPE(tspScene)

#endif // SCENE_H
