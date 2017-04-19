cmake_minimum_required (VERSION 2.8.11)
project (WormHole)
# The version number.
execute_process(
    COMMAND git describe --always --tags
    OUTPUT_VARIABLE netlib_VERSION)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# C_STANDARD
set (CMAKE_C_STANDARD 11)
set (CMAKE_CXX_STANDARD 11)

include_directories("include")
add_definitions(-O3 -g -pipe -flto -ffat-lto-objects) # -Wall -Wextra -Werror eliminado por problema con libressl

set(WARNINGFLAGS "-Wall -Wextra -Werror")

SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -flto -O3 -g -pipe")
SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -flto -O3 -g -pipe")

#Function to make output a bit more quiet
function(message)
    if (NOT MESSAGE_QUIET)
        _message(${ARGN})
    endif()
endfunction()

#Function to remove imported executables
function(add_executable)
    if (NOT NO_COMPILE_EXECUTABLES)
        _add_executable(${ARGN})
    else()
        _add_executable(${ARGN})
        set_target_properties(${ARGV0} PROPERTIES EXCLUDE_FROM_ALL 1 EXCLUDE_FROM_DEFAULT_BUILD 1)
    endif()
endfunction()

#Function to remove custom targets
function(add_custom_target)
    if (NOT NO_ADD_CUSTOM)
        _add_custom_target(${ARGN})
    else()
    endif()
endfunction()

function(INSTALL)
    if (NOT NO_COMPILE_EXECUTABLES)
        _INSTALL(${ARGN})
    else()
    endif()
endfunction()

#Import all .hpp
file(GLOB lib_INCLUDE
    "include/*.h"
    "include/*.hpp"
)

#Import all .cpp
file(GLOB lib_SRC
    "src/*.c"
    "src/*.cpp"
)

include(Config.cmake)

# Dependencies

#Libressl
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/libressl" ${CMAKE_MODULE_PATH})
execute_process(COMMAND sh autogen.sh WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/libressl")

set(NO_COMPILE_EXECUTABLES ON)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/dependencies/libressl)
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/dependencies/libressl dependencies/libressl/build)
unset(NO_COMPILE_EXECUTABLES)


#The library objects

# this is the "object library" target: compiles the sources only once
add_library(netlib_obj OBJECT ${lib_SRC} ${lib_INCLUDE})
# shared libraries need PIC
set_property(TARGET ${hptl_obj} PROPERTY POSITION_INDEPENDENT_CODE 1)
# actual shared and static libraries built from the same object files
add_library(netlib-shared SHARED $<TARGET_OBJECTS:netlib_obj>)
add_library(netlib STATIC $<TARGET_OBJECTS:netlib_obj>)

#Renombrar netlib-shared
set_target_properties(netlib-shared PROPERTIES
        OUTPUT_NAME netlib
        ARCHIVE_OUTPUT_NAME netlib)


#Library Dependencies
target_link_libraries(netlib rt pthread ssl crypto)
target_link_libraries(netlib-shared rt pthread ssl-shared crypto-shared)

#Tests
add_executable(cli src/examples/cli)
add_executable(srv src/examples/srv)

#Link library executables
target_link_libraries(cli netlib)
target_link_libraries(srv netlib)

#Library wall messages
set_target_properties(netlib_obj PROPERTIES COMPILE_FLAGS "${WARNINGFLAGS}")
