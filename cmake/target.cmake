include_guard(GLOBAL)

function(delta_setup_target target)
    cmake_parse_arguments(DELTA_SETUP "" "INCLUDE_ROOT" "" ${ARGN})
    if (NOT DELTA_SETUP_INCLUDE_ROOT)
        set(DELTA_SETUP_INCLUDE_ROOT "${DeltaAscii_SOURCE_DIR}")
    endif()
    target_compile_features(${target} PUBLIC cxx_std_20)
    target_include_directories(${target} PUBLIC
        $<BUILD_INTERFACE:${DELTA_SETUP_INCLUDE_ROOT}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/delt2a>
    )
    if (NOT D2_SANITIZER STREQUAL "none")
        target_link_libraries(${target} PUBLIC DeltaSan)
    endif()
    if (D2_UNICODE_MODE)
        target_compile_definitions(${target} PUBLIC D2_LOCALE_MODE=32)
        target_link_libraries(${target} PUBLIC uni-algo::uni-algo)
    else()
        target_compile_definitions(${target} PUBLIC D2_LOCALE_MODE=8)
    endif()
    target_compile_definitions(${target} PUBLIC
        ASCII=8
        UNICODE=32
    )
    if (D2_SECURITY_WATCH)
        target_compile_definitions(${target} PUBLIC D2_SECURITY_WATCH)
    endif()
    if (D2_SECURITY_ASSERT)
        target_compile_definitions(${target} PUBLIC D2_SECURITY_ASSERT)
    endif()
    if (D2_SECURITY_LOG)
        target_compile_definitions(${target} PUBLIC D2_SECURITY_LOG)
    endif()
    if (D2_STRICT_MODE)
        target_compile_definitions(${target} PUBLIC
            D2_COMPATIBILITY_MODE
            STRICT
        )
    endif()
    if (D2_EXCLUDE_OS_CORE)
        target_compile_definitions(${target} PUBLIC D2_EXCLUDE_OS_CORE)
    endif()
    if (D2_EXCLUDE_OS_EXT)
        target_compile_definitions(${target} PUBLIC D2_EXCLUDE_OS_EXT)
    endif()
endfunction()
