#ifndef D2_HTTP_HPP
#define D2_HTTP_HPP

#include <d2_io_handler.hpp>
#include <d2_module.hpp>

namespace d2::sys
{
    class SystemHTTP : public d2::sys::AbstractModule<SystemHTTP, "HTTP">
    {
    public:
        struct Method
        {
#define METHOD(_value_, _str_) static constexpr auto _value_ = std::string_view(#_str_);
            METHOD(Get, GET)
            METHOD(Post, POST)
            METHOD(Put, PUT)
            METHOD(Patch, PATCH)
            METHOD(Delete, DELETE)
            METHOD(Head, HEAD)
            METHOD(Options, OPTIONS)
#undef METHOD
        };
        struct Response
        {
            d2::IOContext::ptr ctx{nullptr};

            virtual int status() = 0;
            virtual std::optional<std::string_view> get(std::string_view header) = 0;
            virtual std::string_view body() = 0;
            virtual ~Response() = default;
        };
        struct Stream
        {
            using ptr = std::unique_ptr<Stream>;

            virtual void write(std::string_view data) = 0;
            virtual void finish() = 0;
            virtual void cancel() = 0;
            virtual bool is_open() = 0;

            virtual std::size_t up_current() = 0;
            virtual std::size_t up_total() = 0;

            virtual ~Stream() = default;
        };
        struct Chunk
        {
            d2::IOContext::ptr ctx{nullptr};

            virtual std::string_view read() = 0;
            virtual void cancel() = 0;

            virtual std::size_t down_current() = 0;
            virtual std::size_t down_total() = 0;

            virtual ~Chunk() = default;
        };
        struct RequestBuilder
        {
            d2::IOContext::ptr ctx{nullptr};

            virtual void set(std::string_view header, std::string_view value) = 0;
            virtual void body(std::string_view value) = 0;
            virtual void stream() = 0;

            virtual ~RequestBuilder() = default;
        };
        struct Request
        {
            std::string_view method{Method::Get};
            std::string_view url{""};
            std::function<void(RequestBuilder&)> on_setup{nullptr};
            std::function<void(Response&)> on_response{nullptr};
            std::function<void(std::string, d2::IOContext::ptr)> on_error{nullptr};
            std::function<void(Chunk&)> on_chunk{nullptr};
        };
    public:
        virtual Stream::ptr request(Request request) = 0;

        using AbstractModule::AbstractModule;
    };
    using http = SystemHTTP;
} // namespace d2::sys

#endif
