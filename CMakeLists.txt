cmake_minimum_required(VERSION 3.21)
project(DeltaAscii LANGUAGES CXX VERSION 0.1.0)

include(GNUInstallDirs)
include(FetchContent)
include(cmake/target.cmake)
include(cmake/system.cmake)

###########
# OPTIONS #
###########

option(D2_EXCLUDE_LIBRARY_EXTRAS    "Disable extras module"        OFF)
option(D2_EXCLUDE_LIBRARY_ELEMENTS  "Disable elements module"      OFF)
option(D2_SECURITY_WATCH            "Enable runtime debug checks"  ON)
option(D2_SECURITY_LOG              "Enable runtime logging"       ON)
option(D2_SECURITY_ASSERT           "Enable debug assertions"      ON)
option(D2_UNICODE_MODE              "Enable unicode mode"          ON)
option(D2_ENABLE_INSTALL            "Enable install for cmake"     OFF)
option(D2_TEST                      "Enable building of examples"  OFF)

set(D2_SANITIZER "none" CACHE STRING "Sanitizer: none, address, memory, thread, undefined")
set_property(CACHE D2_SANITIZER PROPERTY STRINGS
    none
    address
    memory
    thread
    undefined
)

option(D2_SANITIZER_RECOVER         "Enable sanitizer recovery where supported"      ON)
option(D2_MSAN_TRACK_ORIGINS        "Enable MSAN origin tracking"                    ON)
option(D2_ASAN_USE_AFTER_SCOPE      "Enable ASAN use-after-scope checks"             ON)

######################
# SANITIZER SETTINGS #
######################

set(D2_SANITIZER_COMPILE_FLAGS "")
set(D2_SANITIZER_LINK_FLAGS    "")
set(D2_SANITIZER_DEFINITIONS   "")

