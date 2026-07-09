#include "core/mods/d2_session.hpp"

#include <absl/strings/str_format.h>
#include <ada.h>

namespace d2::sys::net
{
    bool SystemSession::Scope::parse(const rule& rule, std::string_view input)
    {
        if (std::holds_alternative<std::regex>(rule))
        {
            const auto& reg = std::get<std::regex>(rule);
            return std::regex_match(input.begin(), input.end(), reg);
        }
        else
        {
            return input == std::get<std::string>(rule);
        }
    }

    void SystemSession::set_scope(
        std::string_view name, Scope scope, absl::flat_hash_map<std::string, std::string> headers
    )
    {
        std::lock_guard lock(_mtx);
        if (_scopes.contains(name))
            D2_THRW("Failed to set scope: '", name, "'", "; Already present");
        _scopes[name].scope = std::move(scope);
        for (const auto& [key, value] : headers)
            set_header(name, key, value);
    }
    void
    SystemSession::set_header(std::string_view name, std::string_view header, std::string value)
    {
        std::lock_guard lock(_mtx);
        auto f = _scopes.find(name);
        if (f == _scopes.end())
            D2_THRW("Invalid scope: '", name, "'");
        f->second.headers[header] = std::move(value);
    }
    std::optional<std::string>
    SystemSession::get_header(std::string_view name, std::string_view header)
    {
        std::shared_lock lock(_mtx);
        auto f = _scopes.find(name);
        if (f == _scopes.end())
            return std::nullopt;
        auto ff = f->second.headers.find(header);
        if (ff == f->second.headers.end())
            return std::nullopt;
        return ff->second;
    }

    void SystemSession::reset(std::string_view name)
    {
        std::lock_guard lock(_mtx);
        auto f = _scopes.find(name);
        if (f == _scopes.end())
            return;
        _scopes.erase(f);
    }
    void SystemSession::reset()
    {
        std::lock_guard lock(_mtx);
        _scopes.clear();
    }

    void SystemSession::process(
        std::string_view url,
        std::string_view method,
        std::function<void(std::string_view, std::string_view)> callback
    ) const
    {
        auto result = ada::parse<ada::url_aggregator>(url);
        if (!result)
        {
            D2_TLOG(Warning, "Failed to parse URL: '", url, "'");
            return;
        }

        std::shared_lock lock(_mtx);
        for (decltype(auto) it : _scopes)
        {
            auto& scope = it.second.scope;
            if (!scope.short_circuit && Scope::parse(scope.protocol, result->get_protocol()) &&
                Scope::parse(scope.method, method) &&
                Scope::parse(scope.domain, result->get_hostname()) &&
                Scope::parse(scope.path, result->get_pathname()) &&
                Scope::parse(scope.port, result->get_port()))
            {
                for (const auto& [key, value] : it.second.headers)
                    callback(key, value);
            }
        }
    }
} // namespace d2::sys::net
