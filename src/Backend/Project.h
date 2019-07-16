#ifndef PROJECT_H
#define PROJECT_H

#include "Scene.h"
#include <QObject>
#include <QMutex>
#include <QScriptValue>
#include <QSharedPointer>

class QScriptEngine;

struct SProject
{
  SProject() = default;
  SProject(const SProject& other) :
    m_mutex(),
    m_iId(other.m_iId),
    m_iVersion(other.m_iVersion),
    m_sName(other.m_sName),
    m_sTitleCard(other.m_sTitleCard),
    m_sMap(other.m_sMap),
    m_vsScenes(other.m_vsScenes)
  {}

  mutable QMutex            m_mutex;
  qint32                    m_iId;
  qint32                    m_iVersion;
  QString                   m_sName;
  QString                   m_sTitleCard;
  QString                   m_sMap;
  tvspScene                 m_vsScenes;
  tspResourceMap            m_resources;
};

//----------------------------------------------------------------------------------------
//
class CProject : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(CProject)
  CProject() {}
  Q_PROPERTY(qint32  id        READ Id          WRITE SetId)
  Q_PROPERTY(qint32  version   READ Version     WRITE SetVersion)
  Q_PROPERTY(QString name      READ Name        WRITE SetName)
  Q_PROPERTY(QString titleCard READ TitleCard   WRITE SetTitleCard)
  Q_PROPERTY(QString map       READ Map         WRITE SetMap)

public:
  explicit CProject(const std::shared_ptr<SProject>& spProject);
  ~CProject();

  void SetId(qint32 iValue);
  qint32 Id();

  void SetVersion(qint32 iValue);
  qint32 Version();

  void SetName(const QString& sValue);
  QString Name();

  void SetTitleCard(const QString& sValue);
  QString TitleCard();

  void SetMap(const QString& sValue);
  QString Map();

  Q_INVOKABLE void AddScene(const tspScene& value);
  Q_INVOKABLE void ClearScenes();
  Q_INVOKABLE void InsertScene(qint32 iIndex, const tspScene& value);
  Q_INVOKABLE qint32 NumScenes();
  Q_INVOKABLE void RemoveScene(qint32 iIndex);
  Q_INVOKABLE tspSceneRef Scene(qint32 iIndex);

  Q_INVOKABLE void AddResource(const tspResource& value, const QString& sKey);
  Q_INVOKABLE void ClearResources();
  Q_INVOKABLE qint32 NumResources();
  Q_INVOKABLE void RemoveResource(const QString& sValue);
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
