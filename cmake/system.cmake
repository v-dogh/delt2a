include_guard(GLOBAL)

include("${CMAKE_CURRENT_LIST_DIR}/target.cmake")

set(D2_MODULE_EXCLUDE_BASES "" CACHE STRING "Delta module abstract base types to exclude")
set(D2_MODULE_EXCLUDE_TYPES "" CACHE STRING "Delta concrete module types to exclude")
set(D2_MODULE_EXCLUDE_DIRS "" CACHE STRING "Delta module directory groups to exclude")
set(D2_MODULE_ROOTS "" CACHE STRING "Extra Delta module root groups: absolute root followed by dependency targets")

function(delta_register_module)
    set(options
        NO_SETUP
    )
    set(one_value_args
        TYPE
        BASE
        HEADER
        TARGET
        INCLUDE_ROOT
    )
    set(multi_value_args
        PLATFORMS
    )
    cmake_parse_arguments(
        DELTA_MODULE
        "${options}"
        "${one_value_args}"
        "${multi_value_args}"
        ${ARGN}
    )

    foreach(required_arg IN ITEMS TYPE BASE HEADER TARGET)
        if(NOT DELTA_MODULE_${required_arg})
            message(FATAL_ERROR "delta_register_module requires ${required_arg}")
        endif()
    endforeach()
    if(DELTA_MODULE_PLATFORMS)
        list(FIND DELTA_MODULE_PLATFORMS "${CMAKE_SYSTEM_NAME}" platform_index)
        if(platform_index EQUAL -1)
            return()
        endif()
    endif()
    if(D2_MODULE_EXCLUDE_BASES)
        list(FIND D2_MODULE_EXCLUDE_BASES "${DELTA_MODULE_BASE}" base_index)
        if(NOT base_index EQUAL -1)
            return()
        endif()
    endif()
    if(D2_MODULE_EXCLUDE_TYPES)
        list(FIND D2_MODULE_EXCLUDE_TYPES "${DELTA_MODULE_TYPE}" type_index)
        if(NOT type_index EQUAL -1)
            return()
        endif()
    endif()
    if(NOT TARGET "${DELTA_MODULE_TARGET}")
        message(FATAL_ERROR "delta_register_module: target '${DELTA_MODULE_TARGET}' does not exist")
    endif()
    if(IS_ABSOLUTE "${DELTA_MODULE_HEADER}")
        set(delta_module_header_file "${DELTA_MODULE_HEADER}")
    else()
        set(delta_module_header_file "${CMAKE_CURRENT_SOURCE_DIR}/${DELTA_MODULE_HEADER}")
    endif()
    if(NOT EXISTS "${delta_module_header_file}")
        message(FATAL_ERROR "delta_register_module: header '${DELTA_MODULE_HEADER}' does not exist")
    endif()
    if(DELTA_CURRENT_HEADER_ROOT)
        set(delta_header_root "${DELTA_CURRENT_HEADER_ROOT}")
    else()
        set(delta_header_root "${CMAKE_CURRENT_SOURCE_DIR}")
    endif()
    if(NOT IS_ABSOLUTE "${delta_header_root}")
        get_filename_component(delta_header_root
            "${delta_header_root}"
            ABSOLUTE
            BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}"
        )
    endif()

    file(RELATIVE_PATH delta_module_include_header
        "${delta_header_root}"
        "${delta_module_header_file}"
    )
    string(REPLACE "\\" "/" delta_module_include_header "${delta_module_include_header}")

    if(DELTA_MODULE_INCLUDE_ROOT)
        set(delta_module_include_root "${DELTA_MODULE_INCLUDE_ROOT}")
    else()
        set(delta_module_include_root "${CMAKE_CURRENT_SOURCE_DIR}")
    endif()
    if(NOT IS_ABSOLUTE "${delta_module_include_root}")
        get_filename_component(delta_module_include_root
            "${delta_module_include_root}"
            ABSOLUTE
            BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}"
        )
    endif()
    if(NOT DELTA_MODULE_NO_SETUP)
        delta_setup_target("${DELTA_MODULE_TARGET}"
            INCLUDE_ROOT "${delta_module_include_root}"
        )
    endif()
    if(DELTA_CURRENT_MODULE_DEPS)
        target_link_libraries("${DELTA_MODULE_TARGET}" PUBLIC
            ${DELTA_CURRENT_MODULE_DEPS}
        )
    endif()

    set_property(GLOBAL APPEND PROPERTY DELTA_REGISTERED_MODULE_TYPES
        "${DELTA_MODULE_TYPE}"
    )
    set_property(GLOBAL APPEND PROPERTY DELTA_REGISTERED_MODULE_BASES
        "${DELTA_MODULE_BASE}"
    )
    set_property(GLOBAL APPEND PROPERTY DELTA_REGISTERED_MODULE_HEADERS
        "${delta_module_include_header}"
    )
    set_property(GLOBAL APPEND PROPERTY DELTA_REGISTERED_MODULE_TARGETS
        "${DELTA_MODULE_TARGET}"
    )
