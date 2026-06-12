#ifndef PRELOADSCRIPTS_H
#define PRELOADSCRIPTS_H

#include "Systems/Database/Project.h"

#include <QStringList>

#include <memory>

class CScriptCommunicator;
class CScriptRunnerSignalContext;

namespace preload_scripts
{
  QStringList AvailableScriptSuffixes();
  void RunPreLoadScript(const tspProject& spProject,
                        std::weak_ptr<CScriptRunnerSignalContext> wpSignalEmitterContext,
                        const std::map<QString, std::weak_ptr<CScriptCommunicator>>& objToRegister);
}

#endif // PRELOADSCRIPTS_H
