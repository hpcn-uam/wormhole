set(app_name "pcapReader")

# Sources
file(GLOB app_src
    "src/examples/${app_name}.c"
)

file(GLOB app_inc
)

add_wormhole_application("${app_name}" "${ex_runs_dir}/${app_name}.sh" "${app_src}" "${app_inc}" "")
