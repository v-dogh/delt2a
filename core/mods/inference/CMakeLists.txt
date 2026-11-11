find_package(nlohmann_json 3.11 QUIET CONFIG)
if (NOT TARGET nlohmann_json::nlohmann_json)
    set(JSON_BuildTests OFF CACHE BOOL "" FORCE)
    set(JSON_Install ON CACHE BOOL "" FORCE)
    FetchContent_Declare(nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.12.0
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(nlohmann_json)
endif()
add_library(DeltaInference STATIC
    d2_types.hpp d2_types.cpp d2_provider.hpp
    d2_tools.hpp d2_tools.cpp
    d2_conversation.hpp d2_conversation.cpp d2_std.hpp
)
add_library(DeltaAscii::Inference ALIAS DeltaInference)
set_target_properties(DeltaInference PROPERTIES EXPORT_NAME Inference)
target_link_libraries(DeltaInference PUBLIC DeltaCore nlohmann_json::nlohmann_json)
delta_setup_target(DeltaInference)
