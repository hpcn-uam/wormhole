# Copyright (c) 2015-2018 Rafael Leira
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation
# files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy,
# modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
# Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
# COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
# OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

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