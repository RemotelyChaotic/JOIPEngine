#ifndef RESOURCEBUNDLE_H
#define RESOURCEBUNDLE_H

#include "ISerializable.h"
#include <QObject>
#include <QPointer>
#include <QReadWriteLock>
#include <QUrl>
#include <memory>

struct SProject;

namespace joip_resource
{
  const QString c_sResourceBundleSuffix = "jrec";
}

struct SResourceBundle : public ISerializable, public std::enable_shared_from_this<SResourceBundle>
{
  explicit SResourceBundle();
  SResourceBundle(const SResourceBundle& other);
  ~SResourceBundle() override;

  mutable QReadWriteLock    m_rwLock;
  std::shared_ptr<SProject> m_spParent = nullptr;

  QString                   m_sName;
  QUrl                      m_sPath;
  bool                      m_bLoaded = false;

  QJsonObject ToJsonObject() override;
  void FromJsonObject(const QJsonObject& json) override;

  std::shared_ptr<SResourceBundle> GetPtr()
  {
    return shared_from_this();
  }
};

//----------------------------------------------------------------------------------------
//
typedef std::shared_ptr<SResourceBundle>     tspResourceBundle;
typedef std::vector<tspResourceBundle>       tvspResourceBundle;
typedef std::map<QString, tspResourceBundle> tspResourceBundleMap;

Q_DECLARE_METATYPE(tspResourceBundle)

//----------------------------------------------------------------------------------------
//
QString ResourceBundleUrlToAbsolutePath(const tspResourceBundle& spResourceBundle);

#endif // RESOURCEBUNDLE_H
