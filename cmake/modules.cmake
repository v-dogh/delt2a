include_guard(GLOBAL)

function(delta_register_module)
    set(options)
    set(one_value_args
        TYPE
        BASE
        HEADER
        TARGET
    )
    set(multi_value_args
        PLATFORMS
    )
    cmake_parse_arguments(
        MODULE
        "${options}"
        "${one_value_args}"
        "${multi_value_args}"
        ${ARGN}
    )
    foreach(required_arg IN ITEMS TYPE BASE HEADER TARGET)
        if(NOT MODULE_${required_arg})
            message(FATAL_ERROR "delta_register_module requires ${required_arg}")
        endif()
    endforeach()
    if(NOT TARGET "${MODULE_TARGET}")
        message(FATAL_ERROR "delta_register_module: TARGET '${MODULE_TARGET}' does not exist")
    endif()
    delta_setup_target(${MODULE_TARGET})
    if(MODULE_PLATFORMS)
        list(FIND MODULE_PLATFORMS "${CMAKE_SYSTEM_NAME}" platform_index)
        if(platform_index EQUAL -1)
            return()
        endif()
    endif()
    if(MODULE_EXCLUDED_BASES)
        list(FIND MODULE_EXCLUDED_BASES "${MODULE_BASE}" base_index)
        if(NOT base_index EQUAL -1)
            return()
        endif()
    endif()
    if(MODULE_EXCLUDED_TYPES)
        list(FIND MODULE_EXCLUDED_TYPES "${MODULE_TYPE}" type_index)
        if(NOT type_index EQUAL -1)
            return()
        endif()
    endif()
    set_property(GLOBAL APPEND PROPERTY DELTA_REGISTERED_MODULE_TYPES "${MODULE_TYPE}")
    set_property(GLOBAL APPEND PROPERTY DELTA_REGISTERED_MODULE_BASES "${MODULE_BASE}")
    set_property(GLOBAL APPEND PROPERTY DELTA_REGISTERED_MODULE_HEADERS "${MODULE_HEADER}")
    set_property(GLOBAL APPEND PROPERTY DELTA_REGISTERED_MODULE_TARGETS "${MODULE_TARGET}")
endfunction()
