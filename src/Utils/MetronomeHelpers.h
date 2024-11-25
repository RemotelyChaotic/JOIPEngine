#ifndef METRONOMEHELPERS_H
#define METRONOMEHELPERS_H

#include "Systems/DatabaseInterface/ProjectData.h"

#include <QString>
#include <map>
#include <vector>

namespace metronome
{
  const char c_sSfxBubble[] = "Bubble";
  const char c_sSfxMetronome[] = "Metronome";
  const char c_sSfxPlopp[] = "Plopp";
  const char c_sSfxSnapp[] = "Snapp";
  const char c_sSfxTic[] = "Tick";
  const char c_sSfxToc[] = "Tock";
  const char c_sSfxLubDub[] = "Lub-Dub";

  EToyMetronomeCommandModeFlags MapCmdModeToFlags(qint32 mode);
  const std::map<QString, QStringList>& MetronomeSfxMap();
  QStringList MetronomeSfxFromKey(const QString& sKey);
}

#endif // METRONOMEHELPERS_H
