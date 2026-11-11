#pragma once

#include <core/platform/d2_text_layout.hpp>
#include <memory>

namespace d2 { class Element; }

namespace d2::style
{
    class IVSelectable
    {
    protected:
        bool enable_selection{true};
        std::optional<text::Range> _text_selection{};
    public:
        virtual ~IVSelectable() = default;
        bool selectable() const noexcept;
        virtual const text::Layout* text_layout() const { return nullptr; }
        virtual void select_text(std::optional<text::Range> range) { _text_selection = range; }
        virtual std::string_view selection_separator() const { return "\n"; }
        std::optional<text::Range> text_selection() const noexcept { return _text_selection; }
    };
} // namespace d2::style

namespace d2
{
    class TextSelection
    {
    private:
        struct Node
        {
            std::weak_ptr<Element> element{};
            std::size_t revision{0};
            Position origin{};
            Position clip_origin{};
            BoundingBox clip{};
        };
        struct Point
        {
            std::size_t node{0};
            std::size_t grapheme{0};
            auto operator<=>(const Point&) const = default;
        };
        std::weak_ptr<Element> _root{};
        std::vector<Node> _nodes{};
        Point _anchor{}, _focus{};
        bool _dragging{false};

        static std::vector<Node> _collect(const std::shared_ptr<Element>& root);
        std::optional<Point> _hit(Position position, bool nearest) const;
        void _apply();
    public:
        TextSelection() = default;
        TextSelection(const TextSelection&) = delete;
        TextSelection& operator=(const TextSelection&) = delete;
        ~TextSelection();

        bool begin(const std::shared_ptr<Element>& root, Position position);
        bool extend(Position position);
        void finish() noexcept { _dragging = false; }
        bool dragging() const noexcept { return _dragging; }
        bool refresh();
        bool empty() const noexcept { return _nodes.empty() || _anchor == _focus; }
        std::string value();
        void select_all(const std::shared_ptr<Element>& root);
        void clear();
    };
} // namespace d2
