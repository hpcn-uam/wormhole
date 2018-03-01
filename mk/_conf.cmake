if(NOT _confcmake)
set(_confcmake PROCESSED)
# Configuration check 

set(NETLIB_SSL ${WH_SSL} CACHE BOOL "FORCED NETLIB_SSL==WH_SSL" FORCE)

if(NOT WH_SSL)
    message(WARNING "SSL DISABLE NOT FULLY--SUPPORTED")
    if(WH_SSLCERTS)
        message(WARNING "SSL Certs enabled. Should not when SSL is disabled. Forcing disable.")
        set(WH_SSLCERTS OFF CACHE BOOL "FORCED WH_SSLCERTS==WH_SSL" FORCE)
    endif(WH_SSLCERTS)
endif(NOT WH_SSL)

# Write confs
configure_file (
  "${PROJECT_SOURCE_DIR}/include/wh_config.h.in"
  "${PROJECT_SOURCE_DIR}/include/wh_config.h"
  )
configure_file (
  "${PROJECT_SOURCE_DIR}/include/wh_version.h.in"
  "${PROJECT_SOURCE_DIR}/include/wh_version.h"
  )

endif(NOT _confcmake)