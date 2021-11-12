#include "EosHelpers.h"
#include "Systems/DatabaseManager.h"

#include <QTime>
#include <random>

namespace eos
{
  //--------------------------------------------------------------------------------------
  //
  bool LookupRemoteLink(const QString& sResourceLocator, tspProject& spProject,
                        std::shared_ptr<CDatabaseManager> spDbManager,
                        QString& sResource)
  {
    static QRegExp rxRemote(c_sMatcherRemotePrefix);
    qint32 iPos = 0;

    if ((iPos = rxRemote.indexIn(sResourceLocator, iPos)) == -1)
    {
      return false;
    }

    const QString sFound = QUrl(sResourceLocator).fileName();
    if (nullptr == spDbManager->FindResourceInProject(spProject, sFound))
    {
      return false;
    }

    sResource = sFound;
    return true;
  }

  //--------------------------------------------------------------------------------------
  //
  bool LookupGaleryImage(const QString& sResourceLocator, tspProject& spProject,
                         std::shared_ptr<CDatabaseManager> spDbManager,
                         QString& sResource)
  {
    static QRegExp rxGallery(c_sMatcherGallery);
    qint32 iPos = 0;

    QString sGallery;
    QString sId;
    if ((iPos = rxGallery.indexIn(sResourceLocator, iPos)) != -1)
    {
      sGallery = rxGallery.cap(1);
      sId = rxGallery.cap(2);
      iPos += rxGallery.matchedLength();
    }
    else
    {
      return false;
    }

    const QString sFound = sGallery + sId;
    if ("*" == sId)
    {
      tvspResource vspResources =
          spDbManager->FindResourcesInProject(spProject, QRegExp(sGallery + "(.*)"));
      if (vspResources.size() <= 0)
      {
        return false;
      }

      long unsigned int seed =
        static_cast<long unsigned int>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
      std::mt19937 generator(static_cast<long unsigned int>(seed));
      std::uniform_int_distribution<> dis(0, static_cast<qint32>(vspResources.size() - 1));
      qint32 iGeneratedIndex = dis(generator);
      tspResource& spResource = vspResources[iGeneratedIndex];

      QReadLocker locker(&spResource->m_rwLock);
      sResource = spResource->m_sName;
    }
    else
    {
      if (nullptr == spDbManager->FindResourceInProject(spProject, sFound))
      {
        return false;
      }
    }

    sResource = sFound;
    return true;
  }

  //--------------------------------------------------------------------------------------
  //
  bool LookupFile(const QString& sResourceLocator, tspProject& spProject,
                  std::shared_ptr<CDatabaseManager> spDbManager,
                  QString& sResource)
  {
    static QRegExp rxFile(c_sMatcherFile);
    static QRegExp rxFileIsRandom(c_sMatcherFileIsRandom);
    qint32 iPos = 0;

    if ((iPos = rxFile.indexIn(sResourceLocator, iPos)) == -1)
    {
      return false;
    }

    bool bIsRandom = false;
    iPos = 0;
    if ((iPos = rxFileIsRandom.indexIn(sResourceLocator, iPos)) != -1)
    {
      bIsRandom = true;
    }

    const QString sFound = sResourceLocator.mid(5); // remove 'file:'
    if (bIsRandom)
    {
      tvspResource vspResources =
          spDbManager->FindResourcesInProject(spProject, QRegExp(QString(sResourceLocator).replace("*", "(.*)")));
      if (vspResources.size() <= 0)
      {
        return false;
      }

      long unsigned int seed =
        static_cast<long unsigned int>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
      std::mt19937 generator(static_cast<long unsigned int>(seed));
      std::uniform_int_distribution<> dis(0, static_cast<qint32>(vspResources.size() - 1));
      qint32 iGeneratedIndex = dis(generator);
      tspResource& spResource = vspResources[iGeneratedIndex];

      QReadLocker locker(&spResource->m_rwLock);
      sResource = spResource->m_sName;
    }
    else
    {
      if (nullptr == spDbManager->FindResourceInProject(spProject, sFound))
      {
        return false;
      }
    }

    sResource = sFound;
    return true;
  }

  //--------------------------------------------------------------------------------------
  //
  qint64 StringTimeToMs(const QString& sTime)
  {
    qint64 iFinalNumber = 0;
    QString sNumber;
    qint32 iFactor = 1;
    for (qint32 i = sTime.size()-1; i >= 0; --i)
    {
      QChar c = sTime[i];
      if (i > 0)
      {
        if (c == 's') {
          iFinalNumber += sNumber.toInt() * iFactor;
          sNumber = "";
          if ('m' == sTime[i-1]) { iFactor = 1; }
          else { iFactor = 1000; }
        } else if (c == 'm') {
          iFinalNumber += sNumber.toInt() * iFactor;
          sNumber = "";
          if (i+1 < sTime.size() && 's' == sTime[i+1]) { iFactor = 1; }
          else { iFactor = 60 * 1000; }
        } else if (c == 'h') {
          iFinalNumber += sNumber.toInt() * iFactor;
          sNumber = "";
          iFactor = 60 * 60 * 1000;
        } else if (c == 'd') {
          iFinalNumber += sNumber.toInt() * iFactor;
          sNumber = "";
          iFactor = 24 * 60 * 60 * 1000;
        } else if (c == 'w') {
          iFinalNumber += sNumber.toInt() * iFactor;
          sNumber = "";
          iFactor = 7 * 24 * 60 * 60 * 1000;
        }
      }
      if (c.isNumber())
      {
        sNumber.prepend(c);
      }
    }
    iFinalNumber += sNumber.toInt() * iFactor;
    return iFinalNumber;
  }

  //--------------------------------------------------------------------------------------
  //
  qint64 ParseEosDuration(const QString& sDuration)
  {
    static const QRegExp matcherPeriod("^[^-]+-");
    qint32 iPos = 0;
    if ((iPos = matcherPeriod.indexIn(sDuration, iPos)) == -1)
    {
      QStringList vsDurations = sDuration.split("-");
      const QString sMin = vsDurations.first();
      const QString sMax = vsDurations.last();

      qint64 iMin = StringTimeToMs(sMin);
      qint64 iMax = StringTimeToMs(sMax);

      long unsigned int seed =
        static_cast<long unsigned int>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
      std::mt19937 generator(static_cast<long unsigned int>(seed));
      std::uniform_int_distribution<> dis(iMin, iMax);
      return dis(generator);
    }
    else
    {
      return StringTimeToMs(sDuration);
    }
  }
}
