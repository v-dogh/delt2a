#ifndef D2_SCREEN_COMPS_HPP
#define D2_SCREEN_COMPS_HPP

#include <d2_input_base.hpp>
#include <d2_io_handler_frwd.hpp>
#include <d2_styles_base.hpp>
#include <d2_tree_element_frwd.hpp>

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <any>
#include <chrono>
#include <format>
#include <memory>
#include <string>

namespace d2
{
    // Standard tags:
    // SwapClean - if on, the entire tree is deleted when out of view and rebuilt later on
    // SwapOut - if on, the tree is swapped out when out of view
    // OverrideRefresh - when the tree is set, refresh rate is overriden with the value
    class TreeTags
    {
    private:
        absl::flat_hash_map<std::string, std::any> _tags{};
    public:
        TreeTags()
        {
            set<bool>("SwapClean", false);
            set<bool>("SwapOut", true);
            set<std::chrono::milliseconds>("OverrideRefresh", std::chrono::milliseconds(0));
        }
        TreeTags(const TreeTags&) = default;
        TreeTags(TreeTags&&) = default;

        template<typename Type> void set(const std::string& name, Type&& value)
        {
            _tags[name] = std::forward<Type>(value);
        }

        template<typename Type> Type tag(const std::string& name) const
        {
            auto f = _tags.find(name);
            if (f == _tags.end())
                throw std::logic_error{std::format("Attempt to access invalid tag: {}", name)};
            else if (f->second.type() != typeid(Type))
                throw std::logic_error{
                    std::format("Attempt to access tag through invalid type: {}", name)
                };
            return std::any_cast<Type>(f->second);
        }

        TreeTags& operator=(const TreeTags&) = default;
        TreeTags& operator=(TreeTags&&) = default;
    };
    class DynamicDependencyManager
    {
    private:
        absl::flat_hash_map<std::string, std::any> _deps{};
    public:
        template<typename Type> void set(const std::string& name, Type value)
        {
            using dtype = style::Theme::Dependency<std::remove_cvref_t<Type>>;
            auto [it, created] = _deps.emplace(name, std::any{});
            if (created)
                it->second.emplace<dtype>();
            std::any_cast<dtype&>(it->second) = std::forward<Type>(value);
        }
        template<typename Type> style::Theme::Dependency<Type>& get(const std::string& name)
        {
            using dtype = style::Theme::Dependency<std::remove_cvref_t<Type>>;
            if (auto f = _deps.find(name); f == _deps.end())
            {
                set(name, Type());
                return std::any_cast<dtype&>(_deps[name]);
            }
            else
                return std::any_cast<dtype&>(f->second);
        }
    };
} // namespace d2

#endif // D2_SCREEN_COMPS_HPP
