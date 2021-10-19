#ifndef CEOSRESOURCELOCATOR_H
#define CEOSRESOURCELOCATOR_H

#include "Systems/DatabaseInterface/ResourceData.h"
#include <QJsonDocument>
#include <functional>
#include <map>
#include <memory>

class CEosResourceLocator
{
public:
  struct SEosResourceData
  {
    QString m_sHash;
    SResourceData m_data;
    qint32 m_iWidth;
    qint32 m_iHeight;
  };

  CEosResourceLocator(const QJsonDocument& script,
                      const std::vector<QString>& vsSupportedHosts);
  ~CEosResourceLocator();

  bool LocateAllResources(QString* psError);
  QByteArray DownloadResource(std::shared_ptr<SEosResourceData> spResource,
                              std::function<QByteArray(const QUrl&, QString*)> fnFetch,
                              QString* psError);

  using tResourceMap = std::map<QString /*sHash*/, std::shared_ptr<SEosResourceData>>;
  using tGaleryData = std::map<QString /*sGaleryHash*/, tResourceMap>;
  tResourceMap                                  m_resourceMap;

private:
  bool LookupRemoteLink(const QString& sResource, QString* psError);
  bool LookupEOSResource(const QString& sResource, QString* psError);
  bool LookupGaleryImage(const tGaleryData& gallieries, const QString& sResource, QString* psError);
  bool LookupFile(const tResourceMap& files, const QString& sResource, QString* psError);

  const QJsonDocument&                          m_script;
  const std::vector<QString>&                   m_vsSupportedHosts;
  qint32                                        m_iIdCounter;
};

#endif // CEOSRESOURCELOCATOR_H
