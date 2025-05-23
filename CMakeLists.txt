cmake_minimum_required(VERSION 3.21)
project(DeltaAscii LANGUAGES CXX VERSION 0.1.0)

###########
# OPTIONS #
###########

option(D2_EXCLUDE_LIBRARY_TEMPLATES "Disable templates module"  OFF)
option(D2_EXCLUDE_LIBRARY_ELEMENTS  "Disable elements module"   OFF)
option(D2_EXCLUDE_OS_CORE           "Disable default os module" OFF)
option(D2_SANITIZE_MEMORY           "Enable memory sanitizer"   OFF)
option(D2_SANITIZE_THREAD           "Enable thread sanitizer"   OFF)

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
# SUBLIBRARY PROPERTIES #
#########################

function(delta_setup_target target)
    target_compile_features(${target} PUBLIC cxx_std_20)
    if (D2_SANITIZE_MEMORY OR D2_SANITIZE_THREAD)
        target_compile_options (${target} PUBLIC -fno-omit-frame-pointer ${D2_SANITIZER_FLAGS})
        target_link_options    (${target} PUBLIC                         ${D2_SANITIZER_FLAGS})
    endif()
    if(CMAKE_INTERPROCEDURAL_OPTIMIZATION_SUPPORTED)
        set_target_properties  (${target} PROPERTIES INTERPROCEDURAL_OPTIMIZATION ON)
    endif()
endfunction()

#############
# LIBRARIES #
#############

# Core library
add_library(DeltaCore STATIC
    # Core
    delt2a/d2_std.hpp
    delt2a/d2_exceptions.hpp
    delt2a/d2_locale.hpp
    delt2a/d2_platform.hpp
    delt2a/d2_styles.hpp
    delt2a/d2_styles_base.hpp
    delt2a/d2_dsl.hpp
    delt2a/d2_model.hpp
    delt2a/d2_model.cpp
    # Pixel
    delt2a/d2_pixel.hpp
    delt2a/d2_colors.hpp
    delt2a/d2_interpolator.hpp
    delt2a/d2_pixel.cpp
    # Tree
    delt2a/d2_element_units.hpp
    delt2a/d2_tree_element_frwd.hpp
    delt2a/d2_tree_element.hpp
    delt2a/d2_tree.hpp
    delt2a/d2_tree_element.cpp
    # Screen
    delt2a/d2_screen.hpp
    delt2a/d2_screen_frwd.hpp
    delt2a/d2_io_handler.hpp
    delt2a/d2_io_handler_frwd.hpp
    delt2a/d2_screen.cpp
)
target_include_directories(DeltaCore PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)
delta_setup_target(DeltaCore)

# Standard elements library
if (NOT D2_EXCLUDE_LIBRARY_ELEMENTS)
    add_library(DeltaElements STATIC
        delt2a/elements/d2_std.hpp
        delt2a/elements/d2_element_utils.hpp
        # Boxes
        delt2a/elements/d2_box.hpp
        delt2a/elements/d2_box.cpp
        delt2a/elements/d2_draggable_box.hpp
        delt2a/elements/d2_draggable_box.cpp
        delt2a/elements/d2_scrollbox.hpp
        delt2a/elements/d2_scrollbox.cpp
        # Text
        delt2a/elements/d2_text.hpp
        delt2a/elements/d2_text.cpp
        delt2a/elements/d2_input.hpp
        delt2a/elements/d2_input.cpp
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
        delt2a/elements/d2_matrix.hpp
        delt2a/elements/d2_matrix.cpp
        delt2a/elements/d2_graph.hpp
        delt2a/elements/d2_graph.cpp
    )
    target_link_libraries     (DeltaElements PUBLIC DeltaCore)
    target_include_directories(DeltaElements PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
    )
    delta_setup_target(DeltaElements)
endif()

# Standard templates library
if (NOT D2_EXCLUDE_LIBRARY_TEMPLATES)
    add_library(DeltaTemplates STATIC
        # Popups
        delt2a/templates/d2_popup_theme_base.hpp
        delt2a/templates/d2_file_explorer.hpp
        delt2a/templates/d2_file_explorer.cpp
        delt2a/templates/d2_exit_prompt.hpp
        delt2a/templates/d2_exit_prompt.cpp
        delt2a/templates/d2_color_picker.hpp
        delt2a/templates/d2_color_picker.cpp
        delt2a/templates/d2_theme_selector.hpp
        delt2a/templates/d2_theme_selector.cpp
        # Programs
        delt2a/programs/d2_debug_box.hpp
        delt2a/programs/d2_element_test.hpp
        delt2a/programs/d2_quedits.hpp
    )
    target_link_libraries     (DeltaTemplates PUBLIC DeltaCore DeltaElements)
    target_include_directories(DeltaTemplates PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
    )
    delta_setup_target(DeltaTemplates)
endif()

if (NOT D2_EXCLUDE_OS_CORE AND UNIX)
    add_library(DeltaCoreUnix STATIC
        delt2a/os/d2_unix_system_io.hpp
        delt2a/os/d2_unix_system_io.cpp
    )
    target_link_libraries     (DeltaCoreUnix PUBLIC DeltaCore)
    target_include_directories(DeltaCoreUnix PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
    )
    delta_setup_target(DeltaCoreUnix)
endif()

######################
# EXPORTED INTERFACE #
######################

add_library(DeltaAscii       INTERFACE)
add_library(DeltaAscii::Core ALIAS DeltaAscii)

target_link_libraries(DeltaAscii INTERFACE
    DeltaCore
    $<$<TARGET_EXISTS:DeltaElements>:DeltaElements>
    $<$<TARGET_EXISTS:DeltaTemplates>:DeltaTemplates>
    $<$<TARGET_EXISTS:DeltaCoreUnix>:DeltaCoreUnix>
)

include(GNUInstallDirs)

set(_install_targets DeltaCore)
if(TARGET DeltaElements)
    list(APPEND _install_targets DeltaElements)
endif()
if(TARGET DeltaTemplates)
    list(APPEND _install_targets DeltaTemplates)
endif()
if(TARGET DeltaCoreUnix)
    list(APPEND _install_targets DeltaCoreUnix)
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
