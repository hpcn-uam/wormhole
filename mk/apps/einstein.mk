# Includes
include_directories("dependencies/repos/linenoise/")

# Sources
file(GLOB einstein_SRC
    "src/einstein/*.c"
    "src/einstein/*.cpp"
    "dependencies/repos/linenoise/linenoise.c"
)

file(GLOB einstein_INCLUDE
    "src/einstein/*.h"
    "src/einstein/*.hpp"
    "dependencies/repos/linenoise/linenoise.h"
)

# Executable
add_executable(einstein ${einstein_SRC} ${einstein_INCLUDE})

# Dependencies
target_link_libraries(einstein libworm)