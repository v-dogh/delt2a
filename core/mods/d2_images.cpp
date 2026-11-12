#include "d2_images.hpp"

#include <core/io/d2_context.hpp>
#include <core/screen/d2_screen.hpp>
#include <algorithm>

namespace d2::sys
{
    void SystemImages::configure(BoundingBox cell_pixels)
    {
        if (cell_pixels.width < 0 || cell_pixels.height < 0 ||
            bool(cell_pixels.width) != bool(cell_pixels.height))
            throw std::invalid_argument("Image cell metrics must be both zero or both positive");
        _cell_pixels = cell_pixels;
        _invalid = true;
        if (auto ctx = context())
            if (auto screen = ctx->sys_if<SystemScreen>(); screen && screen->root() != nullptr)
                internal::ElementView::from(screen->root()).signal_write(Element::WriteType::Style);
    }
    SystemImages::Plan SystemImages::prepare(std::span<pixel> cells, BoundingBox viewport,
                                              const image::Scene& scene, BoundingBox metrics)
    {
        if (viewport.width < 0 || viewport.height < 0 || cells.size() != std::size_t(viewport.width) * viewport.height)
            throw std::invalid_argument("Image output frame dimensions");
        if (_cell_pixels.width) metrics = _cell_pixels;
        if (metrics.width <= 0 || metrics.height <= 0)
        {
            metrics = _fallback_metrics();
            if (metrics.width <= 0 || metrics.height <= 0)
            {
                if (!scene.empty()) throw std::invalid_argument("Image output requires terminal cell pixel dimensions");
                metrics = {1, 1};
            }
        }
        const auto changed = _invalid || _scene != scene ||
                             _viewport != viewport || _composite.cell_pixels != metrics;
        auto composite = changed ? image::compose(scene, viewport, metrics) : image::Composite{};
        const auto& current = changed ? composite : _composite;
        for (int y = 0; y < current.cells.height; ++y)
            for (int x = 0; x < current.cells.width; ++x)
                if (current.occupied[std::size_t(y) * current.cells.width + x])
                {
                    auto& cell = cells[std::size_t(current.cells.y + y) * viewport.width + current.cells.x + x];
                    cell.v = ' ';
                    cell.style = px::style{};
                }
        const auto text_changed = _repaint_on_change() &&
            (_cells.size() != cells.size() || 
            !std::equal(_cells.begin(), _cells.end(), cells.begin()));
        const auto redraw = changed || (_repaint_on_change() && text_changed);
        Plan plan;
        if (redraw)
        {
            plan.repaint = _repaint_on_change() && (_placed || !current.empty());
            if (_placed || plan.repaint) 
                plan.before = _erase();
            if (!current.empty()) 
                plan.after = _encode(current, cells, viewport.width);
        }
        _placed = !current.empty();
        if (changed) 
        { 
            _composite = std::move(composite); 
            _scene = scene; 
        }
        _viewport = viewport;
        if (_repaint_on_change()) 
            _cells.assign(cells.begin(), cells.end());
        else 
            _cells.clear();
        _invalid = false;
        return plan;
    }
    SystemImages::Plan SystemImages::reset()
    {
        Plan plan;
        if (_placed) 
            plan.before = _erase();
        plan.repaint = _placed && _repaint_on_change();
        _placed = false;
        _scene.clear(); _composite = {}; _cells.clear();
        _invalid = true;
        return plan;
    }
    SystemImages::Status SystemImages::_load_impl()
    {
        configure({});
        return Status::Ok;
    }
    SystemImages::Status SystemImages::_unload_impl()
    {
        if (auto ctx = context())
            if (auto output = ctx->sys_if<SystemOutput>(); output)
                output->clear_images();
        return Status::Ok;
    }
}
