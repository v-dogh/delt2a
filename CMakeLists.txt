cmake_minimum_required(VERSION 3.21)
project(DeltaAscii LANGUAGES CXX VERSION 0.1.0)

include(GNUInstallDirs)

###########
# OPTIONS #
###########

option(D2_EXCLUDE_LIBRARY_TEMPLATES "Disable templates module"     OFF)
option(D2_EXCLUDE_LIBRARY_ELEMENTS  "Disable elements module"      OFF)
option(D2_EXCLUDE_OS_CORE           "Disable default os module"    OFF)
option(D2_EXCLUDE_OS_EXT            "Disable extended os modules"  ON)
option(D2_SANITIZE_MEMORY           "Enable memory sanitizer"      OFF)
option(D2_SANITIZE_THREAD           "Enable thread sanitizer"      OFF)
option(D2_SECURITY_WATCH            "Enable runtime debug checks"  ON)
option(D2_SECURITY_LOG              "Enable runtime logging"       ON)
option(D2_SECURITY_ASSERT           "Enable debug assertions"      ON)
option(D2_UNICODE_MODE              "Enable unicode mode"          ON)
option(D2_STRICT_MODE               "Enable strict component mode" ON)

# Force reset cache
# set(D2_EXCLUDE_LIBRARY_TEMPLATES OFF CACHE BOOL "Disable templates module"     FORCE)
# set(D2_EXCLUDE_LIBRARY_ELEMENTS  OFF CACHE BOOL "Disable elements module"      FORCE)
# set(D2_EXCLUDE_OS_CORE           OFF CACHE BOOL "Disable default os module"    FORCE)
# set(D2_EXCLUDE_OS_EXT            OFF CACHE BOOL "Disable extended os modules"  FORCE)
# set(D2_SANITIZE_MEMORY           OFF CACHE BOOL "Enable memory sanitizer"      FORCE)
# set(D2_SANITIZE_THREAD           OFF CACHE BOOL "Enable thread sanitizer"      FORCE)
# set(D2_SECURITY_WATCH            OFF CACHE BOOL "Enable runtime debug checks"  FORCE)
# set(D2_SECURITY_LOG              OFF CACHE BOOL "Enable runtime logging"       FORCE)
# set(D2_SECURITY_ASSERT           OFF CACHE BOOL "Enable debug assertions"      FORCE)
# set(D2_UNICODE_MODE              OFF CACHE BOOL "Enable unicode mode"          FORCE)
# set(D2_STRICT_MODE               OFF CACHE BOOL "Enable strict component mode" FORCE)

set(D2_SANITIZER_FLAGS "")
if(D2_SANITIZE_MEMORY AND D2_SANITIZE_THREAD)
    message(FATAL_ERROR "ASAN and TSAN cannot be enabled together")
endif()
if(D2_SANITIZE_MEMORY)
    list(APPEND D2_SANITIZER_FLAGS -fsanitize=memory -fsanitize-recover=memory)
endif()
if(D2_SANITIZE_THREAD)
    list(APPEND D2_SANITIZER_FLAGS -fsanitize=thread)
endif()

#########################
# EXTERNAL DEPENDENCIES #
#########################

# DeltaLSX | DeltaCore | DeltaCoreOS (Unix)
find_package(absl REQUIRED)
if (D2_UNICODE_MODE)
    # DeltaCore (for unicode support)
    find_package(uni-algo REQUIRED)
endif()
if (NOT D2_EXCLUDE_OS_CORE AND NOT D2_EXCLUDE_OS_EXT)
    # DeltaCoreOS (for linux audio)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(PIPEWIRE REQUIRED libpipewire-0.3)
endif()


#########################
# SUBLIBRARY PROPERTIES #
#########################

