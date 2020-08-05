# CMake toolchain file for cross compiling x265 for ARM arch
# This feature is only supported as experimental. Use with caution.
# Please report bugs on bitbucket
# Run cmake with: cmake -DCMAKE_TOOLCHAIN_FILE=crosscompile.cmake -G "Unix Makefiles" ../../source && ccmake ../../source

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_CROSSCOMPILING TRUE)
set(CMAKE_SYSTEM_PROCESSOR armv6l)
set(CMAKE_C_COMPILER cl)
set(CMAKE_CXX_COMPILER cl)

add_definitions(-DX265_ARCH_ARM=1 -DHAVE_ARMV6=1 -DHAVE_NEON=1)
