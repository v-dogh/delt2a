#ifndef D2_MOD_BINDER_HPP
#define D2_MOD_BINDER_HPP

#include <d2_io_handler.hpp>
#include <d2_module.hpp>
#include <d2_screen.hpp>
#include <d2_tree_element.hpp>

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <absl/container/node_hash_map.h>

#include <chrono>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <vector>

namespace d2::sys
{
    class SystemBinder
        : public AbstractModule<"Binder">,
          public ConcreteModule<SystemBinder, Access::TUnsafe, Load::Lazy, sys::SystemScreen>
    {
        D2_TAG_MODULE(bdr)
    public:
        using key_list = std::vector<std::pair<in::keytype, in::mode>>;
        using control_list = absl::flat_hash_set<std::pair<in::keytype, std::optional<in::mode>>>;
        using callback_type = std::function<void(std::shared_ptr<IOContext>)>;
        struct Bind
        {
            enum class DepMode
            {
                Weak,
                Strong,
            };
            enum class DepCond
            {
                Visible,
                Active,
                Focused,
                Hovered
            };
            std::vector<key_list> sequences{};
            control_list whitelist{};
            control_list blacklist{};
            bool enable_whitelist{false};
            bool enable_blacklist{false};
            std::string description{};
            std::string display_name{};
            bool enabled{true};
            // Element | Required
            std::vector<std::tuple<std::weak_ptr<Element>, DepMode, DepCond>> dependencies{};
            std::chrono::milliseconds sequential_delay{std::chrono::milliseconds(100)};
            std::chrono::milliseconds hold_threshold{std::chrono::milliseconds(0)};
        };
        enum class ConflictType
        {
            Whitelist,
            Blacklist,
            Collision,
            Failure,
        };
        struct BindError
        {
            ConflictType type{ConflictType::Collision};
            std::string description{};
        };
    private:
        struct BindValue
        {
            Bind bind{};
            callback_type callback{};
            std::size_t epoch{0};
            std::size_t sequence_idx{0};
            std::chrono::steady_clock::time_point sequence_ts{};
            std::vector<d2::Element::EventListener> listeners{};
            bool active{false};
        };
        std::size_t _epoch_ctr{0};
        absl::flat_hash_map<std::string, BindValue> _binds{};
        absl::flat_hash_set<std::string> _active_set{};
        Signals::Handle _key_event{nullptr};

        std::optional<SystemBinder::BindError>
        _bind_dcheck(absl::flat_hash_map<std::string, BindValue>::iterator it);
        void _check_event();
    public:
        using AbstractModule::AbstractModule;

        std::optional<BindError> bind(const std::string& name, Bind value, callback_type callback);
        std::optional<BindError> rebind(const std::string& name, Bind value);
        const Bind& check(const std::string& name) const noexcept;
        void unbind(const std::string& name);
        void freeze(const std::string& name);
        void unfreeze(const std::string& name);
        std::vector<Bind> list() const;
    };
    using binder = SystemBinder;
} // namespace d2::sys

#endif
