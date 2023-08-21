#include "MetronomeHelpers.h"

#include "Systems/Resource.h"

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QLibraryInfo>

namespace
{
  //--------------------------------------------------------------------------------------
  //
  void LoadCustomMetronomeSfxClips(std::map<QString, QString>& metronomeSfxMap)
  {
#if defined(Q_OS_ANDROID)
    Q_UNUSED(metronomeSfxMap)
#else
    constexpr char c_sSfxFolder[] = "sfx";
    const QString sFolder = QLibraryInfo::location(QLibraryInfo::PrefixPath) +
      QDir::separator() + c_sSfxFolder;

    const QStringList vsAudiFormats = SResourceFormats::AudioFormats();

    QDirIterator iter(sFolder, vsAudiFormats,
                      QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks,
                      QDirIterator::Subdirectories);
    while (iter.hasNext())
    {
      QFileInfo info(iter.next());
      if (info.isFile())
      {
        metronomeSfxMap[info.fileName()] = info.absoluteFilePath();
      }
    }
#endif
  }
}

namespace metronome
{
  //--------------------------------------------------------------------------------------
  //
  const std::map<QString, QString>& MetronomeSfxMap()
  {
    static std::map<QString, QString> metronomeSfxMap;
    if (metronomeSfxMap.empty())
    {
      metronomeSfxMap = {
        { c_sSfxBubble, ":/resources/sound/bubble_plopp.wav" },
        { c_sSfxMetronome, ":/resources/sound/metronome-85688.wav" },
        { c_sSfxPlopp, ":/resources/sound/plop-sound-mouth-100690.wav" },
        { c_sSfxSnapp, ":/resources/sound/finger-snap-43482.wav" },
        { c_sSfxTic, ":/resources/sound/menu_selection_soft.wav" },
        { c_sSfxToc, ":/resources/sound/metronome_default.wav" }
      };
      LoadCustomMetronomeSfxClips(metronomeSfxMap);
    }
    return metronomeSfxMap;
  }

  //--------------------------------------------------------------------------------------
  //
  QString MetronomeSfxFromKey(const QString& sKey)
  {
    const auto& map = MetronomeSfxMap();
    auto it = map.find(sKey);
    if (map.end() != it)
    {
      return it->second;
    }
    return map.at(c_sSfxToc);
  }
}
