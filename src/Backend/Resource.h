#ifndef RESOURCE_H
#define RESOURCE_H

#include "ISerializable.h"
#include <enum.h>
#include <QObject>
#include <QReadWriteLock>
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
struct SResource : public ISerializable
{
  explicit SResource(EResourceType type = EResourceType::eOther);
  SResource(const SResource& other);
  ~SResource() override;

  mutable QReadWriteLock  m_rwLock;
  QString                 m_sName;
  QString                 m_sPath;
  EResourceType           m_type;

  QJsonObject ToJsonObject() override;
  void FromJsonObject(const QJsonObject& json) override;
};

//----------------------------------------------------------------------------------------
//
class CResource : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(CResource)
  CResource() {}
  Q_PROPERTY(QString name      READ Name)
  Q_PROPERTY(QString path      READ Path)
  Q_PROPERTY(qint32  type      READ Type)

public:
  explicit CResource(const std::shared_ptr<SResource>& spResource);
  ~CResource();

  QString Name();
  QString Path();
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
