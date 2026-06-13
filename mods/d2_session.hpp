#pragma once

#include <d2_io_handler.hpp>
#include <d2_module.hpp>
#include <regex>
#include <variant>

namespace d2::sys::net
{
    class SystemSession
        : public d2::sys::AbstractModule<SystemSession, "Session", d2::sys::Access::TSafe>
    {
    public:
        struct Scope
        {
            using rule = std::variant<std::string, std::regex, std::monostate>;
            rule protocol{std::monostate{}};
            rule method{std::monostate{}};
            rule domain{std::monostate{}};
            rule path{std::monostate{}};
            rule port{std::monostate{}};
            bool short_circuit{false};

            static bool parse(const rule& rule, std::string_view input);
        };
    private:
        struct ScopeLookup
        {
            Scope scope{};
            absl::flat_hash_map<std::string, std::string> headers{};
        };
    private:
        mutable std::shared_mutex _mtx{};
        absl::flat_hash_map<std::string, ScopeLookup> _scopes{};
    public:
        using AbstractModule::AbstractModule;

        void set_scope(
            std::string_view name,
            Scope scope,
            absl::flat_hash_map<std::string, std::string> headers = {}
        );
        void set_header(std::string_view name, std::string_view header, std::string value);
        std::optional<std::string> get_header(std::string_view name, std::string_view header);

        void reset(std::string_view name);
        void reset();

        void process(
            std::string_view url,
            std::string_view method,
            std::function<void(std::string_view, std::string_view)> callback
        ) const;
    };
    using session = SystemSession;
} // namespace d2::sys::net
