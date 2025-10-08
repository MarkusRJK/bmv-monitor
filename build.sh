#! /bin/bash
#

function log() {
    cmake -E cmake_echo_color --cyan "$(date +"%T"): ${1}" ;
}

function logError() {
    cmake -E cmake_echo_color --red "$(date +"%T"): ERROR ${1}" ;
}

function usage() {
    # FIXME: read all customer logos and list in usage
    echo "$0 [Release|Debug|RelWithDebInfo|MinSizeRel]"
    echo "      [AMD|ARM]"
    echo "      [distclean|clean] [clang-format|clang-format-all]"
    echo "      [all] [test] [package] ..."
    echo "      Build targets must be in right order, i.e. first clean, then ... last package"
}

if [ $# -lt 1 ]; then
    usage
    exit 0
fi

BUILD_FOLDER="./build"

echo
log "Build bmvmonitor"

#================================================================
#   Command line option processing
#================================================================

# default
BUILD_TYPE=Release
# first param must be the build type
if [ ${1^^} == 'DEBUG' ]; then
    BUILD_TYPE='Debug'
    shift
elif [ ${1^^} == 'RELEASE' ]; then
    BUILD_TYPE='Release'
    shift
elif [ ${1^^} == 'MINSIZEREL' ]; then
    BUILD_TYPE='MinSizeRel'
    shift
elif [ ${1^^} == 'RELWITHDEBINFO' ]; then
    BUILD_TYPE='RelWithDebInfo'
    shift
fi
log "Build type: $BUILD_TYPE"

# default
ARCH_INC=""
# first param must be the build type
if [ ${1^^} == 'AMD' ]; then
    ARCH_INC=""
    echo got AMD -> shift
    shift
elif [ ${1^^} == 'ARM' ]; then
    ARCH_INC="-D CMAKE_TOOLCHAIN_FILE=RasPi.cmake"
    echo got ARM -> shift
    shift
fi

DELETE_BUILD_FOLDER=0
CLEAN_FIRST=""
CLANG_FORMAT=""
# default (for quick builds - no clean, no test, no package)
BUILD_TARGETS=""
while [ $# -gt 0 ]; do
    if [ ${1^^} == 'CLEAN' ]; then
        #BUILD_TARGETS="${BUILD_TARGETS} --target clean"
        CLEAN_FIRST="--clean-first"
        shift
    elif [ ${1^^} == 'CLANG-FORMAT' ]; then
        #BUILD_TARGETS="${BUILD_TARGETS} --target clang-format"
        CLANG_FORMAT="--target clang-format"
        shift
    elif [ ${1^^} == 'CLANG-FORMAT-ALL' ]; then
        #BUILD_TARGETS="${BUILD_TARGETS} --target clang-format-all"
        CLANG_FORMAT="--target clang-format-all"
        shift
    elif [ ${1^^} == 'DISTCLEAN' ]; then
        DELETE_BUILD_FOLDER=1
        shift
    elif [ ${1^^} == 'ALL' ]; then
        BUILD_TARGETS="${BUILD_TARGETS} --target all"
        shift
    elif [ ${1^^} == 'TEST' ]; then
        BUILD_TARGETS="${BUILD_TARGETS} --target test"
        shift
    elif [ ${1^^} == 'PACKAGE' ]; then
        BUILD_TARGETS="${BUILD_TARGETS} --target package"
        shift
    elif [ ${1^^} == 'RUN' ]; then
        BUILD_TARGETS="${BUILD_TARGETS} --target run"
        shift
    else
        BUILD_ARGUMENTS="$*"
        break
    fi
done

#================================================================
#================================================================

echo type:    $BUILD_TYPE
echo targets: $BUILD_TARGETS
echo folder:  $BUILD_FOLDER
echo args:    $BUILD_ARGUMENTS
echo delete:  $DELETE_BUILD_FOLDER

if [ $DELETE_BUILD_FOLDER -eq 1 ]; then
  log "Removing build folder."
  rm -rf ${BUILD_FOLDER}
fi

# Create the target build directory and run cmake
# 
if [ ! -d "$BUILD_FOLDER/$BUILD_TYPE" ]; then
  log "Making build folder."
  mkdir -p ${BUILD_FOLDER}/$BUILD_TYPE
fi

cd ${BUILD_FOLDER}/$BUILD_TYPE
# MinGW Make system is a single-config system and build type
# must be set using -DCMAKE_BUILD_TYPE
if [ -z "$(ls -A $BUILD_TYPE 2> /dev/null)" ]; then
  log "Building Make System."
  cmake -S ../.. -Wdev -G "Unix Makefiles" \
        -D CMAKE_VERBOSE_MAKEFILE:BOOL=TRUE \
        -D CMAKE_RULE_MESSAGES:BOOL=TRUE \
        -D CMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE \
        -D CMAKE_BUILD_TYPE:STRING=$BUILD_TYPE ${ARCH_INC}
fi
#2>/dev/null

VIS_TARGETS=`echo ${BUILD_TARGETS} | sed s/--target//g`
log "Building targets in order: ${VIS_TARGETS}"

# NOTES:
#     - on parallel: setting --parallel without a number or number to high
#       may cause out of memory for the C++ compiler
#     - clang formatting must always be done before all
cmake --build . --parallel 8 $CLEAN_FIRST $CLANG_FORMAT ${BUILD_TARGETS:-'--target all'} -- $BUILD_ARGUMENTS

# Include the required resource files
# make install
if [ $? -eq 0 ]; then
    log "Finished."
else
    logError "Build failed."
fi

