# see https://cmake.org/cmake/help/book/mastering-cmake/chapter/Cross%20Compiling%20With%20CMake.html
# and https://earthly.dev/blog/cross-compiling-raspberry-pi/


# where is the target environment located (paths separated by space)
# staging is new: what effect?
#set(CMAKE_STAGING_PREFIX $ENV{HOME}/RPi-dev/staging-arm7-rpi2)

# Compiler must be set to arm
set(RPI_GCC "arm-linux-gnueabihf")
#set(CMAKE_C_COMPILER   ${RPI_GCC}-gcc CACHE FILEPATH "C compiler")
set(CMAKE_CXX_COMPILER ${RPI_GCC}-g++ CACHE FILEPATH "C++ compiler")
# 64-bit compiler
#set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++-12 CACHE FILEPATH "C++ compiler")
#set(CMAKE_Fortran_COMPILER ${RPI_GCC}-gfortran CACHE FILEPATH "Fortran compiler")

# Don't look for programs in the sysroot (these are ARM programs,
# they won't run on the build machine) but search programs in
# the host environment:

# Only look for libraries, headers and packages in the
# sysroot, don't look on the build machine
# search headers and libraries in the target environment
# ONLY = use target lib/includes only
# BOTH = first look in ROOT_PATH then in system
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
# hoping to search for hpp files also in /usr/include/boost/
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
# adjust the default behavior of the FIND_XXX() commands:
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

set(ENV{PKG_CONFIG_LIBDIR} "/usr/lib/arm-linux-gnueabihf/pkgconfig")

set(ENV{PKG_CONFIG_SYSROOT_DIR} "${CMAKE_SYSROOT}")

# for demangling and searching for install use
# nm --demangle --undefined example.o | grep boost

# remote install:
#add_custom_command(TARGET my_target POST_BUILD COMMAND scp $<TARGET_FILE:my_target> user@remote_host:dest_dir_path )