endfunction()

function(delta_add_module_subdirectory source_dir)
    if(NOT IS_DIRECTORY "${source_dir}")
        message(FATAL_ERROR "delta_add_module_subdirectory: not a directory: ${source_dir}")
    endif()
    string(MD5 source_hash "${source_dir}")
    add_subdirectory(
        "${source_dir}"
        "${CMAKE_BINARY_DIR}/_delta_modules/${source_hash}"
    )
endfunction()

function(delta_is_module_group_excluded result group)
    set(excluded FALSE)
    if(D2_MODULE_EXCLUDE_DIRS)
        list(FIND D2_MODULE_EXCLUDE_DIRS "${group}" index)
        if(NOT index EQUAL -1)
            set(excluded TRUE)
        endif()
    endif()
    set("${result}" "${excluded}" PARENT_SCOPE)
endfunction()

function(delta_discover_modules_in_group root_dir group dir)
    if(EXISTS "${dir}/CMakeLists.txt")
        delta_add_module_subdirectory("${dir}")
        return()
    endif()
    file(GLOB children
        CONFIGURE_DEPENDS
        LIST_DIRECTORIES true
        "${dir}/*"
    )
    foreach(child IN LISTS children)
        if(IS_DIRECTORY "${child}")
            delta_discover_modules_in_group("${root_dir}" "${group}" "${child}")
        endif()
    endforeach()
endfunction()

function(delta_discover_module_root root_dir)
    if(NOT IS_DIRECTORY "${root_dir}")
        message(FATAL_ERROR "delta_discover_module_root: root does not exist: ${root_dir}")
    endif()
    file(GLOB children
        CONFIGURE_DEPENDS
        LIST_DIRECTORIES true
        "${root_dir}/*"
    )
    foreach(child IN LISTS children)
        if(NOT IS_DIRECTORY "${child}")
            continue()
        endif()
        get_filename_component(group "${child}" NAME)
        delta_is_module_group_excluded(group_excluded "${group}")
        if(group_excluded)
            message(STATUS "Delta: skipping module group '${group}' from root '${root_dir}'")
            continue()
        endif()
        delta_discover_modules_in_group("${root_dir}" "${group}" "${child}")
    endforeach()
endfunction()

