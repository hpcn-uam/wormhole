set(app_name "rtt_server")
set(app_dir  "${perf_dir}/rtt")

# Sources
file(GLOB app_src
    "${app_dir}/${app_name}.c"
)

file(GLOB app_inc
)

add_wormhole_application("perf_${app_name}" "${app_dir}/${app_name}.sh" "${app_src}" "${app_inc}" "")
