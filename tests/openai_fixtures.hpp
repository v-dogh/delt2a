#pragma once

#include <core/mods/inference/d2_types.hpp>

namespace openai_test
{
    using namespace d2::ai;
    inline Request request()
    {
        Request request;
        request.model = {"local", "test"};
        request.messages.push_back(Message::text(Role::User, "hello"));
        request.tools.push_back({"add", "Add", {{"type", "object"}}});
        return request;
    }
    inline Json chunk(Json delta, Json finish = nullptr)
    {
        return {{"id", "completion-1"}, {"model", "served-test"}, {"choices", Json::array({
            {{"index", 0}, {"delta", std::move(delta)}, {"finish_reason", std::move(finish)}}})}};
    }
    inline Json response(std::string text = "hello")
    {
        return {{"id", "completion-1"}, {"model", "served-test"}, {"choices", Json::array({
            {{"index", 0}, {"message", {{"role", "assistant"}, {"content", std::move(text)}}}, {"finish_reason", "stop"}}})},
            {"usage", {{"prompt_tokens", 12}, {"completion_tokens", 3},
                {"prompt_tokens_details", {{"cached_tokens", 4}}}, {"completion_tokens_details", {{"reasoning_tokens", 1}}}}}};
    }
    inline std::string event(const Json& value) { return "data: " + value.dump() + "\n\n"; }
    inline std::string text_stream()
    {
        auto usage = response()["usage"];
        return event(chunk({{"role", "assistant"}, {"content", "he"}})) +
            event(chunk({{"content", "llo"}}, "stop")) +
            event(Json{{"choices", Json::array()}, {"usage", usage}}) + "data: [DONE]\n\n";
    }
    inline void expect(ErrorCode code, auto action)
    {
        try { action(); }
        catch (const Error& error) { CHECK(error.code == code); return; }
        throw std::runtime_error("Expected inference error");
    }
}
