#include "elements/d2_image.hpp"

namespace d2::dx
{
    mt::co_future<bool> Image::load(std::filesystem::path path, image::LoadOptions options, const std::string& loader)
    {
        auto ctx = state() ? context() : nullptr;
        if (!ctx || !ctx->is_synced()) 
            throw std::logic_error("Image::load requires the active UI thread");
        auto decoder = loader.empty() ? ctx->sys<sys::SystemImageLoader>() : ctx->sys<sys::SystemImageLoader>(loader);
        auto pending = decoder->load_async(std::move(path), options);
        _load_ticket = std::make_shared<const char>(0);
        return _finish_load(std::static_pointer_cast<Image>(shared_from_this()), ctx, _load_ticket, std::move(pending));
    }
    mt::co_future<bool> Image::_finish_load(std::weak_ptr<Image> target, IOContext::wptr context,
        std::shared_ptr<const char> ticket, mt::future<image::Raster::ptr> pending)
    {
        auto raster = co_await pending;
        if (!raster) 
            throw std::runtime_error("Image loader returned a null raster");
        mt::future<bool> applied;
        {
            auto ctx = context.lock();
            if (!ctx) 
                co_return false;
            applied = ctx->sync_async([target, context, ticket, raster = std::move(raster)]
            {
                auto image = target.lock();
                auto ctx = context.lock();
                if (!ctx || !image || !image->state() || image->_load_ticket != ticket || image->context() != ctx) 
                    return false;
                image->set<Source>(raster);
                return true;
            });
        }
        co_return co_await applied;
    }
    void Image::cancel_load()
    {
        if (auto ctx = state() ? context() : nullptr; ctx && !ctx->is_synced())
            throw std::logic_error("Image::cancel_load requires the UI thread");
        _load_ticket.reset();
    }
    void Image::_signal_base_impl(write_flag type, style::uai_property prop)
    {
        if (prop == Source) _load_ticket.reset();
        data::_signal_base_impl(type, prop);
    }
    void Image::_frame_impl(PixelBuffer::View buffer)
    {
        buffer.fill(pixel::combine(data::foreground_color, data::background_color));
        if (data::source)
        {
            const auto begin = _border_base(), end = _border_base_inv();
            const auto left = std::clamp(begin.x, 0, buffer.width());
            const auto top = std::clamp(begin.y, 0, buffer.height());
            const auto width = std::max(0, buffer.width() - left - std::clamp(end.x, 0, buffer.width()));
            const auto height = std::max(0, buffer.height() - top - std::clamp(end.y, 0, buffer.height()));
            if (width && height)
                buffer.image(data::source, {left, top, width, height}, data::source_rect);
        }
        ContainerHelper::_render_border(buffer);
    }
}
