cmake_minimum_required(VERSION 3.21)
project(DeltaAscii LANGUAGES CXX VERSION 0.1.0)

include(GNUInstallDirs)

###########
# OPTIONS #
###########

option(D2_EXCLUDE_LIBRARY_TEMPLATES "Disable templates module"     OFF)
option(D2_EXCLUDE_LIBRARY_ELEMENTS  "Disable elements module"      OFF)
option(D2_EXCLUDE_OS_CORE           "Disable default os module"    OFF)
option(D2_EXCLUDE_OS_EXT            "Disable extended os modules"  OFF)
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
    target_compile_features    (${target} PUBLIC cxx_std_20)
    target_include_directories (${target} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
    )
    if (D2_SANITIZE_MEMORY OR D2_SANITIZE_THREAD)
        target_compile_options (${target} PUBLIC -fno-omit-frame-pointer ${D2_SANITIZER_FLAGS})
        target_link_options    (${target} PUBLIC                         ${D2_SANITIZER_FLAGS})
    endif()
    if (D2_UNICODE_MODE)
        target_compile_definitions(${target} PUBLIC D2_LOCALE_MODE=32)
        target_link_libraries     (${target} PUBLIC uni-algo)
    else()
        target_compile_definitions(${target} PUBLIC D2_LOCALE_MODE=8)
    endif()
    target_compile_definitions(${target} PUBLIC
        ASCII=8
        UNICODE=32
    )
    if (D2_SECURITY_WATCH)
        add_compile_definitions(D2_SECURITY_WATCH)
    endif()
    if (D2_SECURITY_ASSERT)
        add_compile_definitions(D2_SECURITY_ASSERT)
    endif()
    if (D2_SECURITY_LOG)
        add_compile_definitions(D2_SECURITY_LOG)
    endif()
    if (D2_STRICT_MODE)
        add_compile_definitions(D2_COMPATIBILITY_MODE STRICT)
    endif()
endfunction()

#############
# LIBRARIES #
#############

# Scheduler
add_subdirectory(delt2a/mt)
# Logs
add_subdirectory(delt2a/logs)

# LSX library
add_library(DeltaLSX STATIC
    delt2a/lsx/d2_solver.hpp
    delt2a/lsx/d2_solver.cpp
    delt2a/lsx/d2_solver_types.hpp
    delt2a/lsx/d2_solver_types.cpp
    delt2a/lsx/d2_solver_ops.hpp
    delt2a/lsx/d2_solver_ops.cpp
)
target_link_libraries(DeltaLSX PRIVATE
    absl::base
    absl::flat_hash_map
)
delta_setup_target(DeltaLSX)

