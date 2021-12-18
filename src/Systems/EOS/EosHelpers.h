#ifndef EOSHELPERS_H
#define EOSHELPERS_H

#include "Systems/Project.h"
#include <QString>
#include <memory>

class CDatabaseManager;

namespace eos
{
  const QString c_sMatcherRemotePrefix = "^https*:\\/\\/";
  const QString c_sMatcherGallery = "^gallery:([^/]+)\\/(.*)$";
  const QString c_sMatcherFile = "^file:(.*)$";
  const QString c_sMatcherFileIsRandom = "\\*";

  const QString c_sEosKeyFile = ".eoskey";
  const QString c_sKey = "3YzCjnWTLV6SYMgK7Lql";

  QString GetEOSK();

  bool LookupRemoteLink(const QString& sResourceLocator, tspProject& spProject,
                        std::shared_ptr<CDatabaseManager> spDbManager,
                        QString& sResource);

  bool LookupGaleryImage(const QString& sResourceLocator, tspProject& spProject,
                         std::shared_ptr<CDatabaseManager> spDbManager,
                         QString& sResource);

  bool LookupFile(const QString& sResourceLocator, tspProject& spProject,
                  std::shared_ptr<CDatabaseManager> spDbManager,
                  QString& sResource);

  qint64 ParseEosDuration(const QString& sDuration);

  bool VerifyProjectEditable(tspProject& spProject);
}

#endif // EOSHELPERS_H
