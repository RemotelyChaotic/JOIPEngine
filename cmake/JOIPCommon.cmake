# Code from https://cmake.org/pipermail/cmake/2015-April/060464.html
function(configure_file_generate)
   # Configure files that contain both normal items
   # to configure (referenced as ${VAR} or @VAR@) as well as
   # generator expressions (referenced as $<...>).
   # The arguments of this function have exactly the
   # same syntax and meaning as configure_file.

   set(intermediate_file_suffix "_cf_only")
   list(GET ARGV 0 input_file)
   list(GET ARGV 1 output_file)
   set(intermediate_file ${output_file}${intermediate_file_suffix})

   # Locally modify ARGV so that output file for configure file is
   # redirected to intermediate_file.
   list(REMOVE_AT ARGV 1)
   list(INSERT ARGV 1 ${intermediate_file})

   # Configure all but generator expressions.
   configure_file(${ARGV})

   # Configure generator expressions.
   # N.B. these ${output_file} results will only be available
   # at generate time.
   file(GENERATE
     OUTPUT ${output_file}
     INPUT ${intermediate_file}
     )
endfunction(configure_file_generate)

# Gets a variety of qt-Paths
function(qt_get_variables)
  get_target_property(qmake Qt5::qmake LOCATION)
  execute_process(
      COMMAND ${qmake} -query QT_VERSION
      OUTPUT_VARIABLE qt_version
      OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  set(QT_VERSION ${qt_version} CACHE INTERNAL "")
  execute_process(
      COMMAND ${qmake} -query QT_INSTALL_PREFIX
      OUTPUT_VARIABLE qt_install_prefix
      OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  set(QT_INSTALL_PREFIX ${qt_install_prefix} CACHE INTERNAL "")
  execute_process(
      COMMAND ${qmake} -query QT_INSTALL_HEADERS
      OUTPUT_VARIABLE qt_install_headers
      OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  set(QT_INSTALL_HEADERS ${qt_install_headers} CACHE INTERNAL "")
  execute_process(
      COMMAND ${qmake} -query QT_INSTALL_LIBS
      OUTPUT_VARIABLE qt_install_libs
      OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  set(QT_INSTALL_LIBS ${qt_install_libs} CACHE INTERNAL "")
  execute_process(
      COMMAND ${qmake} -query QT_INSTALL_BINS
      OUTPUT_VARIABLE qt_install_bins
      OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  set(QT_INSTALL_BINS ${qt_install_bins} CACHE INTERNAL "")
  execute_process(
      COMMAND ${qmake} -query QT_INSTALL_PLUGINS
      OUTPUT_VARIABLE qt_install_plugins
      OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  set(QT_INSTALL_PLUGINS ${qt_install_plugins} CACHE INTERNAL "")
  execute_process(
      COMMAND ${qmake} -query QT_INSTALL_QML
      OUTPUT_VARIABLE qt_install_qml
      OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  set(QT_INSTALL_QML ${qt_install_qml} CACHE INTERNAL "")
  execute_process(
      COMMAND ${qmake} -query QT_INSTALL_TRANSLATIONS
      OUTPUT_VARIABLE qt_install_translations
      OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  set(QT_INSTALL_TRANSLATIONS ${qt_install_translations} CACHE INTERNAL "")
  execute_process(
      COMMAND ${qmake} -query QMAKE_SPEC
      OUTPUT_VARIABLE qmake_spec
      OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  set(QMAKE_SPEC ${qmake_spec} CACHE INTERNAL "")
endfunction()
