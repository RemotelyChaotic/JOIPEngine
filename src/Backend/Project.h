#ifndef PROJECT_H
#define PROJECT_H

#include "ISerializable.h"
#include "Resource.h"
#include "Scene.h"
#include <QObject>
#include <QReadWriteLock>
#include <QScriptValue>
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
  qint32                    m_iVersion;
  QString                   m_sName;
  QString                   m_sTitleCard;
  QString                   m_sMap;
  bool                      m_bUsesWeb;
  bool                      m_bNeedsCodecs;
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
  Q_PROPERTY(qint32  id           READ Id            )
  Q_PROPERTY(qint32  version      READ Version       )
  Q_PROPERTY(QString name         READ Name          )
  Q_PROPERTY(QString titleCard    READ TitleCard     )
  Q_PROPERTY(QString map          READ Map           )
  Q_PROPERTY(bool    isUsingWeb   READ IsUsingWeb    )
  Q_PROPERTY(bool    iUsingCodecs READ IsUsingCodecs )

public:
  explicit CProject(const std::shared_ptr<SProject>& spProject);
  ~CProject();

  qint32 Id();
  qint32 Version();
  QString Name();
  QString TitleCard();
  QString Map();
  bool IsUsingWeb();
  bool IsUsingCodecs();

  Q_INVOKABLE qint32 NumScenes();
  Q_INVOKABLE tspSceneRef Scene(qint32 iIndex);

  Q_INVOKABLE qint32 NumResources();
  Q_INVOKABLE tspResourceRef Resource(const QString& sValue);

private:
  std::shared_ptr<SProject>  m_spData;
};

//----------------------------------------------------------------------------------------
//
typedef std::shared_ptr<SProject> tspProject;
typedef QSharedPointer<CProject>  tspProjectRef;
typedef std::vector<tspProject>   tvspProject;

Q_DECLARE_METATYPE(CProject*)
Q_DECLARE_METATYPE(tspProject)
Q_DECLARE_METATYPE(tspProjectRef)

// qScriptRegisterMetaType(&engine, ProjectToScriptValue, ProjectFromScriptValue);
QScriptValue ProjectToScriptValue(QScriptEngine* pEngine, CProject* const& pIn);
void ProjectFromScriptValue(const QScriptValue& object, CProject*& pOut);

#endif // PROJECT_H
