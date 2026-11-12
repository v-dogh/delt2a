#pragma once

#include "d2_openai.hpp"

namespace d2::ai::openai::detail
{
    void validate_config(const Config& config);
    Json encode(const Request& request, const Compatibility& wire);
    Error provider_error(int status, std::string_view body);
    std::optional<std::chrono::milliseconds> retry_after(std::string_view value);

    class EventStream
    {
        std::string _line, _data, _event;
        std::size_t _limit;
        bool _first{true}, _skip_lf{false};
        void line(const std::function<void(std::string_view, std::string_view)>& emit);
    public:
        explicit EventStream(std::size_t limit) : _limit(limit) {}
        void feed(std::string_view bytes,
                  const std::function<void(std::string_view, std::string_view)>& emit);
        void finish() const;
    };

    class Decoder
    {
        struct Call { std::size_t block; std::string arguments; };
        Request _request;
        Compatibility _wire;
        EventHandler _events;
        Response _response;
        std::map<std::size_t, Call> _calls;
        std::optional<std::size_t> _text, _refusal, _reasoning;
        bool _finished{false}, _done{false};
        void text(std::string_view value, std::optional<std::size_t>& block);
        void message(const Json& message, bool streaming);
        void accept(const Json& object, bool streaming);
    public:
        Decoder(Request request, Compatibility wire, EventHandler events);
        void event(std::string_view type, std::string_view data);
        void json(std::string_view body);
        Response finish();
    };
}
