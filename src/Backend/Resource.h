#ifndef RESOURCE_H
#define RESOURCE_H

#include <enum.h>
#include <QObject>
#include <QMutex>
#include <QScriptValue>
#include <QSharedPointer>

BETTER_ENUM(EResourceType, qint32,
            eImage = 0,
            eMovie = 1,
            eSound = 2,
            eOther = 3);

//----------------------------------------------------------------------------------------
//
struct SResource
{
  explicit SResource(EResourceType type = EResourceType::eOther) :
    m_sPath(),
    m_type(type)
  {}
  SResource(const SResource& other) :
    m_sPath(other.m_sPath),
    m_type(other.m_type)
  {}

  QString                 m_sPath;
  EResourceType           m_type;
};

//----------------------------------------------------------------------------------------
//
class CResource : public QObject
{
  Q_OBJECT
  Q_PROPERTY(QString path      READ Name        WRITE SetName)
  Q_PROPERTY(qint32  type      READ Type        WRITE SetType)

public:
  CResource();
  explicit CResource(const CResource& other);
  ~CResource();

  void SetPath(const QString& sValue);
  QString Path();

  void SetType(qint32 type);
  qint32 Type();

  SResource Data();

private:
  mutable QMutex      m_mutex;
  SResource           m_data;
};

//----------------------------------------------------------------------------------------
//
typedef QSharedPointer<CResource>       tspResource;
typedef std::vector<tspResource>        tvspResource;
typedef std::map<QString, tspResource>  tspResourceMap;

Q_DECLARE_METATYPE(CResource*)
Q_DECLARE_METATYPE(tspResource)

// qScriptRegisterMetaType(&engine, ProjectToScriptValue, ProjectFromScriptValue);
QScriptValue ResourceToScriptValue(QScriptEngine* pEngine, CResource* const& pIn);
void ResourceFromScriptValue(const QScriptValue& object, CResource*& pOut);


#endif // RESOURCE_H
