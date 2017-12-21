cmake_minimum_required (VERSION 2.8.11)
project (WormHole)
# The version number.
execute_process(
    COMMAND git describe --always --tags
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    OUTPUT_VARIABLE wh_VERSION)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# C_STANDARD
set (CMAKE_C_STANDARD 11)
set (CMAKE_CXX_STANDARD 14)

include_directories("include")
include_directories("include/libworm")
add_definitions(-O3 -g -pipe -flto -ffat-lto-objects) # -Wall -Wextra -Werror eliminado por problema con libressl

set(WARNINGFLAGS "-Wall -Wextra -Werror")

SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -flto -ldl -O3 -g -pipe")
SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -flto -ldl -O3 -g -pipe")

#include cmakeLimiter
include(dependencies/repos/cmakeLimiter/cmakeLimiter.cmake)

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

#Disable compilation
cmakelimiter_compile_executables(0)
cmakelimiter_compile_libraries(0)

#Netlib
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/repos/netlib" ${CMAKE_MODULE_PATH})

set(DESTDIR "/")
set(OPENSSLDIR "/etc/ssl")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/dependencies/repos/netlib/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/dependencies/repos/netlib/dependencies/libressl/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/dependencies/repos/netlib/src)
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/dependencies/repos/netlib dependencies/repos/netlib/build)

#HPTL
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/dependencies/repos/hptimelib/include)
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/dependencies/repos/hptimelib dependencies/repos/hptimelib/build)

#Re-enable compilation
cmakelimiter_compile_libraries(1)
cmakelimiter_compile_executables(1)

#LibWorm: Binary-Headers
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/structures.o/structures.o
    COMMAND mkdir -p "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/structures.o/"
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/tools/parseFile.bash ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/structures.o/parseFile.bash
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/structures.o/include
    #Include netlib
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/repos/netlib/include ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/structures.o/include/netlib    
    COMMAND cd "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/structures.o/" && ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/structures.o/parseFile.bash > ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/structures.o/structures.h
    COMMAND cd "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/structures.o/" && objcopy --input binary --output elf64-x86-64 --binary-architecture i386 structures.h structures.o
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/tools/parseFile.bash ${libworm_INCLUDE}
    VERBATIM
)
add_library(worm-structures STATIC ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/structures.o/structures.o ${libworm_INCLUDE})
SET_SOURCE_FILES_PROPERTIES(
  ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/structures.o/structures.o
  PROPERTIES
  EXTERNAL_OBJECT true
  GENERATED true
)
SET_TARGET_PROPERTIES(
  worm-structures
  PROPERTIES
  LINKER_LANGUAGE C 
)

#LibWorm
set(libworm_libdeps dl pthread)

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
target_link_libraries(libworm        worm-structures netlib          hptl           ${libworm_libdeps})
target_link_libraries(libworm-shared worm-structures netlib-shared   hptl-shared    ${libworm_libdeps})

# Aplicactions & Examples
unset(files CACHE)
file(GLOB files "mk/*.cmake")
list(SORT files)
foreach(file ${files})
    #MESSAGE( STATUS "Including file: " ${file})
    include(${file})
endforeach()