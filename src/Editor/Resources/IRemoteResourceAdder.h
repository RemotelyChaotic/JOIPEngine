#ifndef IREMOTERESOURCEADDER_H
#define IREMOTERESOURCEADDER_H

#include "Systems/DatabaseInterface/ResourceData.h"

#include <QUrl>

class IRemoteResourceAdder
{
public:
  static constexpr char c_sUserAgent[] = "User-Agent";
  static constexpr char c_sDownloadProperty[] = "DownloadAsFile";

  virtual ~IRemoteResourceAdder(){}

  virtual void AddResource(const QUrl& url, bool bDownloadAndAddAsFile = false) = 0;

  virtual bool CanDownloadAndSaveAsFile() const = 0;

  virtual bool CanHandleUrl(const QUrl& url) const = 0;

  virtual QString Name() const = 0;

//signals:
  virtual void SignalNewResourceFile(const SResourceData& res, const std::vector<STagData>& vsTags,
                                     const QByteArray& ba, bool bAddAsFile) = 0;

protected:
  IRemoteResourceAdder(){}
};

#endif // IREMOTERESOURCEADDER_H
