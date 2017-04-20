

unset(files CACHE)
file(GLOB files "mk/examples/*.mk")
foreach(file ${files})
    MESSAGE( STATUS "Including file: " ${file})
    include(${file})
endforeach()