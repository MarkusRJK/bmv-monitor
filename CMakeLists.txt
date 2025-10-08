cmake_minimum_required(VERSION 3.10)

project(bmvmonitor
  VERSION
    0.1
  DESCRIPTION
    "Serial bus to DBus communication"
  LANGUAGES
    CXX)
# no c++20 on Rasp; max is 17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#add_compile_definitions(
#    _SOME_DEF
#)

# Create a compile_commands.json file for clangd.
# It is also used by Intellisense in vscode so we don't need to list system include directories in c_cpp_properties.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# execute_process(COMMAND gitversion -b ${BRANCH_NAME} -showvariable SemVer
#                 WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
#                 OUTPUT_VARIABLE APP_VERSION
#                 OUTPUT_STRIP_TRAILING_WHITESPACE)



# message(STATUS "**************************************************")
# message(STATUS "Automated var content of ${PROJECT_NAME}:")
# message(STATUS "gitversion path: ${GITVERSIONPATH}")
# message(STATUS "APP_VERSION: ${APP_VERSION}")
# message(STATUS "PACK_VERSION: ${PACK_VERSION}")
# message(STATUS "COMMIT_SHA: ${COMMIT_SHA}")
# message(STATUS "BRANCH_NAME: ${BRANCH_NAME}")
# message(STATUS "**************************************************")

macro(print_all_variables)
    message(STATUS "print_all_variables------------------------------------------{")
    get_cmake_property(_variableNames VARIABLES)
    foreach (_variableName ${_variableNames})
        message(STATUS "${_variableName}=${${_variableName}}")
    endforeach()
    message(STATUS "print_all_variables------------------------------------------}")
endmacro()

# create special `PkgConfig::<MODULE>` variables for each of the pkg-config packages required
#not existing pkg_check_modules(BOOST-SYSTEM REQUIRED IMPORTED_TARGET libboost-system1.65)

# alternative to find_package below
#PKG_CHECK_MODULES(SDBUSCPP, [sdbus-c++ >= 0.6],
#    AC_MSG_ERROR([You need the sdbus-c++ library (version 0.6 or newer)]
#    [http://www.kistler.com/])
#)

#add_compile_definitions(
#    APP_VERSION=${APP_VERSION}
#    COMMIT_SHA=${COMMIT_SHA}
#    BRANCH_NAME=${BRANCH_NAME}
    # use wide characters
#    UNICODE
#    _UNICODE
#)

if (CMAKE_BUILD_TYPE MATCHES Debug)
    message("Debug build")
else()
    message("Release build")
endif()


#find all folders containing headers and list them in the INCLUDE_DIRS variable
# TODO: see https://stackoverflow.com/questions/62045438/using-cmake-glob-recurse-to-find-directories for dirs
#file(GLOB_RECURSE INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include)

#add_custom_target(clang-format
#            COMMAND clang-format -style=file -i ${MODIFIED_FILES}
#            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
#            DEPENDS ${MODIFIED_FILES}
#)
#add_custom_target(clang-format-all
#            COMMAND clang-format -style=file -i ${SOURCES}
#            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
#            DEPENDS ${SOURCES}
#)

set(CMAKE_THREAD_LIBS_INIT "-lpthread")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
set(CMAKE_HAVE_THREADS_LIBRARY 1)
set(CMAKE_USE_WIN32_THREADS_INIT 0)
set(CMAKE_USE_PTHREADS_INIT 1)
# this triggers -pthread in all compilations and linkage
set(THREADS_PREFER_PTHREAD_FLAG ON)
# which is the right flag? appears the above
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)

# the `pkg_check_modules` function is created with this call
find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)

find_package(sdbus-c++ REQUIRED)
find_package(sdbus-c++-tools REQUIRED)

# recommended way to search for boost!!! (no components or required
#find_package(Boost) # causes a policy warning (no more need for this call)
message("boost incl" $Boost_INCLUDE_DIRS)
message("boost incl" $Boost_LIBRARIES)


