#ifndef RESOURCE_H
#define RESOURCE_H

#include <enum.h>
#include <QObject>
#include <QMutex>
#include <QScriptValue>
#include <QSharedPointer>
#include <memory>
#include <set>

BETTER_ENUM(EResourceType, qint32,
            eImage = 0,
            eMovie = 1,
            eSound = 2,
            eOther = 3);

class QScriptEngine;

//----------------------------------------------------------------------------------------
//
struct SResource
{
  explicit SResource(EResourceType type = EResourceType::eOther) :
    m_mutex(),
    m_sPath(),
    m_type(type)
  {}
  SResource(const SResource& other) :
    m_mutex(),
    m_sPath(other.m_sPath),
    m_type(other.m_type)
  {}

  mutable QMutex          m_mutex;
  QString                 m_sPath;
  EResourceType           m_type;
};

//----------------------------------------------------------------------------------------
//
class CResource : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(CResource)
  CResource() {}
  Q_PROPERTY(QString path      READ Path        WRITE SetPath)
  Q_PROPERTY(qint32  type      READ Type        WRITE SetType)

public:
  explicit CResource(const std::shared_ptr<SResource>& spResource);
  ~CResource();

  void SetPath(const QString& sValue);
  QString Path();

  void SetType(qint32 type);
  qint32 Type();

private:
  std::shared_ptr<SResource>m_spData;
};

//----------------------------------------------------------------------------------------
//
typedef std::shared_ptr<SResource>      tspResource;
typedef QSharedPointer<CResource>       tspResourceRef;
typedef std::vector<tspResource>        tvspResource;
typedef std::set<QString>               tvsResourceRefs;
typedef std::map<QString, tspResource>  tspResourceMap;

Q_DECLARE_METATYPE(CResource*)
Q_DECLARE_METATYPE(tspResource)
Q_DECLARE_METATYPE(tspResourceRef)

// qScriptRegisterMetaType(&engine, ProjectToScriptValue, ProjectFromScriptValue);
QScriptValue ResourceToScriptValue(QScriptEngine* pEngine, CResource* const& pIn);
void ResourceFromScriptValue(const QScriptValue& object, CResource*& pOut);


#endif // RESOURCE_H
