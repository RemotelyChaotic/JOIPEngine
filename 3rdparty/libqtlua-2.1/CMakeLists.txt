# Copyright (C) 2007-2009 LuaDist.
# Created by Peter Kapec, modified by Alexandre Becoulet for qtlua mainstream

# Redistribution and use of this file is allowed according to the terms of the MIT license.
# For details see the COPYRIGHT file distributed with LuaDist.
# Please note that the package source code is licensed under its own license.

CMAKE_MINIMUM_REQUIRED(VERSION 3.5)
PROJECT(qtlua)

SET(INSTALL_BIN bin CACHE PATH "Where to install binaries to.")
SET(INSTALL_LIB lib CACHE PATH "Where to install libraries to.")
SET(INSTALL_INC include CACHE PATH "Where to install headers to.")

# Find Qt4
#	SET(QT_USE_QTUITOOLS 1)
#	SET(QT_USE_QTXML 1)
#	FIND_PACKAGE(Qt4 REQUIRED)
#	INCLUDE(${QT_USE_FILE})

# Find Qt5
	SET(CMAKE_INCLUDE_CURRENT_DIR ON)
  FIND_PACKAGE(Qt5 REQUIRED Core Widgets Xml)
#	SET(CMAKE_AUTOMOC ON)

# Find Lua
#	FIND_PACKAGE (Lua REQUIRED)
#	INCLUDE_DIRECTORIES (${LUA_INCLUDE_DIR})

###########################################################################

MESSAGE("Configure: qtlua - generating config.hh:")
INCLUDE(CheckIncludeFiles)
INCLUDE(CheckFunctionExists)
INCLUDE(CheckSymbolExists)
INCLUDE(CheckLibraryExists)

CHECK_INCLUDE_FILES (dlfcn.h HAVE_DLFCN_H)
CHECK_INCLUDE_FILES (inttypes.h HAVE_INTTYPES_H)
CHECK_INCLUDE_FILES (memory.h HAVE_MEMORY_H)
CHECK_INCLUDE_FILES (stdint.h HAVE_STDINT_H)
CHECK_INCLUDE_FILES (stdlib.h HAVE_STDLIB_H)
CHECK_INCLUDE_FILES (strings.h HAVE_STRINGS_H)
CHECK_INCLUDE_FILES (string.h HAVE_STRING_H)
CHECK_INCLUDE_FILES ("sys/stat.h" HAVE_SYS_STAT_H)
CHECK_INCLUDE_FILES ("sys/types.h" HAVE_SYS_TYPES_H)
CHECK_INCLUDE_FILES (unistd.h HAVE_UNISTD_H)

#does not work with out setup
#SET(CMAKE_REQUIRED_INCLUDES ${LUA_INCLUDE_DIR})
#SET(CMAKE_REQUIRED_LIBRARIES lua_static)
#CHECK_SYMBOL_EXISTS(lua_gc "lua.h" HAVE_LUA_GC)
#CHECK_SYMBOL_EXISTS(luaL_openlibs "lualib.h" HAVE_LUA_OPENLIBS)
#CHECK_SYMBOL_EXISTS(luaopen_os "lualib.h" HAVE_LUA_OSLIB)
#CHECK_SYMBOL_EXISTS(luaopen_package "lualib.h" HAVE_LUA_PACKAGELIB)
#CHECK_SYMBOL_EXISTS(luaopen_jit "lualib.h" HAVE_LUA_JITLIB)
#CHECK_SYMBOL_EXISTS(luaopen_ffi "lualib.h" HAVE_LUA_FFILIB)
set(HAVE_LUA_GC TRUE)
set(HAVE_LUA_OPENLIBS TRUE)
set(HAVE_LUA_OSLIB TRUE)
set(HAVE_LUA_PACKAGELIB TRUE)
set(HAVE_LUA_JITLIB FALSE)
set(HAVE_LUA_JITLIB FALSE)

CHECK_INCLUDE_FILES("stdlib.h;stdarg.h;string.h;float.h" STDC_HEADERS)
SET(SHLIBEXT ${CMAKE_SHARED_LIBRARY_SUFFIX})

# LT_OBJDIR

SET(PACKAGE_NAME "\"libqtlua\"")
SET(PACKAGE_TARNAME "\"libqtlua\"")
SET(PACKAGE_VERSION "\"2.1\"")
SET(PACKAGE_STRING "\"libqtlua 2.1\"")
SET(PACKAGE_BUGREPORT "\"\"")
SET(PACKAGE_URL "\"http://www.nongnu.org/libqtlua/\"")

# all is defined, so generate config.hh
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.hh.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.hh)

MESSAGE("Configure: qtlua - done.")
# end of generating config.hh file

###########################################################################

ADD_DEFINITIONS(-DHAVE_CONFIG_H)
#ADD_DEFINITIONS(-DQT_NO_CAST_FROM_BYTEARRAY)
ADD_DEFINITIONS(-DQTLUAREF_DEBUG)

INCLUDE_DIRECTORIES (${CMAKE_CURRENT_BINARY_DIR})
INCLUDE_DIRECTORIES( src )

ADD_SUBDIRECTORY( src )
if(NOT ANDROID)
  ADD_SUBDIRECTORY( tools )
endif()

