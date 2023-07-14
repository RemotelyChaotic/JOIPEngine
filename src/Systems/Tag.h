#ifndef STAG_H
#define STAG_H

#include "ISerializable.h"
#include "DatabaseInterface/TagData.h"
#include <QReadWriteLock>
#include <QColor>
#include <QObject>
#include <QString>
#include <map>
#include <memory>

struct SProject;

class STag : public ISerializable, std::enable_shared_from_this<STag>,
             public STagData
{
public:
  STag();
  STag(QString sType, QString sName, QString sDescribtion);
  STag(const STag& other);
  ~STag();

  mutable QReadWriteLock  m_rwLock;
  std::shared_ptr<SProject> m_spParent;

  QJsonObject ToJsonObject() override;
  void FromJsonObject(const QJsonObject& json) override;

  std::shared_ptr<STag> GetPtr()
  {
    return shared_from_this();
  }
};

QColor CalculateTagColor(const STagData& kink);

//----------------------------------------------------------------------------------------
//
typedef std::shared_ptr<STag>      tspTag;
typedef std::map<QString, tspTag>  tspTagMap;

Q_DECLARE_METATYPE(tspTag)

#endif // STAG_H
