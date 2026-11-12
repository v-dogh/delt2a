cmake_minimum_required(VERSION 3.16)

project(thread-pool LANGUAGES CXX)
include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(TPMP_ENABLE_INSTALL "Enable install" ON)

include(FetchContent)
set(TPMP_CONCURRENTQUEUE_SOURCE_DIR "" CACHE PATH "Existing concurrentqueue headers (optional)")
if (NOT TPMP_CONCURRENTQUEUE_SOURCE_DIR)
    FetchContent_Declare(tpmp_concurrentqueue
        GIT_REPOSITORY https://github.com/cameron314/concurrentqueue.git
        GIT_TAG 6dd38b8a1dbaa7863aa907045f32308a56a6ff5d
        SOURCE_SUBDIR _headers_only
    )
    FetchContent_MakeAvailable(tpmp_concurrentqueue)
    set(TPMP_CONCURRENTQUEUE_SOURCE_DIR "${tpmp_concurrentqueue_SOURCE_DIR}")
endif()
if (NOT PROJECT_IS_TOP_LEVEL)
    set(TPMP_CONCURRENTQUEUE_SOURCE_DIR "${TPMP_CONCURRENTQUEUE_SOURCE_DIR}" PARENT_SCOPE)
endif()
add_library(TPMPQueue INTERFACE)
target_include_directories(TPMPQueue INTERFACE
    $<BUILD_INTERFACE:${TPMP_CONCURRENTQUEUE_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/delt2a/mt/detail/moodycamel>
)

add_library(TPMP STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/pool.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pool.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/control.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/defer.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/future.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/task_queue.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/task_ring.hpp
)

target_compile_features(TPMP PUBLIC cxx_std_20)
target_include_directories(TPMP PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/delt2a>
)
find_package(Threads REQUIRED)
target_link_libraries(TPMP PUBLIC Threads::Threads TPMPQueue)

if (TPMP_ENABLE_INSTALL)
    install(TARGETS TPMP TPMPQueue
        EXPORT  TPMPTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
    install(
        DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
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
    install(
        EXPORT      TPMPTargets
        FILE        TPMPTargets.cmake
        NAMESPACE   TPMP::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/TPMP-lib
    )
endif()
