set(ex_runs_dir "${CMAKE_SOURCE_DIR}/src/examples/runscripts/")
set(ex_out_dir "${CMAKE_BINARY_DIR}/examples/")
set(ex_out_bin_dir "${CMAKE_BINARY_DIR}/bin/examples")

if (WH_EXAMPLES)

    add_custom_target("examples-tar")
    add_custom_target("examples" ALL DEPENDS "examples-tar" )

    function(add_wormhole_application appName app_runscript app_sources app_includes app_libs)
        set(tmp_dir "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/appsANDexamples/${appName}")
        MESSAGE (STATUS "Adding example app: " ${appName})
        # Command to create the executable

        # Executable
        add_executable(${appName} ${app_sources} ${app_includes})
        # Dependencies
        target_link_libraries(${appName} libworm ${app_libs})
        # Location
        set_target_properties(${appName} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${ex_out_bin_dir})


        # Command to create the tar
        add_custom_command(
            OUTPUT "${ex_out_dir}/${appName}.tgz"

            COMMAND mkdir -p ${tmp_dir}
            COMMAND mkdir -p ${ex_out_dir}

            COMMAND ${CMAKE_COMMAND} -E copy_directory  ${CMAKE_CURRENT_BINARY_DIR}/lib ${tmp_dir}/lib
            COMMAND ${CMAKE_COMMAND} -E copy            ${app_runscript} ${tmp_dir}/run.sh
            COMMAND ${CMAKE_COMMAND} -E copy            ${ex_out_bin_dir}/${appName} ${tmp_dir}/${appName}

            # Certificates
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${certs_dir} ${tmp_dir}/certs
            COMMAND ${CMAKE_COMMAND} -E remove ${tmp_dir}/certs/prv/ca.key.pem

            COMMAND tar -czf "${ex_out_dir}/${appName}.tgz" -C "${tmp_dir}/.." "${appName}"
            DEPENDS ${appName} libworm certificates ${app_runscript}  #${app_libs}
            VERBATIM
        )
        # Target to create the tar
        add_custom_target(
            "${appName}-tar"
            DEPENDS "${ex_out_dir}/${appName}.tgz"
            )
        add_dependencies("examples-tar" "${appName}-tar")
        unset(tmp_dir)
    endfunction(add_wormhole_application)

    unset(files CACHE)
    file(GLOB files "mk/examples/*.cmake")
    foreach(file ${files})
        #MESSAGE( STATUS "Including file: " ${file})
        include(${file})
    endforeach()
endif()