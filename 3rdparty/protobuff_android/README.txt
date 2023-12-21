Need to place compiled protobuff libraries here, because Android cross-compile does not work correctly.

To compile protobuff use:
    "-Dprotobuf_BUILD_TESTS=OFF"
    "-Dprotobuf_BUILD_EXAMPLES:BOOL=OFF"
    "-Dprotobuf_INSTALL=ON"
    "-Dprotobuf_WITH_ZLIB=OFF"
    "-Dprotobuf_BUILD_PROTOC_BINARIES:BOOL=OFF"
    "-DBUILD_TESTING:BOOL=OFF"

The compilation will fail at the deploy-step because wr're not building any APKs, but you will be able to extract the built libraries from the root-build folder.