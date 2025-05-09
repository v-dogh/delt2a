cmake_minimum_required(VERSION 3.16)

project(DeltaAscii LANGUAGES CXX)

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g -fno-omit-frame-pointer")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=default -flto")
# set(ENV{ASAN_OPTIONS} "${ASAN_OPTIONS}:second_deadlock_stack=1")
# set(ENV{ASAN_OPTIONS} "${ASAN_OPTIONS}:symbolize=1:log_path=/home/dogh/logs.txt=continue_on_error=1")
# add_compile_options(-fsanitize=address,undefined,leak,bounds,cfi)
# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address,undefined,leak,bounds,cfi")
# Concurrency options
# add_compile_options(-fsanitize=thread,bounds,cfi)
# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=thread,bounds,cfi")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(DeltaAscii main.cpp
    # Core

    delt2a/d2_std.hpp

    delt2a/d2_exceptions.hpp
    delt2a/d2_locale.hpp
    delt2a/d2_platform.hpp
    delt2a/d2_styles.hpp
    delt2a/d2_dsl.hpp

    # Pixel stuff

    delt2a/d2_pixel.hpp
    delt2a/d2_colors.hpp
    delt2a/d2_interpolator.hpp

    # Tree data

    delt2a/d2_tree_element_frwd.hpp
    delt2a/d2_tree_element.hpp
    delt2a/d2_element_units.hpp

    # Screen

    delt2a/d2_screen.hpp
    delt2a/d2_screen_frwd.hpp
    delt2a/d2_io_handler.hpp
    delt2a/d2_io_handler_frwd.hpp

    # IO Handlers

    delt2a/os/d2_unix_system_io.hpp

    # Popups

    delt2a/templates/d2_file_explorer.hpp
    delt2a/templates/d2_exit_prompt.hpp
    delt2a/templates/d2_color_picker.hpp
    delt2a/templates/d2_debug_box.hpp
    delt2a/templates/d2_popup_theme_base.hpp
    delt2a/templates/d2_theme_selector.hpp

    # Programs

    delt2a/templates/d2_element_test.hpp
    delt2a/templates/d2_quedits.hpp

    # Elements

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

    delt2a/d2_styles_base.hpp
)

include(GNUInstallDirs)
install(TARGETS DeltaAscii
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
