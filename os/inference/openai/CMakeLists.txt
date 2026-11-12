if (NOT TARGET DeltaInference)
    return()
endif()

add_library(OpenAIProvider STATIC
    d2_openai.hpp d2_openai.cpp
    d2_openai_protocol.hpp d2_openai_protocol.cpp
)
add_library(DeltaAscii::OpenAI ALIAS OpenAIProvider)
set_target_properties(OpenAIProvider PROPERTIES EXPORT_NAME OpenAI)
target_link_libraries(OpenAIProvider PUBLIC DeltaInference)
delta_register_module(
    TYPE "d2::sys::SystemOpenAI"
    BASE "d2::sys::SystemProvider"
    HEADER "d2_openai.hpp"
    TARGET OpenAIProvider
)
