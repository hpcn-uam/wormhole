################################
# WORMHOLE MODULES
#################################

option(WH_APPS             "Enable the compilation of WH apps, including zeus" ON)
option(WH_EXAMPLES         "Enable the compilation of the examples" ON)
option(WH_REMOTE_EXAMPLES  "Enable the compilation of the remote (and propieatary) examples" ON)
option(WH_PERFTESTS        "Enable the compilation of the perftests" ON)
option(WH_SSLCERTS         "Enable the autogeneration of certs" OFF)

#################################
# WORMHOLE CONFIG
#################################

option(WH_STATISTICS       "Enable statistic tracing on each connection" ON)
option(WH_SSL              "Enable SSL support" OFF)

#################################
# WORMHOLE DEVEL-CONFIG
#################################

option(ZEUS_DEBUG          "Enable Zeus debug info" OFF)
option(LIBWORM_DEBUG       "Enable libworm debug info" OFF)
option(LIBWORM_ROUTE_DEBUG "Enable libworm-routing debug info" OFF)

option(WH_PREFETCHING      "Enable prefetching when possible" ON)
