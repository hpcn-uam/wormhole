# Includes
include_directories("dependencies/repos/linenoise/")

# Sources
file(GLOB zeus_SRC
    "src/zeus/*.c"
    "src/zeus/*.cpp"
    "dependencies/repos/linenoise/linenoise.c"
)

file(GLOB zeus_INCLUDE
    "src/zeus/*.h"
    "src/zeus/*.hpp"
    "dependencies/repos/linenoise/linenoise.h"
)

# Executable
add_executable(zeus ${zeus_SRC} ${zeus_INCLUDE})

# Dependencies
target_link_libraries(zeus libworm)