#ifndef METRONOMEHELPERS_H
#define METRONOMEHELPERS_H

#include <QString>
#include <map>

namespace metronome
{
  const char c_sSfxBubble[] = "Bubble";
  const char c_sSfxMetronome[] = "Metronome";
  const char c_sSfxPlopp[] = "Plopp";
  const char c_sSfxSnapp[] = "Snapp";
  const char c_sSfxTic[] = "Tick";
  const char c_sSfxToc[] = "Tock";

  const std::map<QString, QString>& MetronomeSfxMap();
  QString MetronomeSfxFromKey(const QString& sKey);
}

#endif // METRONOMEHELPERS_H
