#ifndef LUASANDBOXWRAPPER_H
#define LUASANDBOXWRAPPER_H

#include <QCoreApplication>
#include <QtGlobal>

#ifdef BUILDLUASANDBOX
  #define DLLLUASANDBOX Q_DECL_EXPORT
#else
  #define DLLLUASANDBOX Q_DECL_IMPORT
#endif

inline void InitLuaSandboxResources()
{
  Q_INIT_RESOURCE(lua_sandbox_resources);
}
Q_COREAPP_STARTUP_FUNCTION(InitLuaSandboxResources)

#endif // LUASANDBOXWRAPPER_H
