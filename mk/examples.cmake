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

            COMMAND ${CMAKE_COMMAND} -E make_directory  ${tmp_dir}
            COMMAND ${CMAKE_COMMAND} -E make_directory  ${ex_out_dir}

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

        # Include include_directories
        FOREACH(cur_dir ${ARGN})
            include_directories("${cur_dir}")
        ENDFOREACH(cur_dir)

        # Target to create the tar
        add_custom_target(
            "${appName}-tar"
            DEPENDS "${ex_out_dir}/${appName}.tgz"
            )
        add_dependencies("examples-tar" "${appName}-tar")
        unset(tmp_dir)
    endfunction(add_wormhole_application)

    if (WH_REMOTE_EXAMPLES)
        find_package(Git REQUIRED)
        
        function(add_wormhole_remote_application giturl gittag appName app_runscript app_sources app_includes app_libs)
            MESSAGE (STATUS "Downloading example app: " ${appName})
            set(clone_dir "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/appsANDexamples/sources/${appName}")

            if(EXISTS ${clone_dir})
                execute_process (COMMAND ${GIT_EXECUTABLE} -C "${clone_dir}" pull RESULT_VARIABLE gitstatus) 
                execute_process (COMMAND ${GIT_EXECUTABLE} -C "${clone_dir}" checkout -f ${gittag} RESULT_VARIABLE gitstatus) 
            else()
                execute_process (COMMAND ${CMAKE_COMMAND} -E make_directory  ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/appsANDexamples/sources/)
                execute_process (COMMAND ${GIT_EXECUTABLE} clone --recursive -j8 -b "${gittag}" "${giturl}" "${clone_dir}" RESULT_VARIABLE gitstatus) 
            endif()
            
            if(NOT ${gitstatus})
                prepend_string(concat_app_sources "${clone_dir}/" ${app_sources})
                prepend_string(concat_app_includes "${clone_dir}/" ${app_includes})
                prepend_string(final_app_includeDir "${clone_dir}/" ${ARGN})

                file(GLOB final_app_sources ${concat_app_sources})
                file(GLOB final_app_includes ${concat_app_includes})

                add_wormhole_application(${appName} ${app_runscript} "${final_app_sources}" "${final_app_includes}" ${app_libs} ${final_app_includeDir})
            else()
                MESSAGE(WARNING "Can't add nor download example " ${appName} ". Probably, you dont have enough permissions to download it")            
            endif()

        endfunction(add_wormhole_remote_application)    
    else()
        function(add_wormhole_remote_application giturl gittag appName app_runscript app_sources app_includes app_libs)
            MESSAGE(STATUS "Can't add example " ${appName} " as remote applications are disabled")
        endfunction(add_wormhole_remote_application)  
    endif()  

    # Include example-config files from examples dir
    unset(files CACHE)
    file(GLOB files "mk/examples/*.cmake")
    foreach(file ${files})
        #MESSAGE( STATUS "Including file: " ${file})
        include(${file})
    endforeach()  
endif()