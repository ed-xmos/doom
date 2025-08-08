cmake_minimum_required(VERSION 3.21)
include($ENV{XMOS_CMAKE_PATH}/xcommon.cmake)
project(doom)

set(APP_HW_TARGET           XCORE-AI-EXPLORER-800.xn)
set(APP_PCA_ENABLE ON)

set(APP_COMPILER_FLAGS      -Os
                            -g
                            -report
                            -Wcpp,-std=gnu++98
                            -lquadflash
                            -DBOARD_SUPPORT_BOARD=XK_EVK_XU316
                            -DF_CPU=100000000
                            -D__EMSCRIPTEN__
                            -Xmapper --errdemote53
                            -D__XMOS__
                            -mcmodel=large
                            -DPATH_MAX=64
                            -DVERSION=1
                            -DDOOMWADDIR="\"\""
                            -DNO_PREDEFINED_LUMPS=1
                            -DUSE_EXTMEM=1
                            -Wno-unknown-pragmas
                            -DDEBUG_PRINT_SUPPORT=1
                            -DUSB_TILE=tile[1]
                            -DXUD_CORE_CLOCK=800
                            -fno-rtti
                            -fno-exceptions
                            )

set(APP_XSCOPE_SRCS         src/config.xscope)

set(XMOS_SANDBOX_DIR        ${CMAKE_CURRENT_LIST_DIR}/..)


# Function to glob files recursively and return paths relative to a base directory
function(glob_relative RESULT_VAR BASE_DIR GLOB_PATTERN)
    file(GLOB_RECURSE ABS_FILES "${GLOB_PATTERN}")
    set(REL_FILES "")
    foreach(FILE_PATH ${ABS_FILES})
        file(RELATIVE_PATH REL_PATH "${BASE_DIR}" "${FILE_PATH}")
        list(APPEND REL_FILES "${REL_PATH}")
    endforeach()
    set(${RESULT_VAR} "${REL_FILES}" PARENT_SCOPE)
endfunction()


#Fetch lib_filesystem
if(NOT EXISTS ${XMOS_SANDBOX_DIR}/lib_filesystem)
    include(FetchContent)
    FetchContent_Declare(
        lib_filesystem
        GIT_REPOSITORY git@github.com:xmos/lib_filesystem
        GIT_TAG master
        SOURCE_DIR ${XMOS_SANDBOX_DIR}/lib_filesystem
    )
    FetchContent_Populate(lib_filesystem)
endif()

#Fetch arduino-midi-sound-module
if(NOT EXISTS ${XMOS_SANDBOX_DIR}/arduino-midi-sound-module)
    include(FetchContent)
    FetchContent_Declare(
        arduino-midi-sound-module
        GIT_REPOSITORY git@github.com:ed-xmos/arduino-midi-sound-module.git
        GIT_TAG master
        SOURCE_DIR ${XMOS_SANDBOX_DIR}/arduino-midi-sound-module
    )
    FetchContent_Populate(arduino-midi-sound-module)
endif()


#Add filesystem sources and includes
set(APP_INCLUDES            src
                            src/xcore
                            ../lib_filesystem/lib_filesystem/api
                            ../lib_filesystem/lib_filesystem/src/file_systems/fatfs
                            ../lib_filesystem/lib_filesystem/src/file_systems/petit_fatfs
                            ../arduino-midi-sound-module/arduino-midi-sound-module
                            ../arduino-midi-sound-module/arduino-midi-sound-module/emscripten )

glob_relative(APP_XC_SRCS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/src/*.xc)
glob_relative(APP_C_SRCS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)
glob_relative(APP_CXX_SRCS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

# Glob source C/XC FFS files
glob_relative(FFS_XC_SRCS ${CMAKE_CURRENT_SOURCE_DIR} "${XMOS_SANDBOX_DIR}/lib_filesystem/lib_filesystem/*.xc")
glob_relative(FFS_C_SRCS ${CMAKE_CURRENT_SOURCE_DIR} "${XMOS_SANDBOX_DIR}/lib_filesystem/lib_filesystem/src/*.c")

# Glob source CPP MIDI files
set(MIDI_CXX_SRCS ../arduino-midi-sound-module/arduino-midi-sound-module/emscripten/avr/mocks.cpp)

list(APPEND APP_XC_SRCS ${FFS_XC_SRCS})
list(APPEND APP_C_SRCS ${FFS_C_SRCS})
list(APPEND APP_CXX_SRCS ${MIDI_CXX_SRCS})

# List of files to exclude
set(EXCLUDE_FILES
    "src/l_system.cpp"
    "src/l_video_sdl.cpp"
    "src/l_udp.cpp"
    "src/l_sound_sdl.cpp"
    "src/d_server.cpp"
)

# Remove excluded files from ALL_SOURCES
foreach(EXCLUDE_FILE ${EXCLUDE_FILES})
    list(REMOVE_ITEM APP_CXX_SRCS ${EXCLUDE_FILE})
endforeach()

message(STATUS ${APP_XC_SRCS})
message(STATUS ${APP_C_SRCS})
message(STATUS ${APP_CXX_SRCS})

#Fetch I2S and SPI from fwk_io
set(XMOS_DEP_DIR_i2s ${XMOS_SANDBOX_DIR}/fwk_io/modules)
set(XMOS_DEP_DIR_spi ${XMOS_SANDBOX_DIR}/fwk_io/modules)
if(NOT EXISTS ${XMOS_SANDBOX_DIR}/fwk_io)
    include(FetchContent)
    FetchContent_Declare(
        fwk_io
        GIT_REPOSITORY git@github.com:xmos/fwk_io
        GIT_TAG feature/xcommon_cmake
        SOURCE_DIR ${XMOS_SANDBOX_DIR}/fwk_io
    )
    FetchContent_Populate(fwk_io)
endif()

list(APPEND APP_DEPENDENT_MODULES   i2s
                                    spi
                                    lib_xud
                                    lib_board_support)

XMOS_REGISTER_APP()
