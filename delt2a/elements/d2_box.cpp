#include "d2_box.hpp"

namespace d2::dx
{
    int Box::_perfect_width() const noexcept
    {
        const auto bw = (data::container_options & ContainerOptions::EnableBorder) ?
                        resolve_units(data::border_width) : 0;
        int perfect = 0;
        for (decltype(auto) it : _elements)
        {
            if (it->contextual_layout(Element::Layout::Width))
            {
                continue;
            }
            else if (it->contextual_layout(Element::Layout::X))
            {
                perfect = std::max(perfect, it->layout(Element::Layout::Width));
            }
            else
            {
                const auto xpos = it->layout(Element::Layout::X) - (bw * it->getzindex() < overlap);
                const auto width = it->layout(Element::Layout::Width);
                perfect = std::max(perfect, xpos + width);
            }
        }
        return perfect + (bw * 2) + data::width.raw();
    }
    int Box::_perfect_height() const noexcept
    {
        const auto bw = (data::container_options & ContainerOptions::EnableBorder) ?
                        resolve_units(data::border_width) : 0;
        int perfect = 0;
        for (decltype(auto) it : _elements)
        {
            if (it->contextual_layout(Element::Layout::Height))
            {
                continue;
            }
            else if (it->contextual_layout(Element::Layout::Y))
            {
                perfect = std::max(perfect, it->layout(Element::Layout::Height));
            }
            else
            {
                const auto ypos = it->layout(Element::Layout::Y) - (bw * it->getzindex() < overlap);
                const auto height = it->layout(Element::Layout::Height);
                perfect = std::max(perfect, ypos + height);
            }
        }
        return perfect + (bw * 2) + data::height.raw();
    }

    int Box::_get_border_impl(BorderType type, cptr elem) const noexcept
    {
        if (
            (data::container_options & ContainerOptions::EnableBorder) &&
            elem->getzindex() < overlap
        )
            return resolve_units(data::border_width);
        return 0;
    }
    Unit Box::_layout_impl(enum Element::Layout type) const noexcept
    {
        switch (type)
        {
            case Element::Layout::X: return data::x;
            case Element::Layout::Y: return data::y;
            case Element::Layout::Width:
                if (data::width.getunits() == Unit::Auto)
                    return _perfect_width();
                return data::width;
            case Element::Layout::Height:
                if (data::height.getunits() == Unit::Auto)
                    return _perfect_height();
                return data::height;
        }
    }

    void Box::_frame_impl(PixelBuffer::View buffer) noexcept
    {
        // Fill in the gaps

        buffer.fill(Pixel::combine(data::foreground_color, data::background_color));

        // Fleshbox

        std::vector<Element*> ptrs{};
        std::size_t idx = 0;
        {
            ptrs.reserve(_elements.size());
            foreach_internal([&ptrs](ptr ptr) { ptrs.push_back(ptr.get()); });
            std::sort(ptrs.begin(), ptrs.end(), [](const auto& a, const auto& b) { return a->getzindex() < b->getzindex(); });

            _render_start();

            // Border is zindex = overlap (relative) so we defer the rest to after we render the border
            while (idx < ptrs.size() && ptrs[idx]->getzindex() < overlap)
            {
                auto& obj = *ptrs[idx];
                if (obj.getstate(Display))
                    _object_render(buffer, obj.shared_from_this());
                idx++;
            }
        }

        ContainerHelper::_render_border(buffer);

        // Render the rest of the objects
        {
            while (idx < ptrs.size())
            {
                auto& obj = *ptrs[idx];
                if (obj.getstate(Display))
                    _object_render(buffer, obj.shared_from_this());
                idx++;
            }
        }
    }

    void Box::_object_render(PixelBuffer::View buffer, ptr obj)
    {
        // Mezon the monstrous
        // It is high time
        const auto [ x, y ] = obj->position();
        const auto [ width, height ] = obj->box();
        const auto w = static_cast<int>(buffer.width());
        const auto h = static_cast<int>(buffer.height());

        if (x - width <= w &&
                y - height <= h)
        {
            const auto f = obj->frame();
            buffer.inscribe(x, y, f.buffer());
        }
    }
}
