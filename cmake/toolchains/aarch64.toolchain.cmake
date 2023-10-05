set (CMAKE_SYSTEM_NAME Linux)
set (CMAKE_SYSTEM_PROCESSOR aarch64)
set (CMAKE_SYSROOT "/home/pan/ToolChains/rpi_sysroot_64")

set (MACHINE_NAME  "raspberrypi-4B"    CACHE STRING "")

set (CC="/usr/bin/aarch64-linux-gnu-gcc")
set (CXX="/usr/bin/aarch64-linux-gnu-g++")
set (CROSS_PREFIX "/usr/bin/aarch64-linux-gnu-")

set (CMAKE_C_COMPILER   "${CROSS_PREFIX}gcc")
set (CMAKE_CXX_COMPILER "${CROSS_PREFIX}g++")

set (CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set (CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set (CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
