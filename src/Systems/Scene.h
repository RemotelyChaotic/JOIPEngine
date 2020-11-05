#ifndef SCENE_H
#define SCENE_H

#include "ISerializable.h"
#include <enum.h>
#include <QJSEngine>
#include <QObject>
#include <QPointer>
#include <QReadWriteLock>
#include <QSharedPointer>
#include <memory>
#include <set>

BETTER_ENUM(ESceneTransitionType, qint32,
            eRandom = 0,
            eSelection = 1);

class CProject;
class CResource;
class QScriptEngine;
struct SProject;
typedef std::set<QString>        tvsResourceRefs;

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
  Q_PROPERTY(qint32  id               READ getId     CONSTANT)
  Q_PROPERTY(QString name             READ getName   CONSTANT)
  Q_PROPERTY(QString script           READ getScript CONSTANT)

public:
  explicit CScene(QJSEngine* pEngine, const std::shared_ptr<SScene>& spScene);
  ~CScene();

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
  QPointer<CProject>                     m_pLoadedProject;
  std::map<QString, QPointer<CResource>> m_vpLoadedResources;
};

//----------------------------------------------------------------------------------------
//
typedef std::shared_ptr<SScene> tspScene;
typedef std::vector<tspScene>   tvspScene;

Q_DECLARE_METATYPE(CScene*)
Q_DECLARE_METATYPE(tspScene)

#endif // SCENE_H
