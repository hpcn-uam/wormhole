if (WH_APPS)
    unset(files CACHE)
    file(GLOB files "mk/apps/*.cmake")
    foreach(file ${files})
        #MESSAGE( STATUS "Including file: " ${file})
        include(${file})
    endforeach()
endif()