function(delta_discover_modules)
    set(options)
    set(one_value_args
        TARGET
        HEADER_ROOT
    )
    set(multi_value_args
        ROOTS
        DEPS
        EXCLUDE_BASES
        EXCLUDE_TYPES
        EXCLUDE_DIRS
    )
    cmake_parse_arguments(
        DELTA_DISCOVER
        "${options}"
        "${one_value_args}"
        "${multi_value_args}"
        ${ARGN}
    )

    if(NOT DELTA_DISCOVER_TARGET)
        message(FATAL_ERROR "delta_discover_modules requires TARGET")
    endif()
    if(NOT TARGET "${DELTA_DISCOVER_TARGET}")
        message(FATAL_ERROR "delta_discover_modules: target '${DELTA_DISCOVER_TARGET}' does not exist")
    endif()
    if(NOT DELTA_DISCOVER_ROOTS)
        message(FATAL_ERROR "delta_discover_modules requires at least one ROOTS entry")
    endif()

    list(APPEND D2_MODULE_EXCLUDE_BASES ${DELTA_DISCOVER_EXCLUDE_BASES})
    list(APPEND D2_MODULE_EXCLUDE_TYPES ${DELTA_DISCOVER_EXCLUDE_TYPES})
    list(APPEND D2_MODULE_EXCLUDE_DIRS ${DELTA_DISCOVER_EXCLUDE_DIRS})
    set(DELTA_CURRENT_MODULE_DEPS ${DELTA_DISCOVER_DEPS})

    if(DELTA_DISCOVER_HEADER_ROOT)
        set(DELTA_CURRENT_HEADER_ROOT "${DELTA_DISCOVER_HEADER_ROOT}")
    else()
        set(DELTA_CURRENT_HEADER_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")
    endif()
    if(NOT IS_ABSOLUTE "${DELTA_CURRENT_HEADER_ROOT}")
        get_filename_component(DELTA_CURRENT_HEADER_ROOT
            "${DELTA_CURRENT_HEADER_ROOT}"
            ABSOLUTE
            BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}"
        )
    endif()
    foreach(root IN LISTS DELTA_DISCOVER_ROOTS)
        delta_discover_module_root("${root}")
    endforeach()

    get_property(delta_module_targets GLOBAL PROPERTY
        DELTA_REGISTERED_MODULE_TARGETS
    )

    if(delta_module_targets)
        list(REMOVE_DUPLICATES delta_module_targets)
        target_link_libraries("${DELTA_DISCOVER_TARGET}" PRIVATE
            ${delta_module_targets}
        )
    endif()
endfunction()
function(delta_discover_module_root_groups)
    set(options)
    set(one_value_args
        TARGET
    )
    set(multi_value_args
        DEPS
    )
    cmake_parse_arguments(
        DELTA_MODULE_ROOT_GROUPS
        "${options}"
        "${one_value_args}"
        "${multi_value_args}"
        ${ARGN}
    )

    if(NOT DELTA_MODULE_ROOT_GROUPS_TARGET)
        message(FATAL_ERROR "delta_discover_module_root_groups requires TARGET")
    endif()
    if(NOT TARGET "${DELTA_MODULE_ROOT_GROUPS_TARGET}")
        message(FATAL_ERROR "delta_discover_module_root_groups: target '${DELTA_MODULE_ROOT_GROUPS_TARGET}' does not exist")
    endif()
    if(NOT D2_MODULE_ROOTS)
        return()
    endif()

    set(delta_current_module_root "")
    set(delta_current_module_deps "")

    foreach(item IN LISTS D2_MODULE_ROOTS)
        if(IS_ABSOLUTE "${item}")
            if(delta_current_module_root)
                delta_discover_modules(
                    TARGET "${DELTA_MODULE_ROOT_GROUPS_TARGET}"
                    ROOTS "${delta_current_module_root}"
                    DEPS
                        ${DELTA_MODULE_ROOT_GROUPS_DEPS}
                        ${delta_current_module_deps}
                    EXCLUDE_BASES ${D2_MODULE_EXCLUDE_BASES}
                    EXCLUDE_TYPES ${D2_MODULE_EXCLUDE_TYPES}
                    EXCLUDE_DIRS ${D2_MODULE_EXCLUDE_DIRS}
                )
            endif()

            if(NOT IS_DIRECTORY "${item}")
                message(FATAL_ERROR "delta_discover_module_root_groups: root does not exist: ${item}")
            endif()

            set(delta_current_module_root "${item}")
            set(delta_current_module_deps "")
        else()
            if(NOT delta_current_module_root)
                message(FATAL_ERROR "delta_discover_module_root_groups: dependency '${item}' appears before any absolute module root")
            endif()
            if(NOT TARGET "${item}")
                message(FATAL_ERROR "delta_discover_module_root_groups: dependency '${item}' is not a target")
            endif()

            list(APPEND delta_current_module_deps "${item}")
        endif()
    endforeach()

    if(delta_current_module_root)
        delta_discover_modules(
            TARGET "${DELTA_MODULE_ROOT_GROUPS_TARGET}"
            ROOTS "${delta_current_module_root}"
            DEPS
                ${DELTA_MODULE_ROOT_GROUPS_DEPS}
                ${delta_current_module_deps}
            EXCLUDE_BASES ${D2_MODULE_EXCLUDE_BASES}
            EXCLUDE_TYPES ${D2_MODULE_EXCLUDE_TYPES}
            EXCLUDE_DIRS ${D2_MODULE_EXCLUDE_DIRS}
        )
    endif()