# Core library
add_library(DeltaCore STATIC
    # Core
    delt2a/d2_std.hpp
    delt2a/d2_dsl.hpp
    delt2a/d2_sdsl.hpp
    delt2a/d2_cdsl.hpp
    delt2a/d2_exceptions.hpp
    delt2a/d2_styles.hpp
    delt2a/d2_styles_base.hpp
    delt2a/d2_model.hpp
    delt2a/d2_model.cpp
    delt2a/d2_meta.hpp
    # Platform
    delt2a/d2_locale.hpp
    delt2a/d2_chardef.hpp
    delt2a/d2_platform.hpp
    delt2a/d2_platform_string.hpp
    delt2a/d2_platform_string.cpp
    delt2a/d2_fragmented_buffer.hpp
    delt2a/d2_fragmented_buffer.cpp
    # Pixel
    delt2a/d2_pixel.hpp
    delt2a/d2_colors.hpp
    delt2a/d2_interpolator.cpp
    delt2a/d2_interpolator.hpp
    delt2a/d2_pixel.cpp
    delt2a/d2_extended_page.hpp
    delt2a/d2_extended_page.cpp
    # Tree
    delt2a/d2_element_units.hpp
    delt2a/d2_tree_state.hpp
    delt2a/d2_tree_state.cpp
    delt2a/d2_tree_element_frwd.hpp
    delt2a/d2_tree_element.hpp
    delt2a/d2_tree_parent.hpp
    delt2a/d2_tree.hpp
    delt2a/d2_tree_element.cpp
    delt2a/d2_tree_parent.cpp
    # Context
    delt2a/d2_io_handler_frwd.hpp
    delt2a/d2_io_handler.hpp
    delt2a/d2_io_handler.cpp
    delt2a/d2_module.hpp
    delt2a/d2_module.cpp
    delt2a/d2_signal_handler.hpp
    delt2a/d2_signal_handler.cpp
    delt2a/d2_screen_comps.hpp
    delt2a/d2_screen.hpp
    delt2a/d2_screen.cpp
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
        delt2a/elements/d2_std.hpp
        delt2a/elements/d2_element_utils.hpp
        delt2a/elements/d2_element_utils.cpp
        # Boxes
        delt2a/elements/d2_box.hpp
        delt2a/elements/d2_box.cpp
        delt2a/elements/d2_flow_box.hpp
        delt2a/elements/d2_flow_box.cpp
        delt2a/elements/d2_draggable_box.hpp
        delt2a/elements/d2_draggable_box.cpp
        delt2a/elements/d2_scrollbox.hpp
        delt2a/elements/d2_scrollbox.cpp
        delt2a/elements/d2_demand_scrollbox.hpp
        delt2a/elements/d2_demand_scrollbox.cpp
        # Text
        delt2a/elements/d2_text.hpp
        delt2a/elements/d2_text.cpp
        delt2a/elements/d2_input.hpp
        delt2a/elements/d2_input.cpp
        delt2a/elements/d2_multiline_input.hpp
        delt2a/elements/d2_multiline_input.cpp
        # Interactive
        delt2a/elements/d2_button.hpp
        delt2a/elements/d2_button.cpp
        delt2a/elements/d2_slider.hpp
        delt2a/elements/d2_slider.cpp
        delt2a/elements/d2_dropdown.hpp
        delt2a/elements/d2_dropdown.cpp
        delt2a/elements/d2_switch.hpp
        delt2a/elements/d2_switch.cpp
        delt2a/elements/d2_checkbox.hpp
        delt2a/elements/d2_checkbox.cpp
        # Other
        delt2a/elements/d2_image.hpp
        delt2a/elements/d2_image.cpp
        delt2a/elements/d2_matrix.hpp
        delt2a/elements/d2_matrix.cpp
        delt2a/elements/d2_graph.hpp
        delt2a/elements/d2_graph.cpp
        delt2a/elements/d2_separator.hpp
        delt2a/elements/d2_separator.cpp
    )
    target_link_libraries(DeltaElements PUBLIC DeltaCore)
    delta_setup_target(DeltaElements)
endif()

# Standard templates library
if (NOT D2_EXCLUDE_LIBRARY_TEMPLATES)
    add_library(DeltaTemplates STATIC
        # Presets
        delt2a/templates/d2_widget_theme_base.hpp
        delt2a/templates/d2_standard_widget_theme.hpp      
        delt2a/templates/d2_loading_indicators.hpp
        delt2a/templates/d2_file_explorer.hpp
        delt2a/templates/d2_file_explorer.cpp
        delt2a/templates/d2_exit_prompt.hpp
        delt2a/templates/d2_exit_prompt.cpp
        delt2a/templates/d2_color_picker.hpp
        delt2a/templates/d2_color_picker.cpp
        delt2a/templates/d2_theme_selector.hpp
        delt2a/templates/d2_theme_selector.cpp
        delt2a/templates/d2_audio_device_selector.hpp
        delt2a/templates/d2_audio_device_selector.cpp
        # Programs
        delt2a/programs/d2_debug_box.hpp
        delt2a/programs/d2_debug_box.cpp
        delt2a/programs/d2_element_test.hpp
        delt2a/programs/d2_quedits.hpp
    )
    target_link_libraries(DeltaTemplates PUBLIC DeltaCore DeltaElements)
    delta_setup_target(DeltaTemplates)
endif()

# Platform library
add_library(DeltaCoreOS STATIC)
target_link_libraries(DeltaCoreOS PUBLIC DeltaCore)
if (NOT D2_EXCLUDE_OS_CORE AND UNIX)
    target_sources(DeltaCoreOS PUBLIC
        delt2a/os/d2_unix_system_io.hpp
        delt2a/os/d2_unix_system_io.cpp
    )
    target_link_libraries(DeltaCoreOS PRIVATE absl::strings)
    if (LINUX)
        target_sources(DeltaCoreOS PUBLIC
            delt2a/os/d2_linux_system_input.hpp
            delt2a/os/d2_linux_system_input.cpp
        )
        if (NOT D2_EXCLUDE_OS_EXT)
            target_sources(DeltaCoreOS PUBLIC
                delt2a/os/d2_linux_system_audio.hpp
                delt2a/os/d2_linux_system_audio.cpp
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
