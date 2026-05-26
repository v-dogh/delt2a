#include "d2_interpolator.hpp"
#include "d2_styles_base.hpp"

#include <chrono>
#include <d2_exceptions.hpp>
#include <d2_tree_element.hpp>

namespace d2::interp
{
    bool Interpolator::_nptr()
    {
        return !_ptr.owner_before(std::weak_ptr<Element>{}) &&
               !std::weak_ptr<Element>{}.owner_before(_ptr);
    }
    bool Interpolator::_vptr()
    {
        return _nptr() || !_ptr.expired();
    }
    std::shared_ptr<Element> Interpolator::_hold()
    {
        return _ptr.lock();
    }
    float Interpolator::_progress() const
    {
        return _duration.count() == 0 ? 1.f
                                      : float(std::chrono::duration_cast<std::chrono::milliseconds>(
                                                  (std::chrono::steady_clock::now() - _start)
                                        )
                                                  .count()) /
                                            float(_duration.count());
    }

    Interpolator::Interpolator(
        std::chrono::milliseconds ms, std::shared_ptr<Element> ptr, style::uai_property prop
    ) : _duration(ms), _ptr(ptr), _prop(prop)
    {
    }

    std::pair<Element*, style::uai_property> Interpolator::target() const
    {
        return {_ptr.lock().get(), _prop};
    }

    void Interpolator::stop()
    {
        _start = _start.max();
    }
    void Interpolator::mute()
    {
        _start = _start.min();
    }
    void Interpolator::unmute()
    {
        _start = std::chrono::steady_clock::now();
    }
    void Interpolator::start()
    {
        if (!_vptr())
            return;
        const auto _ = _hold();
        _start_impl();
        _start = std::chrono::steady_clock::now();
    }
    std::chrono::milliseconds Interpolator::update()
    {
        if (_deadline > std::chrono::steady_clock::now())
            return std::chrono::duration_cast<std::chrono::milliseconds>(
                _deadline - std::chrono::steady_clock::now()
            );
        if (_start != std::chrono::steady_clock::time_point::min())
        {
            if (!_vptr())
                return std::chrono::milliseconds::max();
            const auto ptr = _hold();
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
    bool Interpolator::keep_alive()
    {
        return _start != _start.max() && _keep_alive_impl(_progress()) && _vptr();
    }
    std::chrono::milliseconds Interpolator::duration() const noexcept
    {
        return _duration;
    }
} // namespace d2::interp