endfunction()

function(delta_generate_module_registry)
    set(options)
    set(one_value_args
        TARGET
        OUTPUT
        INCLUDE_ROOT
        TEMPLATE
    )
    set(multi_value_args)
    cmake_parse_arguments(
        DELTA_REGISTRY
        "${options}"
        "${one_value_args}"
        "${multi_value_args}"
        ${ARGN}
    )

    if(NOT DELTA_REGISTRY_OUTPUT)
        message(FATAL_ERROR "delta_generate_module_registry requires OUTPUT")
    endif()
    if(NOT DELTA_REGISTRY_TEMPLATE)
        set(DELTA_REGISTRY_TEMPLATE "${CMAKE_CURRENT_LIST_DIR}/cmake/d2_platform.hpp.in")
    endif()
    if(NOT EXISTS "${DELTA_REGISTRY_TEMPLATE}")
        message(FATAL_ERROR "delta_generate_module_registry: template does not exist: ${DELTA_REGISTRY_TEMPLATE}")
    endif()
    if(DELTA_REGISTRY_TARGET)
        if(NOT TARGET "${DELTA_REGISTRY_TARGET}")
            message(FATAL_ERROR "delta_generate_module_registry: target '${DELTA_REGISTRY_TARGET}' does not exist")
        endif()
    endif()
    if(NOT DELTA_REGISTRY_INCLUDE_ROOT)
        get_filename_component(DELTA_REGISTRY_INCLUDE_ROOT
            "${DELTA_REGISTRY_OUTPUT}"
            DIRECTORY
        )
    endif()
    if(NOT IS_ABSOLUTE "${DELTA_REGISTRY_INCLUDE_ROOT}")
        get_filename_component(DELTA_REGISTRY_INCLUDE_ROOT
            "${DELTA_REGISTRY_INCLUDE_ROOT}"
            ABSOLUTE
            BASE_DIR "${CMAKE_CURRENT_BINARY_DIR}"
        )
    endif()

    get_filename_component(delta_registry_output_dir
        "${DELTA_REGISTRY_OUTPUT}"
        DIRECTORY
    )

    file(MAKE_DIRECTORY "${delta_registry_output_dir}")
    get_property(delta_module_headers GLOBAL PROPERTY
        DELTA_REGISTERED_MODULE_HEADERS
    )
    get_property(delta_module_types GLOBAL PROPERTY
        DELTA_REGISTERED_MODULE_TYPES
    )

    if(delta_module_headers)
        list(REMOVE_DUPLICATES delta_module_headers)
    endif()

    if(delta_module_types)
        list(REMOVE_DUPLICATES delta_module_types)
    endif()

    set(D2_GENERATED_MODULE_INCLUDES "")
    foreach(header IN LISTS delta_module_headers)
        string(APPEND D2_GENERATED_MODULE_INCLUDES
            "#include <${header}>\n"
        )
    endforeach()
    set(D2_GENERATED_MODULE_TYPES "")
    if(delta_module_types)
        list(JOIN delta_module_types ",\n        " D2_GENERATED_MODULE_TYPES)
        set(D2_GENERATED_MODULE_TYPES "        ${D2_GENERATED_MODULE_TYPES}")
    endif()

    configure_file(
        "${DELTA_REGISTRY_TEMPLATE}"
        "${DELTA_REGISTRY_OUTPUT}"
        @ONLY
    )

    if(DELTA_REGISTRY_TARGET)
        target_include_directories("${DELTA_REGISTRY_TARGET}" PUBLIC
            "${DELTA_REGISTRY_INCLUDE_ROOT}"
        )
    endif()
endfunction()
