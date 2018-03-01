################################
# WORMHOLE MODULES
#################################

option(WH_APPS "Enable the compilation of WH apps, including einstein" ON)
option(WH_EXAMPLES "Enable the compilation of the examples" ON)
option(WH_REMOTE_EXAMPLES "Enable the compilation of the remote examples" ON)
option(WH_SSLCERTS "Enable the autogeneration of certs" OFF)

#################################
# WORMHOLE CONFIG
#################################

option(WH_STATISTICS "Enable transfer statistics on each connection" ON)
option(WH_SSL "Enable SSL support" OFF)

#################################
# WORMHOLE DEVEL-CONFIG
#################################

option(EINSTEIN_DEBUG "Enable Einstein debug info" OFF)
option(LIBWORM_DEBUG "Enable libworm debug info" OFF)
option(LIBWORM_ROUTE_DEBUG "Enable libworm-routing debug info" OFF)

option(WH_PREFETCHING "Enable prefetching when possible" ON)
