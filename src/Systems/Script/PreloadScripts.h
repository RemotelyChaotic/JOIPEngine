#ifndef PRELOADSCRIPTS_H
#define PRELOADSCRIPTS_H

#include "Systems/Database/Project.h"

#include <QStringList>

#include <memory>

class CScriptRunnerSignalContext;

namespace preload_scripts
{
  QStringList AvailableScriptSuffixes();
  void RunPreLoadScript(const tspProject& spProject, std::weak_ptr<CScriptRunnerSignalContext> wpSignalEmitterContext);
}

#endif // PRELOADSCRIPTS_H