if (NOT D2_SANITIZER STREQUAL "none")
    if (NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        message(FATAL_ERROR "Sanitizers require Clang or GCC")
    endif()
    list(APPEND D2_SANITIZER_COMPILE_FLAGS
        -g
        -fno-omit-frame-pointer
        -fno-optimize-sibling-calls
    )
    list(APPEND D2_SANITIZER_DEFINITIONS D2_SANITIZER_ENABLED)
    if (D2_SANITIZER STREQUAL "address")
        list(APPEND D2_SANITIZER_COMPILE_FLAGS -fsanitize=address,undefined)
        list(APPEND D2_SANITIZER_LINK_FLAGS    -fsanitize=address,undefined)
        list(APPEND D2_SANITIZER_DEFINITIONS
            D2_SANITIZER_ADDRESS
            D2_SANITIZER_UNDEFINED
        )
        if (D2_ASAN_USE_AFTER_SCOPE)
            list(APPEND D2_SANITIZER_COMPILE_FLAGS -fsanitize-address-use-after-scope)
        endif()
        if (D2_SANITIZER_RECOVER)
            list(APPEND D2_SANITIZER_COMPILE_FLAGS -fsanitize-recover=undefined)
        endif()
    elseif (D2_SANITIZER STREQUAL "undefined")
        list(APPEND D2_SANITIZER_COMPILE_FLAGS -fsanitize=undefined)
        list(APPEND D2_SANITIZER_LINK_FLAGS    -fsanitize=undefined)
        list(APPEND D2_SANITIZER_DEFINITIONS D2_SANITIZER_UNDEFINED)
        if (D2_SANITIZER_RECOVER)
            list(APPEND D2_SANITIZER_COMPILE_FLAGS -fsanitize-recover=undefined)
        endif()
    elseif (D2_SANITIZER STREQUAL "memory")
        if (NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            message(FATAL_ERROR "MSAN requires Clang")
        endif()
        list(APPEND D2_SANITIZER_COMPILE_FLAGS -fsanitize=memory)
        list(APPEND D2_SANITIZER_LINK_FLAGS    -fsanitize=memory)
        list(APPEND D2_SANITIZER_DEFINITIONS D2_SANITIZER_MEMORY)
        if (D2_MSAN_TRACK_ORIGINS)
            list(APPEND D2_SANITIZER_COMPILE_FLAGS -fsanitize-memory-track-origins=2)
        endif()
        if (D2_SANITIZER_RECOVER)
            list(APPEND D2_SANITIZER_COMPILE_FLAGS -fsanitize-recover=memory)
        endif()
    elseif (D2_SANITIZER STREQUAL "thread")
        list(APPEND D2_SANITIZER_COMPILE_FLAGS -fsanitize=thread)
        list(APPEND D2_SANITIZER_LINK_FLAGS    -fsanitize=thread)
        list(APPEND D2_SANITIZER_DEFINITIONS D2_SANITIZER_THREAD)
    else()
        message(FATAL_ERROR "Unknown sanitizer: ${D2_SANITIZER}")
    endif()
    message(STATUS "Sanitizer: ${D2_SANITIZER}")
endif()

add_library(DeltaSan INTERFACE)
if (NOT D2_SANITIZER STREQUAL "none")
    target_compile_options(DeltaSan     INTERFACE ${D2_SANITIZER_COMPILE_FLAGS})
    target_link_options(DeltaSan        INTERFACE ${D2_SANITIZER_LINK_FLAGS})
    target_compile_definitions(DeltaSan INTERFACE ${D2_SANITIZER_DEFINITIONS})
    add_compile_options(${D2_SANITIZER_COMPILE_FLAGS})
    add_link_options(${D2_SANITIZER_LINK_FLAGS})
endif()

#########################
# EXTERNAL DEPENDENCIES #
#########################

if (NOT TARGET absl::base)
    if (NOT D2_SANITIZER STREQUAL "none")
        FetchContent_Declare(
            absl
            GIT_REPOSITORY https://github.com/abseil/abseil-cpp.git
            GIT_TAG        master
        )
        set(ABSL_PROPAGATE_CXX_STD ON  CACHE BOOL "" FORCE)
        set(ABSL_ENABLE_INSTALL    OFF CACHE BOOL "" FORCE)
        set(ABSL_BUILD_TESTING     OFF CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(absl)
    else()
        find_package(absl REQUIRED)
    endif()
endif()
if (D2_UNICODE_MODE)
    find_package(uni-algo REQUIRED)
endif()
find_package(ada REQUIRED)

#############
# LIBRARIES #
#############

# Export stuff
if (NOT D2_SANITIZER STREQUAL "none" OR NOT D2_ENABLE_INSTALL)
    set(TPMP_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
    set(LOGS_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
endif()

# Scheduler
add_subdirectory(mt)

# Logs
add_subdirectory(logs)

# LSX library
add_library(DeltaLSX STATIC
    lsx/d2_solver.hpp
    lsx/d2_solver.cpp
    lsx/d2_solver_types.hpp
    lsx/d2_solver_types.cpp
    lsx/d2_solver_ops.hpp
    lsx/d2_solver_ops.cpp
)
target_link_libraries(DeltaLSX PUBLIC
    absl::base
    absl::flat_hash_map
)
delta_setup_target(DeltaLSX)

# Core library
add_library(DeltaCore STATIC
    # Core
    d2_std.hpp
    d2_exceptions.hpp
    d2_styles.hpp
    d2_styles_base.hpp
    d2_theme.hpp
    d2_theme.cpp
    d2_model.hpp
    d2_model.cpp
    d2_meta.hpp
    # Platform
    d2_locale.hpp
    d2_chardef.hpp
    d2_platform_string.hpp
    d2_platform_string.cpp
    d2_fragmented_buffer.hpp
    d2_fragmented_buffer.cpp
    # Pixel
    d2_pixel.hpp
    d2_colors.hpp
    d2_interpolator.cpp
    d2_interpolator.hpp
    d2_pixel.cpp
    d2_extended_page.hpp
    d2_extended_page.cpp
    # Tree
    d2_element_units.hpp
    d2_tree_state.hpp
    d2_tree_state.cpp
    d2_tree_element_frwd.hpp
    d2_tree_element.hpp
    d2_tree_parent.hpp
    d2_tree.hpp
    d2_tree_element.cpp
    d2_tree_parent.cpp
    d2_tree_construct.hpp
    # Context
    d2_main_worker.hpp
    d2_main_worker.cpp
    d2_io_handler_frwd.hpp
    d2_io_handler.hpp
    d2_io_handler.cpp
    d2_signal_handler.hpp
    d2_signal_handler.cpp
    d2_screen_comps.cpp
    d2_screen_comps.hpp
    d2_screen.hpp
    d2_screen.cpp
    d2_input_base.hpp
    d2_input_base.cpp
    # Mods
    d2_module.hpp
    d2_module.cpp
    mods/d2_core.hpp
    mods/d2_core.cpp
    mods/d2_ext.hpp
    mods/d2_ext.cpp
    mods/d2_audio.hpp
    mods/d2_audio.cpp
    mods/d2_binder.hpp
    mods/d2_binder.cpp
    mods/d2_http.hpp
    mods/d2_http.cpp
    mods/d2_ws.hpp
    mods/d2_ws.cpp
    mods/d2_session.hpp
    mods/d2_session.cpp
)
target_link_libraries(DeltaCore PUBLIC
    TPMP
    logd
    DeltaLSX
    ada::ada
    absl::base
    absl::flat_hash_map
)
delta_setup_target(DeltaCore)

# Standard elements library
if (NOT D2_EXCLUDE_LIBRARY_ELEMENTS)
    add_library(DeltaElements STATIC
        elements/d2_std.hpp
        elements/d2_element_utils.hpp
        elements/d2_element_utils.cpp
        # Boxes
        elements/d2_box.hpp
        elements/d2_box.cpp
        elements/d2_flow_box.hpp
        elements/d2_flow_box.cpp
        elements/d2_draggable_box.hpp
        elements/d2_draggable_box.cpp
        elements/d2_scrollbox.hpp
        elements/d2_scrollbox.cpp
        elements/d2_demand_scrollbox.hpp
        elements/d2_demand_scrollbox.cpp
        # Text
        elements/d2_text.hpp
        elements/d2_text.cpp
        elements/d2_input.hpp
        elements/d2_input.cpp
        elements/d2_multiline_input.hpp
        elements/d2_multiline_input.cpp
        # Interactive
        elements/d2_button.hpp
        elements/d2_button.cpp
        elements/d2_slider.hpp
        elements/d2_slider.cpp
        elements/d2_dropdown.hpp
        elements/d2_dropdown.cpp
        elements/d2_switch.hpp
        elements/d2_switch.cpp
        elements/d2_checkbox.hpp
        elements/d2_checkbox.cpp
        # Other
        elements/d2_image.hpp
        elements/d2_image.cpp
        elements/d2_matrix.hpp
        elements/d2_matrix.cpp
        elements/d2_graph.hpp
        elements/d2_graph.cpp
        elements/d2_separator.hpp
        elements/d2_separator.cpp
    )
    target_link_libraries(DeltaElements PUBLIC DeltaCore)
    delta_setup_target(DeltaElements)
endif()

# Some funtime extras
if (NOT D2_EXCLUDE_LIBRARY_EXTRAS)
    add_library(DeltaExtras STATIC
        # Presets
        extras/d2_default_theme.hpp
        extras/d2_default_theme.cpp
        extras/d2_default_styles.hpp
        # Widgets
        # extras/d2_loading_indicators.hpp
        # extras/d2_file_explorer.hpp
        # extras/d2_file_explorer.cpp
        # extras/d2_exit_prompt.hpp
        # extras/d2_exit_prompt.cpp
        # extras/d2_color_picker.hpp
        # extras/d2_color_picker.cpp
        # extras/d2_theme_selector.hpp
        # extras/d2_theme_selector.cpp
        # extras/d2_audio_device_selector.hpp
        # extras/d2_audio_device_selector.cpp
        # Other stuff 
        extras/d2_debug_box.hpp
        extras/d2_debug_box.cpp
    )
    if(D2_TEST)
        target_sources(DeltaExtras PRIVATE examples/all.cpp)
    endif()
    target_link_libraries(DeltaExtras PUBLIC DeltaCore DeltaElements)
    delta_setup_target(DeltaExtras)
endif()

# Platform library
add_library(DeltaCoreOS STATIC
    os/d2_registry.cpp
    os/d2_registry.hpp
)
target_link_libraries(DeltaCoreOS PUBLIC DeltaCore)
delta_setup_target(DeltaCoreOS)

delta_discover_modules(
    TARGET DeltaCoreOS
    DEPS 
        DeltaCore
        $<$<TARGET_EXISTS:DeltaElements>:DeltaElements>
        $<$<TARGET_EXISTS:DeltaExtras>:DeltaExtras>
    ROOTS
        "${CMAKE_CURRENT_SOURCE_DIR}/os"
    EXCLUDE_BASES ${D2_MODULE_EXCLUDE_BASES}
    EXCLUDE_TYPES ${D2_MODULE_EXCLUDE_TYPES}
    EXCLUDE_DIRS ${D2_MODULE_EXCLUDE_DIRS}
)
delta_discover_module_root_groups(
    TARGET DeltaCoreOS
    DEPS
        DeltaCore
        $<$<TARGET_EXISTS:DeltaElements>:DeltaElements>
        $<$<TARGET_EXISTS:DeltaExtras>:DeltaExtras>
)
delta_generate_module_registry(
    TARGET DeltaCoreOS
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/generated/d2_platform.hpp"
    INCLUDE_ROOT "${CMAKE_CURRENT_BINARY_DIR}/generated"
)

######################
# EXPORTED INTERFACE #
######################

add_library(DeltaAscii       INTERFACE)
add_library(DeltaAscii::Core ALIAS DeltaAscii)

target_link_libraries(DeltaAscii INTERFACE
    DeltaCore
    $<$<TARGET_EXISTS:DeltaElements>:DeltaElements>
    $<$<TARGET_EXISTS:DeltaExtras>:DeltaExtras>
    $<$<TARGET_EXISTS:DeltaCoreOS>:DeltaCoreOS>
)

if (NOT D2_SANITIZER STREQUAL "none")
    target_link_libraries(DeltaAscii INTERFACE DeltaSan)
elseif(D2_ENABLE_INSTALL)
    set(_install_targets DeltaCore DeltaLSX DeltaCoreOS)
    if(TARGET DeltaElements)
        list(APPEND _install_targets DeltaElements)
    endif()
    if(TARGET DeltaExtras)
        list(APPEND _install_targets DeltaExtras)
    endif()
    install(TARGETS ${_install_targets} DeltaAscii
        EXPORT  DeltaAsciiTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
    install(
        DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/delt2a
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
    install(
        EXPORT      DeltaAsciiTargets
        FILE        DeltaAsciiTargets.cmake
        NAMESPACE   DeltaAscii::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/DeltaAscii
    )
endif()
