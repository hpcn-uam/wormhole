################################
# WORMHOLE CONFIG
#################################

option(EINSTEIN_DEBUG "Enable Einstein debug text" ON)
option(WORMS_DEBUG "Enable Einstein debug text" ON)

#################################
# CONFIG FILES
#################################
#Config file
#configure_file (
#  "${PROJECT_SOURCE_DIR}/include/config.hpp.in"
#  "${PROJECT_BINARY_DIR}/include/config.hpp"
#  )
configure_file (
  "${PROJECT_SOURCE_DIR}/include/wh_config.h.in"
  "${PROJECT_SOURCE_DIR}/include/wh_config.h"
  )


