cmake_minimum_required(VERSION 3.21)
project(DeltaAscii LANGUAGES CXX VERSION 0.1.0)

include(GNUInstallDirs)
include(FetchContent)
include(cmake/target.cmake)
include(cmake/system.cmake)

###########
# OPTIONS #
###########

# General

option(D2_EXCLUDE_LIBRARY_INFERENCE "Disable inference abstractions"             OFF)
option(D2_EXCLUDE_LIBRARY_EXTRAS    "Disable extras module"                     OFF)
option(D2_EXCLUDE_LIBRARY_ELEMENTS  "Disable elements module"                   OFF)
option(D2_SECURITY_WATCH            "Enable runtime debug checks"               ON)
option(D2_SECURITY_LOG              "Enable runtime logging"                    ON)
option(D2_SECURITY_ASSERT           "Enable debug assertions"                   ON)
option(D2_UNICODE_MODE              "Enable unicode mode"                       ON)
option(D2_ENABLE_INSTALL            "Enable install for cmake"                  OFF)
option(D2_TEST                      "Enable building of examples"               OFF)
option(D2_BUILD_TESTS               "Build regression tests"                    OFF)
option(D2_BUILD_DOCS                "Build Doxygen API reference"                OFF)

# Sanitizing

option(D2_SANITIZER_RECOVER         "Enable sanitizer recovery where supported" ON)
option(D2_MSAN_TRACK_ORIGINS        "Enable MSAN origin tracking"               ON)
option(D2_ASAN_USE_AFTER_SCOPE      "Enable ASAN use-after-scope checks"        ON)

