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
