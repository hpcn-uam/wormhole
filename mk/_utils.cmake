FUNCTION(prepend_string var prefix)
    SET(listVar "")
    FOREACH(f ${ARGN})
    LIST(APPEND listVar "${prefix}/${f}")
    ENDFOREACH(f)
    SET(${var} "${listVar}" PARENT_SCOPE)
ENDFUNCTION(prepend_string)