#include "core/screen/d2_animate.hpp"

#include <chrono>
#include <core/utils/d2_exceptions.hpp>
#include <core/tree/d2_styles_base.hpp>
#include <core/tree/d2_tree_element.hpp>

namespace d2
{
    bool Animation::_nptr()
    {
        return !_ptr.owner_before(std::weak_ptr<Element>{}) &&
               !std::weak_ptr<Element>{}.owner_before(_ptr);
    }
    bool Animation::_vptr()
    {
        return _nptr() || !_ptr.expired();
    }
    std::pair<std::shared_ptr<Element>, std::shared_ptr<Animation>> Animation::_hold()
    {
        return {_ptr.lock(), shared_from_this()};
    }
    float Animation::_progress() const
    {
        return _duration.count() == 0 ? 1.f
                                      : float(std::chrono::duration_cast<std::chrono::milliseconds>(
                                                  (std::chrono::steady_clock::now() - _start)
                                        )
                                                  .count()) /
                                            float(_duration.count());
    }

    Animation::Animation(
        std::chrono::milliseconds ms, std::shared_ptr<Element> ptr, style::uai_property prop
    ) : _duration(ms), _ptr(ptr), _prop(prop)
    {
    }

    std::pair<std::weak_ptr<Element>, style::uai_property> Animation::target() const
    {
        return {_ptr, _prop};
    }

    void Animation::stop()
    {
        _start = _start.max();
    }
    void Animation::mute()
    {
        _start = _start.min();
    }
    void Animation::unmute()
    {
        _start = std::chrono::steady_clock::now();
    }
    void Animation::start()
    {
        if (!_vptr())
            return;
        const auto _ = _hold();
        _start_impl();
        _start = std::chrono::steady_clock::now();
    }
    std::chrono::milliseconds Animation::update()
    {
        if (_deadline > std::chrono::steady_clock::now())
            return std::chrono::duration_cast<std::chrono::milliseconds>(
                _deadline - std::chrono::steady_clock::now()
            );
        if (_start != std::chrono::steady_clock::time_point::min())
        {
            if (!_vptr())
                return std::chrono::milliseconds::max();
            const auto [ptr, _] = _hold();
            if (!ptr->getstate(Element::State::Display))
                return std::chrono::milliseconds::max();
            const auto prog = _progress();
            const auto wake = _update_impl(ptr, prog);
            if (wake == Wake::now)
                return Wake::refresh;
            else if (wake == Wake::never)
                return std::chrono::milliseconds::max();
            return std::clamp(
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    (wake * _duration) - (std::chrono::steady_clock::now() - _start)
                ),
                std::chrono::milliseconds(0),
                std::chrono::milliseconds::max()
            );
        }
        return std::chrono::milliseconds::max();
    }
    bool Animation::keep_alive()
    {
        return _start != _start.max() && _keep_alive_impl(_progress()) && _vptr();
    }
    std::chrono::milliseconds Animation::duration() const noexcept
    {
        return _duration;
    }
} // namespace d2
