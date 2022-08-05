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
  const QString c_sMatcherTimeDuration = "^([0-9]+)|(([0-9]+w)?([0-9]+d)?([0-9]+h)?([0-9]+m)?([0-9]+s)?([0-9]+ms)?)$";

  const QString c_sEosKeyFile = ".eoskey";
  const QString c_sKey = "3YzCjnWTLV6SYMgK7Lql";

  enum EAlignMode : size_t
  {
    eCenter = 0,
    eLeft,
    eRight
  };

  const std::vector<QString> c_vsAlignStrings = {
    "center", "left", "right"
  };

  enum EPlayMode : size_t
  {
    eAutoplay = 0,
    eInstant,
    ePause,
    eCustom
  };

  const std::vector<QString> c_vsPlayModeStrings = {
    "autoplay", "instant", "pause", "custom"
  };

  enum ETimerStyle : size_t
  {
    eNormal = 0,
    eHidden,
    eSecret
  };

  const std::vector<QString> c_vsTimerStyleStrings = {
    "normal", "hidden", "secret"
  };

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

  QString GetEosLocator(tspResource spResource);
}

#endif // EOSHELPERS_H
