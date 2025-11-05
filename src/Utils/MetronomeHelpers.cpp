#include "MetronomeHelpers.h"
#include "Application.h"
#include "Settings.h"

#include "Systems/Database/Resource.h"

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QLibraryInfo>

namespace
{
  //--------------------------------------------------------------------------------------
  //
  void LoadCustomMetronomeSfxClips(std::map<QString, QStringList>& metronomeSfxMap,
                                   const QString& sFolder)
  {
#if defined(Q_OS_ANDROID)
    Q_UNUSED(metronomeSfxMap)
#else
    const QStringList vsAudiFormats = SResourceFormats::AudioFormats();

    QDirIterator iter(sFolder, vsAudiFormats,
                      QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks,
                      QDirIterator::Subdirectories);
    while (iter.hasNext())
    {
      QFileInfo info(iter.next());
      if (info.isFile())
      {
        metronomeSfxMap[info.fileName()] = QStringList{} << info.absoluteFilePath();
      }
      else if (info.isDir())
      {
        std::map<QString, QStringList> childFiles;
        LoadCustomMetronomeSfxClips(childFiles, info.absoluteFilePath());
        for (const auto& [_, vsChildren] : childFiles)
        {
          for (const QString& sFile : vsChildren)
          {
            metronomeSfxMap[info.fileName()].push_back(sFile);
          }
        }
      }
    }
#endif
  }
}

namespace metronome
{
  //--------------------------------------------------------------------------------------
  //
  EToyMetronomeCommandModeFlags MapCmdModeToFlags(qint32 mode)
  {
    return EToyMetronomeCommandMode::eDefault == mode ?
             EToyMetronomeCommandModeFlags(CApplication::Instance()->Settings()->MetronomeDefCommands()) :
             EToyMetronomeCommandModeFlags(mode);
  }

  //--------------------------------------------------------------------------------------
  //
  const std::map<QString, QStringList>& MetronomeSfxMap()
  {
    static std::map<QString, QStringList> metronomeSfxMap;
    if (metronomeSfxMap.empty())
    {
      metronomeSfxMap = {
        { c_sSfxBubble, {":/resources/sound/bubble_plopp.wav"} },
        { c_sSfxMetronome, {":/resources/sound/metronome-85688.wav"} },
        { c_sSfxPlopp, {":/resources/sound/plop-sound-mouth-100690.wav"} },
        { c_sSfxSnapp, {":/resources/sound/finger-snap-43482.wav"} },
        { c_sSfxTic, {":/resources/sound/menu_selection_soft.wav"} },
        { c_sSfxToc, {":/resources/sound/metronome_default.wav"} },
        { c_sSfxLubDub, {":/resources/sound/lubdub/HB (bass boost 5.0 -10dB) pt1.wav",
                         ":/resources/sound/lubdub/HB (bass boost 5.0 -10dB) pt2 (trim).wav"} }
      };
      constexpr char c_sSfxFolder[] = "sfx";
#if defined(Q_OS_LINUX)
      QString sFolder = QLibraryInfo::location(QLibraryInfo::PrefixPath);
      const QString sAppImg = qgetenv("APPIMAGE");
      if (!sAppImg.isEmpty())
      {
        sFolder = QFileInfo(sAppImg).absolutePath();
      }
      sFolder = sFolder +
#else
      const QString sFolder = QLibraryInfo::location(QLibraryInfo::PrefixPath) +
#endif
        QDir::separator() + c_sSfxFolder;
      LoadCustomMetronomeSfxClips(metronomeSfxMap, sFolder);
    }
    return metronomeSfxMap;
  }

  //--------------------------------------------------------------------------------------
  //
  QStringList MetronomeSfxFromKey(const QString& sKey)
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
