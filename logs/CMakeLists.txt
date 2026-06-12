cmake_minimum_required(VERSION 3.16)

project(logd LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(LOGD_ENABLE_INSTALL "Enable install" OFF)

if (NOT TARGET absl::base)
    find_package(absl REQUIRED)
endif()

add_library(logd STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/logd.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/logd.cpp
)
target_include_directories(logd PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(logd PRIVATE
    absl::stacktrace
    absl::symbolize
    absl::base
)

if (LOGD_ENABLE_INSTALL)
    install(TARGETS ${_install_targets} logd
        EXPORT  logdTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
    install(
        DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
    install(
        EXPORT      logdTargets
        FILE        logdTargets.cmake
        NAMESPACE   logd::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/logd-lib
    )
endif()
