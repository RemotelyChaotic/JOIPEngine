cmake_minimum_required(VERSION 3.12)

project(QmlSortFilterProxyModel)

include(QR)

QrStaticLib(
  QT Qml
  SOURCES
    filters/alloffilter.cpp
    filters/anyoffilter.cpp
    filters/expressionfilter.cpp
    filters/filter.cpp
    filters/filtercontainer.cpp
    filters/filtercontainerfilter.cpp
    filters/filtersqmltypes.cpp
    filters/indexfilter.cpp
    filters/rangefilter.cpp
    filters/regexpfilter.cpp
    filters/rolefilter.cpp
    filters/valuefilter.cpp
    proxyroles/expressionrole.cpp
    proxyroles/filterrole.cpp
    proxyroles/joinrole.cpp
    proxyroles/proxyrole.cpp
    proxyroles/proxyrolecontainer.cpp
    proxyroles/proxyrolesqmltypes.cpp
    proxyroles/regexprole.cpp
    proxyroles/singlerole.cpp
    proxyroles/switchrole.cpp
    qqmlsortfilterproxymodel.cpp
    sorters/expressionsorter.cpp
    sorters/filtersorter.cpp
    sorters/rolesorter.cpp
    sorters/sorter.cpp
    sorters/sortercontainer.cpp
    sorters/sortersqmltypes.cpp
    sorters/stringsorter.cpp
    )

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Qml,INTERFACE_INCLUDE_DIRECTORIES>
    )
