#ifndef JSONLUAWRAPPER_H
#define JSONLUAWRAPPER_H

#include <QCoreApplication>
#include <QtGlobal>

#ifdef BUILDJSONLUAWRAPPER
  #define DLLJSONLUAWRAPPER Q_DECL_EXPORT
#else
  #define DLLJSONLUAWRAPPER Q_DECL_IMPORT
#endif

inline void InitJsonLuaResources()
{
  Q_INIT_RESOURCE(json_lua_resources);
}
Q_COREAPP_STARTUP_FUNCTION(InitJsonLuaResources)

#endif // JSONLUAWRAPPER_H
