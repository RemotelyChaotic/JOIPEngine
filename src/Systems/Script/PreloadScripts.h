#ifndef PRELOADSCRIPTS_H
#define PRELOADSCRIPTS_H

#include "Systems/Database/Project.h"

#include <QStringList>

namespace preload_scripts
{
  QStringList AvailableScriptSuffixes();
  void RunPreLoadScript(const tspProject& spProject);
}

#endif // PRELOADSCRIPTS_H