function(delta_setup_target target)
    target_compile_features(${target} PUBLIC cxx_std_20)
    target_include_directories(${target} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
    )
    if (D2_SANITIZE_MEMORY OR D2_SANITIZE_THREAD)
        target_compile_options(${target} PUBLIC
            -fno-omit-frame-pointer
            ${D2_SANITIZER_FLAGS}
        )
        target_link_options(${target} PUBLIC
            ${D2_SANITIZER_FLAGS}
        )
    endif()
    if (D2_UNICODE_MODE)
        target_compile_definitions(${target} PUBLIC D2_LOCALE_MODE=32)
        target_link_libraries(${target} PUBLIC uni-algo)
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

#############
# LIBRARIES #
#############

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
target_link_libraries(DeltaLSX PRIVATE
    absl::base
    absl::flat_hash_map
)
delta_setup_target(DeltaLSX)

# Core library
add_library(DeltaCore STATIC
    # Core
    d2_std.hpp
    d2_dsl.hpp
    d2_sdsl.hpp
    d2_cdsl.hpp
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
    d2_platform.hpp
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
)
target_link_libraries(DeltaCore PRIVATE
    TPMP
    logd
    DeltaLSX
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

# Standard templates library
if (NOT D2_EXCLUDE_LIBRARY_TEMPLATES)
    add_library(DeltaTemplates STATIC
        # Presets
        templates/d2_default_theme.hpp
        templates/d2_default_theme.cpp
        templates/d2_default_styles.hpp
        # Widgets
        # templates/d2_loading_indicators.hpp
        # templates/d2_file_explorer.hpp
        # templates/d2_file_explorer.cpp
        # templates/d2_exit_prompt.hpp
        # templates/d2_exit_prompt.cpp
        # templates/d2_color_picker.hpp
        # templates/d2_color_picker.cpp
        # templates/d2_theme_selector.hpp
        # templates/d2_theme_selector.cpp
        # templates/d2_audio_device_selector.hpp
        # templates/d2_audio_device_selector.cpp
        # Programs
        programs/d2_debug_box.hpp
        programs/d2_debug_box.cpp
        programs/d2_element_test.hpp
        programs/d2_quedits.hpp
    )
    target_link_libraries(DeltaTemplates PUBLIC DeltaCore DeltaElements)
    delta_setup_target(DeltaTemplates)
endif()

# Platform library
add_library(DeltaCoreOS STATIC)
target_link_libraries(DeltaCoreOS PUBLIC DeltaCore)
if (NOT D2_EXCLUDE_OS_CORE AND UNIX)
    target_sources(DeltaCoreOS PUBLIC
        os/d2_unix_system_io.hpp
        os/d2_unix_system_io.cpp
    )
    target_link_libraries(DeltaCoreOS PRIVATE absl::strings)
    if (LINUX)
        target_sources(DeltaCoreOS PUBLIC
            os/d2_linux_system_input.hpp
            os/d2_linux_system_input.cpp
        )
        if (NOT D2_EXCLUDE_OS_EXT)
            target_sources(DeltaCoreOS PUBLIC
                os/d2_linux_system_audio.hpp
                os/d2_linux_system_audio.cpp
            )
            target_include_directories (DeltaCoreOS PUBLIC ${PIPEWIRE_INCLUDE_DIRS})
            target_compile_options     (DeltaCoreOS PUBLIC ${PIPEWIRE_CFLAGS_OTHER})
            target_link_libraries      (DeltaCoreOS PUBLIC ${PIPEWIRE_LIBRARIES})
        endif()
    endif()
endif()
delta_setup_target(DeltaCoreOS)

######################
# EXPORTED INTERFACE #
######################

add_library(DeltaAscii       INTERFACE)
add_library(DeltaAscii::Core ALIAS DeltaAscii)

target_link_libraries(DeltaAscii INTERFACE
    DeltaCore
    $<$<TARGET_EXISTS:DeltaElements>:DeltaElements>
    $<$<TARGET_EXISTS:DeltaTemplates>:DeltaTemplates>
    $<$<TARGET_EXISTS:DeltaCoreOS>:DeltaCoreOS>
)

set(_install_targets DeltaCore DeltaLSX)
if(TARGET DeltaElements)
    list(APPEND _install_targets DeltaElements)
endif()
if(TARGET DeltaTemplates)
    list(APPEND _install_targets DeltaTemplates)
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
