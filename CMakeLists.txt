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



message(STATUS "**************************************************")
message(STATUS "Automated var content of ${PROJECT_NAME}:")
message(STATUS "gitversion path: ${GITVERSIONPATH}")
message(STATUS "APP_VERSION: ${APP_VERSION}")
message(STATUS "PACK_VERSION: ${PACK_VERSION}")
message(STATUS "COMMIT_SHA: ${COMMIT_SHA}")
message(STATUS "BRANCH_NAME: ${BRANCH_NAME}")
message(STATUS "**************************************************")

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
# recommended way to search for boost!!! (no components or required
#find_package(Boost) # causes a policy warning (no more need for this call)
message("boost incl" $Boost_INCLUDE_DIRS)
message("boost incl" $Boost_LIBRARIES)


set(Boost_USE_STATIC_LIBS OFF) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF) 



#include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include /opt/arm-linux-gnueabihf/usr/include)
include_directories(include $Boost_INCLUDE_DIRS)

# this is important to get the linker work
#set(CMAKE_EXE_LINKER_FLAGS -fuse-ld=gold)

add_executable(${PROJECT_NAME} src/main.cpp src/LineParser.cpp src/SerialPortCommunicator.cpp src/DeviceCache.cpp)

# must be after find_package and add_executable!
# -static HERE!!! makes it work... how to do without static??
target_link_options(${PROJECT_NAME} PRIVATE -static
      -L /usr/lib/arm-linux-gnueabihf/)
# TODO: use libpthreadpool!!
target_link_libraries(${PROJECT_NAME} PRIVATE
      boost_system boost_program_options pthread 
      Threads::Threads) # triggers -pthread in linkage


# Enable for Debugging of cmake variables:
#print_all_variables()


# install()...
#add_custom_command(TARGET my_target POST_BUILD COMMAND scp $<TARGET_FILE:my_target> user@remote_host:dest_dir_path ) or
#install(CODE "execute_process(COMMAND /usr/bin/rsync -avh ${INSTALL_DIR} user@remote:/home/user/)")

#use readelf -d SerialReader to examine libs

# install gcc-aarch64-linux-gnu und g++-aarch64?? and binutils-aarch64-linux-gnu
# use aarch64-linux-gnu-gcc


#sudo apt install g++-12-aarch64-linux-gnu

