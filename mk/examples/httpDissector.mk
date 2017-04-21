set(app_name "httpDissector")

# Sources
file(GLOB app_src
    "dependencies/repos/httpDissector/httpDissector.c"
    "dependencies/repos/httpDissector/hash_table.c"
    "dependencies/repos/httpDissector/collision_list_pool.c"
    "dependencies/repos/httpDissector/counters.c"
    "dependencies/repos/httpDissector/http_event_pool.c"
    "dependencies/repos/httpDissector/process_packet.c"
    "dependencies/repos/httpDissector/alist.c"
    "dependencies/repos/httpDissector/tools.c"
    "dependencies/repos/httpDissector/http.c"
    "dependencies/repos/httpDissector/args_parse.c"
    "dependencies/repos/httpDissector/hpcap_utils.c"
    "dependencies/repos/httpDissector/worm_pcap_bridge.c"
)

file(GLOB app_inc
    "dependencies/repos/httpDissector/*.h"
    "dependencies/repos/httpDissector/include/*.h"
)

include_directories("dependencies/repos/httpDissector/include")

add_wormhole_application("${app_name}" "${ex_runs_dir}/${app_name}.sh" "${app_src}" "${app_inc}" "pcap")
