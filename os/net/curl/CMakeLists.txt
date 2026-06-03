find_package(CURL QUIET)
if(NOT CURL_FOUND)
    return()
endif()

add_library(CurlSystemNetwork STATIC
    d2_curl_global.hpp
    d2_curl_global.cpp
    d2_http_curl.hpp
    d2_http_curl.cpp
    d2_ws_curl.hpp
    d2_ws_curl.cpp
)
target_link_libraries(CurlSystemNetwork PUBLIC CURL::libcurl)

delta_register_module(
    TYPE   "d2::sys::net::SystemHTTPCurl"
    BASE   "d2::sys::net::SystemHTTP"
    HEADER "d2_http_curl.hpp"
    TARGET CurlSystemNetwork
)
delta_register_module(
    TYPE   "d2::sys::net::SystemWSCurl"
    BASE   "d2::sys::net::SystemWS"
    HEADER "d2_ws_curl.hpp"
    TARGET CurlSystemNetwork
)
