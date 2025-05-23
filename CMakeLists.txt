cmake_minimum_required(VERSION 3.16)

project(DeltaAscii LANGUAGES CXX)

option(D2_EXCLUDE_LIBRARY_TEMPLATES "Disable elements module"   OFF)
option(D2_EXCLUDE_LIBRARY_ELEMENTS  "Disable templates module"  OFF)
option(D2_EXCLUDE_OS_CORE           "Disable default os module" OFF)
option(D2_SANITIZE_MEMORY           "Enable memory sanitizer"   OFF)
option(D2_SANITIZE_THREAD           "Enable thread sanitizer"   OFF)

if (D2_SANITIZE_MEMORY OR D2_SANITIZE_THREAD)
    set(CMAKE_CXX_FLAGS   "${CMAKE_CXX_FLAGS} -O0 -g -fno-omit-frame-pointer -fvisibility=default")
    set(ENV{ASAN_OPTIONS} "${ASAN_OPTIONS}:second_deadlock_stack=1:symbolize=1")
    if (D2_SANITIZE_THREAD)
        add_compile_options(-fsanitize=address,undefined,leak,bounds,cfi)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address,undefined,leak,bounds,cfi")
    elseif(D2_SANITIZE_MEMORY)
        add_compile_options(-fsanitize=thread,bounds,cfi)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=thread,bounds,cfi")
    endif()
endif()

set(CMAKE_CXX_STANDARD                 20)
set(CMAKE_CXX_STANDARD_REQUIRED        ON)
set(CMAKE_POSITION_INDEPENDENT_CODE    ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_ARCHIVE_CREATE "<CMAKE_AR> qcD <TARGET> <OBJECTS>")
    set(CMAKE_CXX_ARCHIVE_APPEND "<CMAKE_AR> qD  <TARGET> <OBJECTS>")
    set(CMAKE_CXX_ARCHIVE_FINISH "<CMAKE_RANLIB> <TARGET>")
endif()

add_executable(DeltaAscii main.cpp)
target_include_directories(DeltaAscii
    PRIVATE
    ${CMAKE_SOURCE_DIR}/delt2a
)

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

    # Pixel stuff

    delt2a/d2_pixel.hpp
    delt2a/d2_colors.hpp
    delt2a/d2_interpolator.hpp

    delt2a/d2_pixel.cpp

    # Tree data

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
target_link_libraries(DeltaAscii PRIVATE DeltaCore)

if(NOT D2_EXCLUDE_LIBRARY_ELEMENTS)
    add_library(DeltaElements STATIC
        delt2a/elements/d2_std.hpp
        delt2a/elements/d2_element_utils.hpp
        delt2a/elements/d2_draggable_box.hpp
        delt2a/elements/d2_button.hpp
        delt2a/elements/d2_scrollbox.hpp
        delt2a/elements/d2_slider.hpp
        delt2a/elements/d2_text.hpp
        delt2a/elements/d2_box.hpp
        delt2a/elements/d2_input.hpp
        delt2a/elements/d2_dropdown.hpp
        delt2a/elements/d2_matrix.hpp
        delt2a/elements/d2_switch.hpp
        delt2a/elements/d2_graph.hpp
        delt2a/elements/d2_checkbox.hpp

        delt2a/elements/d2_box.cpp
        delt2a/elements/d2_button.cpp
        delt2a/elements/d2_checkbox.cpp
        delt2a/elements/d2_draggable_box.cpp
        delt2a/elements/d2_dropdown.cpp
        delt2a/elements/d2_graph.cpp
        delt2a/elements/d2_input.cpp
        delt2a/elements/d2_matrix.cpp
        delt2a/elements/d2_scrollbox.cpp
        delt2a/elements/d2_slider.cpp
        delt2a/elements/d2_switch.cpp
        delt2a/elements/d2_text.cpp
    )
    target_link_libraries(DeltaAscii PRIVATE DeltaElements)
endif()
if(NOT D2_EXCLUDE_LIBRARY_TEMPLATES)
    add_library(DeltaTemplates STATIC
        # Popups

        delt2a/templates/d2_popup_theme_base.hpp

        delt2a/templates/d2_file_explorer.hpp
        delt2a/templates/d2_exit_prompt.hpp
        delt2a/templates/d2_color_picker.hpp
        delt2a/templates/d2_theme_selector.hpp

        delt2a/templates/d2_file_explorer.cpp
        delt2a/templates/d2_theme_selector.cpp
        delt2a/templates/d2_exit_prompt.cpp
        delt2a/templates/d2_color_picker.cpp

        # Programs

        delt2a/programs/d2_debug_box.hpp
        delt2a/programs/d2_element_test.hpp
        delt2a/programs/d2_quedits.hpp
    )
    target_link_libraries(DeltaAscii PRIVATE DeltaTemplates)
endif()

if (NOT D2_EXCLUDE_OS_CORE)
    if (UNIX)
        add_library(DeltaCoreUnix STATIC
            # IO Handlers

            delt2a/os/d2_unix_system_io.hpp
            delt2a/os/d2_unix_system_io.cpp
        )
        target_link_libraries(DeltaAscii PRIVATE DeltaCoreUnix)
    else()
        message(FATAL_ERROR "Unsupported platform")
    endif()
endif()

include(GNUInstallDirs)
install(TARGETS DeltaAscii
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