set(D2_SANITIZER "none" CACHE STRING "Sanitizer: none, address, memory, thread, undefined")
set_property(CACHE D2_SANITIZER PROPERTY STRINGS
    none
    address
    memory
    thread
    undefined
)

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
            GIT_TAG        d9e4955c65cd4367dd6bf46f4ccb8cd3d100540b
        )
        set(ABSL_PROPAGATE_CXX_STD ON  CACHE BOOL "" FORCE)
        set(ABSL_ENABLE_INSTALL    OFF CACHE BOOL "" FORCE)
        set(ABSL_BUILD_TESTING     OFF CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(absl)
    else()
        find_package(absl REQUIRED)
    endif()
endif()
if (D2_UNICODE_MODE AND NOT TARGET uni-algo::uni-algo)
    find_package(uni-algo REQUIRED)
endif()
if (NOT TARGET ada::ada)
    find_package(ada REQUIRED)
endif()

#############
# LIBRARIES #
#############

# Export stuff
set(TPMP_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
set(LOGD_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)

# Scheduler
add_subdirectory(mt)

# Logs
add_subdirectory(logs)

# LSX library
add_library(DeltaLSX STATIC
    core/lsx/d2_solver.hpp
    core/lsx/d2_solver.cpp
    core/lsx/d2_solver_types.hpp
    core/lsx/d2_solver_types.cpp
    core/lsx/d2_solver_ops.hpp
    core/lsx/d2_solver_ops.cpp
)
target_link_libraries(DeltaLSX PUBLIC
    absl::base
    absl::flat_hash_map
)
delta_setup_target(DeltaLSX)

# Core library
add_library(DeltaCore STATIC
    d2_std.hpp
    # Utils
    core/utils/d2_exceptions.hpp
    core/utils/d2_meta.hpp
    core/utils/d2_model.hpp
    core/utils/d2_model.cpp
    # Types
    core/types/d2_vtypes.hpp
    core/types/d2_pixel.hpp
    core/types/d2_pixel.cpp
    core/types/d2_image.hpp
    core/types/d2_image.cpp
    core/types/d2_element_units.hpp
    # Platform
    core/platform/d2_locale.hpp
    core/platform/d2_platform_string.hpp
    core/platform/d2_platform_string.cpp
    core/platform/d2_text_layout.hpp
    core/platform/d2_text_layout.cpp
    core/platform/d2_extended_page.hpp
    core/platform/d2_extended_page.cpp
    core/platform/d2_fragmented_buffer.hpp
    core/platform/d2_fragmented_buffer.cpp
    core/platform/d2_chardef.hpp
    core/platform/d2_colors.hpp
    # Tree
    core/tree/d2_tree_element_frwd.hpp
    core/tree/d2_tree_element.hpp
    core/tree/d2_tree_element.cpp
    core/tree/d2_tree_parent.hpp
    core/tree/d2_tree_parent.cpp
    core/tree/d2_tree_state.hpp
    core/tree/d2_tree_state.cpp
    core/tree/d2_tree.hpp
    core/tree/d2_tree_construct.hpp
    core/tree/d2_theme.hpp
    core/tree/d2_theme.cpp
    core/tree/d2_styles.hpp
    core/tree/d2_styles_base.hpp
    core/tree/d2_text_select.hpp
    core/tree/d2_text_select.cpp
    # IO
    core/io/d2_context_frwd.hpp
    core/io/d2_context.hpp
    core/io/d2_context.cpp
    core/io/d2_main_worker.hpp
    core/io/d2_main_worker.cpp
    core/io/d2_signal_handler.hpp
    core/io/d2_signal_handler.cpp
    core/io/d2_input_base.hpp
    core/io/d2_input_base.cpp
    # Screen
    core/screen/d2_screen.hpp
    core/screen/d2_screen.cpp
    core/screen/d2_screen_comps.cpp
    core/screen/d2_screen_comps.hpp
    core/screen/d2_animate.cpp
    core/screen/d2_animate.hpp
    # Mods
    core/io/d2_module.hpp
    core/io/d2_module.cpp
    core/mods/d2_images.hpp
    core/mods/d2_images.cpp
    core/mods/d2_image_loader.hpp
    core/mods/d2_image_loader.cpp
    core/mods/d2_core.hpp
    core/mods/d2_core.cpp
    core/mods/d2_ext.hpp
    core/mods/d2_ext.cpp
    core/mods/d2_audio.hpp
    core/mods/d2_audio.cpp
    core/mods/d2_binder.hpp
    core/mods/d2_binder.cpp
    core/mods/d2_http.hpp
    core/mods/d2_http.cpp
    core/mods/d2_ws.hpp
    core/mods/d2_ws.cpp
    core/mods/d2_session.hpp
    core/mods/d2_session.cpp
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
if (NOT D2_EXCLUDE_LIBRARY_EXTRAS AND NOT D2_EXCLUDE_LIBRARY_ELEMENTS)
    add_library(DeltaExtras STATIC
        # Presets
        extras/d2_default_theme.hpp
        extras/d2_default_theme.cpp
        extras/d2_default_styles.hpp
        # Other stuff 
        extras/d2_loading_indicators.hpp
        extras/d2_debug_box.hpp
        extras/d2_debug_box.cpp
    )
    if(D2_TEST)
        target_sources(DeltaExtras PRIVATE examples/all.cpp)
    endif()
    target_link_libraries(DeltaExtras PUBLIC DeltaCore DeltaElements)
    delta_setup_target(DeltaExtras)
endif()

if (NOT D2_EXCLUDE_LIBRARY_INFERENCE)
    add_subdirectory(core/mods/inference)
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
    DEPS DeltaCore
    ROOTS
        "${CMAKE_CURRENT_SOURCE_DIR}/os"
    EXCLUDE_BASES ${D2_MODULE_EXCLUDE_BASES}
    EXCLUDE_TYPES ${D2_MODULE_EXCLUDE_TYPES}
    EXCLUDE_DIRS ${D2_MODULE_EXCLUDE_DIRS}
)
delta_discover_module_root_groups(
    TARGET DeltaCoreOS
    DEPS DeltaCore
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

set_target_properties(DeltaAscii PROPERTIES EXPORT_NAME Core)
set(_delta_targets DeltaCore DeltaLSX DeltaCoreOS TPMP TPMPQueue logd)
set(_delta_interface DeltaCore DeltaCoreOS)
foreach(component IN ITEMS DeltaElements DeltaExtras DeltaInference)
    if (TARGET ${component})
        list(APPEND _delta_targets ${component})
        list(APPEND _delta_interface ${component})
    endif()
endforeach()
target_link_libraries(DeltaAscii INTERFACE ${_delta_interface})

if (NOT D2_SANITIZER STREQUAL "none")
    target_link_libraries(DeltaAscii INTERFACE DeltaSan)
    if (D2_ENABLE_INSTALL)
        message(FATAL_ERROR "Install a non-sanitized build")
    endif()
elseif(D2_ENABLE_INSTALL)
    include(CMakePackageConfigHelpers)
    get_property(_delta_modules GLOBAL PROPERTY DELTA_REGISTERED_MODULE_TARGETS)
    list(APPEND _delta_targets ${_delta_modules})
    list(REMOVE_DUPLICATES _delta_targets)
    install(TARGETS ${_delta_targets} DeltaAscii
        EXPORT DeltaAsciiTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
    install(DIRECTORY core elements extras mt logs os
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/delt2a
        FILES_MATCHING PATTERN "*.hpp"
    )
    install(FILES
        "${TPMP_CONCURRENTQUEUE_SOURCE_DIR}/concurrentqueue.h"
        "${TPMP_CONCURRENTQUEUE_SOURCE_DIR}/blockingconcurrentqueue.h"
        "${TPMP_CONCURRENTQUEUE_SOURCE_DIR}/lightweightsemaphore.h"
        "${TPMP_CONCURRENTQUEUE_SOURCE_DIR}/LICENSE.md"
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/delt2a/mt/detail/moodycamel
    )
    install(FILES d2_std.hpp "${CMAKE_CURRENT_BINARY_DIR}/generated/d2_platform.hpp"
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/delt2a
    )
    install(FILES core/platform/generated/UNICODE-LICENSE.txt
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/delt2a/core/platform/generated
    )
    set(D2_CONFIG_INFERENCE FALSE)
    if (TARGET DeltaInference)
        set(D2_CONFIG_INFERENCE TRUE)
    endif()
    set(D2_CONFIG_PNG FALSE)
    get_property(D2_CONFIG_PNG GLOBAL PROPERTY DELTA_IMAGE_PNG)
    if (NOT D2_CONFIG_PNG)
        set(D2_CONFIG_PNG FALSE)
    endif()
    set(D2_CONFIG_CURL FALSE)
    set(D2_CONFIG_PIPEWIRE FALSE)
    if (TARGET CurlSystemNetwork)
        set(D2_CONFIG_CURL TRUE)
    endif()
    if (TARGET PipewireSystemAudio)
        set(D2_CONFIG_PIPEWIRE TRUE)
    endif()
    configure_package_config_file(cmake/DeltaAsciiConfig.cmake.in
        "${CMAKE_CURRENT_BINARY_DIR}/DeltaAsciiConfig.cmake"
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/DeltaAscii
    )
    write_basic_package_version_file("${CMAKE_CURRENT_BINARY_DIR}/DeltaAsciiConfigVersion.cmake"
        VERSION ${PROJECT_VERSION} COMPATIBILITY SameMajorVersion
    )
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/DeltaAsciiConfig.cmake"
                  "${CMAKE_CURRENT_BINARY_DIR}/DeltaAsciiConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/DeltaAscii
    )
    install(EXPORT DeltaAsciiTargets FILE DeltaAsciiTargets.cmake
        NAMESPACE DeltaAscii:: DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/DeltaAscii
    )
endif()

if (D2_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if (D2_TEST AND TARGET DeltaExtras)
    add_executable(d2_manual examples/manual.cpp)
    target_link_libraries(d2_manual PRIVATE DeltaAscii::Core)
    add_executable(d2_hello examples/hello.cpp)
    target_link_libraries(d2_hello PRIVATE DeltaAscii::Core)
    add_executable(d2_selection examples/selection.cpp)
    target_link_libraries(d2_selection PRIVATE DeltaAscii::Core)
    if (TARGET ImagesKitty AND TARGET ImagesSixel)
        add_executable(d2_images examples/images.cpp)
        target_link_libraries(d2_images PRIVATE DeltaAscii::Core)
        if (TARGET PngLoader)
            target_link_libraries(d2_images PRIVATE PngLoader)
            target_compile_definitions(d2_images PRIVATE D2_EXAMPLE_PNG)
        endif()
    endif()
    add_executable(d2_editing examples/editing.cpp)
    target_link_libraries(d2_editing PRIVATE DeltaAscii::Core)
endif()

if (D2_TEST AND TARGET OpenAIProvider)
    add_executable(d2_inference examples/inference.cpp)
    target_link_libraries(d2_inference PRIVATE DeltaAscii::Core DeltaAscii::OpenAI)
endif()

if (D2_BUILD_DOCS)
    find_package(Doxygen REQUIRED)
    find_package(Python3 3.9 REQUIRED COMPONENTS Interpreter)
    set(DOXYGEN_EXTRACT_ALL YES)
    set(DOXYGEN_WARN_IF_UNDOCUMENTED NO)
    set(DOXYGEN_GENERATE_LATEX NO)
    set(DOXYGEN_FILTER_PATTERNS "*.md=\"${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/doxygen_filter.py\"")
    set(DOXYGEN_VERBATIM_VARS DOXYGEN_FILTER_PATTERNS)
    set(DOXYGEN_GENERATE_XML YES)
    set(DOXYGEN_GENERATE_TREEVIEW YES)
    set(DOXYGEN_SOURCE_BROWSER YES)
    set(DOXYGEN_HAVE_DOT NO)
    set(DOXYGEN_QUIET YES)
    set(DOXYGEN_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/docs")
    set(DOXYGEN_STRIP_FROM_PATH "${CMAKE_CURRENT_SOURCE_DIR}")
    set(DOXYGEN_EXCLUDE_PATTERNS "*/generated/*" "*/detail/*")
    set(DOXYGEN_USE_MDFILE_AS_MAINPAGE "${CMAKE_CURRENT_SOURCE_DIR}/docs/index.md")
    doxygen_add_docs(d2_docs d2_std.hpp core elements extras mt logs os docs)
endif()
