cmake_minimum_required (VERSION 2.8.11)
project (WormHole)
# The version number.
execute_process(
    COMMAND git describe --always --tags
    OUTPUT_VARIABLE wh_VERSION)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# C_STANDARD
set (CMAKE_C_STANDARD 11)
set (CMAKE_CXX_STANDARD 11)

include_directories("include")
include_directories("include/libworm")
add_definitions(-O3 -g -pipe -flto -ffat-lto-objects) # -Wall -Wextra -Werror eliminado por problema con libressl

set(WARNINGFLAGS "-Wall -Wextra -Werror")

SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -flto -ldl -O3 -g -pipe")
SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -flto -ldl -O3 -g -pipe")

#Import all .hpp
file(GLOB libworm_INCLUDE
    "include/libworm/*.h"
    "include/libworm/*.hpp"
)

#Import all .cpp
file(GLOB libworm_SRC
    "src/libworm/*.c"
    "src/libworm/*.cpp"
)

include(Config.cmake)

# Dependencies
set(NO_REDEFINEFUNTIONS ON)

#Netlib
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/repos/netlib" ${CMAKE_MODULE_PATH})

set(NO_COMPILE_EXECUTABLES ON)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/dependencies/repos/netlib/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/dependencies/repos/netlib/src)
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/dependencies/repos/netlib dependencies/repos/netlib/build)
unset(NO_COMPILE_EXECUTABLES)

#HPTL
set(NO_COMPILE_EXECUTABLES ON)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/dependencies/repos/hptimelib/include)
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/dependencies/repos/hptimelib dependencies/repos/hptimelib/build)
unset(NO_COMPILE_EXECUTABLES)

#LibWorm
add_library(libworm_obj OBJECT ${libworm_SRC} ${libworm_INCLUDE})
# shared libraries need PIC
set_property(TARGET ${libworm_obj} PROPERTY POSITION_INDEPENDENT_CODE 1)
# actual shared and static libraries built from the same object files
add_library(libworm-shared SHARED $<TARGET_OBJECTS:libworm_obj>)
add_library(libworm STATIC $<TARGET_OBJECTS:libworm_obj>)

#Library wall messages
set_target_properties(libworm_obj PROPERTIES COMPILE_FLAGS "${WARNINGFLAGS} -fPIC")

#Renombrar libworms
set_target_properties(libworm PROPERTIES
        OUTPUT_NAME worm
        ARCHIVE_OUTPUT_NAME worm)
set_target_properties(libworm-shared PROPERTIES
        OUTPUT_NAME worm
        ARCHIVE_OUTPUT_NAME worm)


#Library Dependencies
#target_link_libraries(libworm rt pthread ssl crypto)
#target_link_libraries(libworm-shared rt pthread ssl-shared crypto-shared)
target_link_libraries(libworm netlib hptl)
target_link_libraries(libworm-shared netlib-shared hptl-shared)