set(Boost_USE_STATIC_LIBS OFF) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF) 



#include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include /opt/arm-linux-gnueabihf/usr/include)
include_directories(${CMAKE_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS} ${CMAKE_BINARY_DIR})

# this is important to get the linker work
#set(CMAKE_EXE_LINKER_FLAGS -fuse-ld=gold)

# Pfad zur XML-Datei im Projektverzeichnis
set(DBUS_XML_INPUT ${CMAKE_SOURCE_DIR}/dbus/bmvmonitor.xml)

# Zielpfade für die generierten Header-Dateien im Build-Verzeichnis
set(SERVER_HEADER ${CMAKE_BINARY_DIR}/${PROJECT_NAME}-server.h)
set(CLIENT_HEADER ${CMAKE_BINARY_DIR}/${PROJECT_NAME}-client.h)

# Use the sdbus-c++-xml2cpp in SDBusCpp namespace to generate the headers
#add_custom_target("generate")
add_custom_command(
    #TARGET ${PROJECT_NAME} - cannot be used together with OUTPUT
    PRE_BUILD
    # FIXME: output to include folder within build dir
    OUTPUT ${SERVER_HEADER} ${CLIENT_HEADER}
    COMMAND SDBusCpp::sdbus-c++-xml2cpp ${DBUS_XML_INPUT}
              --proxy=${CLIENT_HEADER} --adaptor=${SERVER_HEADER}
    DEPENDS ${DBUS_XML_INPUT}
    COMMENT "Generating D-Bus interfaces for ${PROJECT_NAME}"
)


add_executable(${PROJECT_NAME} src/main.cpp src/LineParser.cpp src/SerialPortCommunicator.cpp src/DeviceCache.cpp ${SERVER_HEADER})

# must be after find_package and add_executable!
# Set link option for static linking e.g. if building 32-bit armhf for 64-bit RasPi
#target_link_options(${PROJECT_NAME} PRIVATE -static)

# it appears that the cmake mechanism selects the correct lib include!!!
#target_link_options(${PROJECT_NAME} PRIVATE
#      -L /usr/lib/${RPI_GCC}/)

# TODO: use libpthreadpool!!
target_link_libraries(${PROJECT_NAME} PRIVATE
      boost_system boost_program_options pthread
      SDBusCpp::sdbus-c++
      Threads::Threads) # triggers -pthread in linkage


# Enable for Debugging of cmake variables:
#print_all_variables()


# install()...
#add_custom_command(TARGET my_target POST_BUILD COMMAND scp $<TARGET_FILE:my_target> user@remote_host:dest_dir_path ) or
#install(CODE "execute_process(COMMAND /usr/bin/rsync -avh ${INSTALL_DIR} user@remote:/home/user/)")

#use readelf -d bmvmonitor to examine libs

#install(SCRIPT <dir>/post_install.cmake)

set(TEST_SYSTEM tcp:192.168.29.31:54321)

# option 2: define a deploy target and add post-build commands
add_custom_target("run")
# this runs first the command of target run which is socat...
# then builds project_name (depends) - post_build???
# the command is run in ${CMAKE_BINARY_DIR} which is e.g. build/Release
add_custom_command(TARGET run
    POST_BUILD
    # ensure ssh homecontrol sudo socat tcp-l:54321,reuseaddr,fork file:/dev/ttyUSB1,nonblock,waitlock=/var/run/tty.lock
    # FIXME: ensure socat is not already running
    COMMAND pkill socat || echo -n ""
    COMMAND socat pty,link=${CMAKE_BINARY_DIR}/ttyV0,waitslave ${TEST_SYSTEM} &
    COMMAND sleep 3
    COMMAND ${CMAKE_BINARY_DIR}/${PROJECT_NAME} 19200 ${CMAKE_BINARY_DIR}/ttyV0
    DEPENDS ${PROJECT_NAME}
)
