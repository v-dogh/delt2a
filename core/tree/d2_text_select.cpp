#include "d2_text_select.hpp"
#include <core/tree/d2_tree_parent.hpp>
#include <algorithm>
#include <limits>

namespace d2::style
{
    bool IVSelectable::selectable() const noexcept { return enable_selection; }
}

namespace d2
{
    namespace
    {
        style::IVSelectable* provider(const std::shared_ptr<Element>& element)
        {
            return dynamic_cast<style::IVSelectable*>(element.get());
        }
        std::shared_ptr<Element> hit(const std::shared_ptr<Element>& root, Position position)
        {
            if (!root || !root->getstate(Element::Display) ||
                !root->box().contains(position - root->position_screen_space()))
                return {};
            auto result = root;
            if (auto parent = std::dynamic_pointer_cast<ParentElement>(root))
            {
                std::shared_ptr<Element> child;
                parent->foreach_internal([&](Element::ptr element)
                {
                    if (element->getstate(Element::Display) &&
                        element->box().contains(position - element->position_screen_space()) &&
                        (!child || element->getzindex() > child->getzindex()))
                        child = element;
                    return true;
                });
                if (child && child->getzindex() > ParentElement::underlap)
                    result = hit(child, position);
            }
            return result;
        }
    }

    std::vector<TextSelection::Node> TextSelection::_collect(const std::shared_ptr<Element>& root)
    {
        std::vector<Node> nodes;
        if (!root)
            return nodes;
        auto visit = [&](auto&& self, const Element::ptr& element, Position clip, BoundingBox bounds) -> void
        {
            if (!element->getstate(Element::Display))
                return;
            const auto origin = element->position_screen_space();
            const auto box = element->box();
            const auto left = std::max(clip.x, origin.x);
            const auto top = std::max(clip.y, origin.y);
            const auto right = std::min<long long>(static_cast<long long>(clip.x) + bounds.width,
                                                   static_cast<long long>(origin.x) + box.width);
            const auto bottom = std::min<long long>(static_cast<long long>(clip.y) + bounds.height,
                                                    static_cast<long long>(origin.y) + box.height);
            clip = {left, top};
            bounds = {static_cast<int>(std::max(0LL, right - left)),
                      static_cast<int>(std::max(0LL, bottom - top))};
            if (auto text = provider(element); text && text->selectable())
                if (const auto layout = text->text_layout())
                    nodes.push_back({element, layout->revision(), origin, clip, bounds});
            if (auto parent = std::dynamic_pointer_cast<ParentElement>(element))
                parent->foreach_internal([&](Element::ptr child)
                {
                    self(self, child, clip, bounds);
                    return true;
                });
        };
        visit(visit, root, root->position_screen_space(), root->box());
        return nodes;
    }

    std::optional<TextSelection::Point> TextSelection::_hit(Position position, bool nearest) const
    {
        const auto target = hit(_root.lock(), position);
        std::optional<std::size_t> found;
        long long distance = std::numeric_limits<long long>::max();
        for (std::size_t i = 0; i < _nodes.size(); ++i)
        {
            const auto& node = _nodes[i];
            if (node.clip.empty())
                continue;
            if (node.element.lock() == target)
            {
                found = i;
                break;
            }
            if (nearest)
            {
                const auto dx = std::max({0LL, static_cast<long long>(node.clip_origin.x) - position.x,
                    static_cast<long long>(position.x) - node.clip_origin.x - node.clip.width + 1});
                const auto dy = std::max({0LL, static_cast<long long>(node.clip_origin.y) - position.y,
                    static_cast<long long>(position.y) - node.clip_origin.y - node.clip.height + 1});
                const auto next = dx + dy;
                if (next < distance)
                {
                    distance = next;
                    found = i;
                }
            }
        }
        if (!found)
            return {};
        const auto& node = _nodes[*found];
        const auto element = node.element.lock();
        if (!element)
            return {};
        return Point{*found, provider(element)->text_layout()->hit_test(position - node.origin)};
    }

    void TextSelection::_apply()
    {
        const auto first = std::min(_anchor, _focus);
        const auto last = std::max(_anchor, _focus);
        for (std::size_t i = 0; i < _nodes.size(); ++i)
        {
            if (const auto element = _nodes[i].element.lock())
            {
                auto text = provider(element);
                std::optional<text::Range> range;
                if (i >= first.node && i <= last.node && !empty())
                    range = text::Range{i == first.node ? first.grapheme : 0,
                                        i == last.node ? last.grapheme : text->text_layout()->size()};
                text->select_text(range);
            }
        }
    }
    bool TextSelection::refresh()
    {
        if (_nodes.empty())
            return false;
        auto current = _collect(_root.lock());
        bool valid = current.size() == _nodes.size();
        for (std::size_t i = 0; valid && i < current.size(); ++i)
            valid = current[i].element.lock() == _nodes[i].element.lock() &&
                    current[i].revision == _nodes[i].revision;
        if (!valid)
        {
            clear();
            return false;
        }
        _nodes = std::move(current);
        return true;
    }
    bool TextSelection::begin(const std::shared_ptr<Element>& root, Position position)
    {
        clear();
        _root = root;
        _nodes = _collect(root);
        if (auto point = _hit(position, false))
        {
            _anchor = _focus = *point;
            _dragging = true;
            return true;
        }
        clear();
        return false;
    }
    bool TextSelection::extend(Position position)
    {
        if (!_dragging || !refresh())
            return false;
        if (auto point = _hit(position, true))
        {
            _focus = *point;
            _apply();
            return true;
        }
        return false;
    }
    void TextSelection::select_all(const std::shared_ptr<Element>& root)
    {
        clear();
        _root = root;
        _nodes = _collect(root);
        if (!_nodes.empty())
        {
            _anchor = {};
            const auto last = _nodes.back().element.lock();
            _focus = {_nodes.size() - 1, provider(last)->text_layout()->size()};
            _apply();
        }
    }
    std::string TextSelection::value()
    {
        if (!refresh() || empty())
            return {};
        const auto first = std::min(_anchor, _focus);
        const auto last = std::max(_anchor, _focus);
        std::string result;
        for (auto i = first.node; i <= last.node; ++i)
        {
            const auto element = _nodes[i].element.lock();
            const auto text = provider(element);
            const auto layout = text->text_layout();
            result += layout->extract({i == first.node ? first.grapheme : 0,
                                       i == last.node ? last.grapheme : layout->size()});
            if (i != last.node)
                result += text->selection_separator();
        }
        return result;
    }
    void TextSelection::clear()
    {
        auto nodes = std::move(_nodes);
        _nodes.clear();
        _root.reset();
        _anchor = _focus = {};
        _dragging = false;
        for (const auto& node : nodes)
            if (const auto element = node.element.lock())
                if (auto text = provider(element))
                    text->select_text({});
    }
    TextSelection::~TextSelection()
    {
        try { clear(); }
        catch (...) {}
    }
} // namespace d2
