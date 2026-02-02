cmake_minimum_required(VERSION 3.16)

project(logd LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(absl REQUIRED)

add_library(logd STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/runtime_logs.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/runtime_logs.cpp
)
target_link_libraries(logd PRIVATE
    absl::stacktrace
    absl::symbolize
    absl::base
)

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
