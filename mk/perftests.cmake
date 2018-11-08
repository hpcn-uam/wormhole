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

set(perf_dir "${CMAKE_SOURCE_DIR}/tests/wormhole/")
set(ex_out_dir "${CMAKE_BINARY_DIR}/perftest/")
set(ex_out_bin_dir "${CMAKE_BINARY_DIR}/bin/perftest")

if (WH_PERFTESTS)

    add_custom_target("perftests-tar")
    add_custom_target("perftests" ALL DEPENDS "perftests-tar" )

    function(add_wormhole_application appName app_runscript app_sources app_includes app_libs)
        set(tmp_dir "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/appsANDperftests/${appName}")
        MESSAGE (STATUS "Adding perftest app: " ${appName})
        # Command to create the executable

        # Executable
        add_executable(${appName} ${app_sources} ${app_includes})
        # Dependencies
        target_link_libraries(${appName} libworm ${app_libs})
        # Location
        set_target_properties(${appName} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${ex_out_bin_dir})

        if(WH_SSLCERTS)
            set(certificatesDep "certificates")
        endif(WH_SSLCERTS)

        if(APP_EXTRADIRS)
            foreach (extradir IN ITEMS ${APP_EXTRADIRS})
                set(APP_EXTRACOMMANDS ${APP_EXTRACOMMANDS} COMMAND ${CMAKE_COMMAND} -E copy_directory ${extradir} ${tmp_dir}/extra)
            endforeach(extradir)
            unset(APP_EXTRADIRS PARENT_SCOPE)
        endif(APP_EXTRADIRS)

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

            # Extra Directories 
            ${APP_EXTRACOMMANDS}

            COMMAND tar -czf "${ex_out_dir}/${appName}.tgz" -C "${tmp_dir}/.." "${appName}"
            DEPENDS ${appName} libworm ${certificatesDep} ${app_runscript}  #${app_libs}
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
        add_dependencies("perftests-tar" "${appName}-tar")
        unset(tmp_dir)
    endfunction(add_wormhole_application)

    # Include perftest-config files from perftests dir
    unset(files CACHE)
    file(GLOB files "${perf_dir}/*/*.cmake")
    foreach(file ${files})
        #MESSAGE( STATUS "Including file: " ${file})
        include(${file})
    endforeach()  
endif()