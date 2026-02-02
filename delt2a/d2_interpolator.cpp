#include "d2_interpolator.hpp"

namespace d2::interp
{
    bool Interpolator::_nptr()
    {
        return
            !ptr_.owner_before(std::weak_ptr<Element> {}) &&
            !std::weak_ptr<Element> {}.owner_before(ptr_);
    }
    bool Interpolator::_vptr()
    {
        return _nptr() || !ptr_.expired();
    }
    auto Interpolator::_hold()
    {
        return ptr_.lock();
    }
    float Interpolator::_progress() const
    {
        return
            duration_.count() == 0 ? 1.f :
                float(std::chrono::duration_cast<std::chrono::milliseconds>((std::chrono::high_resolution_clock::now() -
                                                                             start_)).count()) /
                    float(duration_.count());
    }

    Interpolator::Interpolator(std::chrono::milliseconds ms, std::shared_ptr<Element> ptr)
        : duration_(ms), ptr_(ptr) {}

    Element* Interpolator::target() const
    {
        return ptr_.lock().get();
    }

    std::size_t Interpolator::hash_code()
    {
        return _hash_code_impl() ^ std::hash<Element*>()(ptr_.lock().get());
    }
    void Interpolator::stop()
    {
        start_ = start_.max();
    }
    void Interpolator::mute()
    {
        start_ = start_.min();
    }
    void Interpolator::unmute()
    {
        start_ = std::chrono::high_resolution_clock::now();
    }
    void Interpolator::start()
    {
        if (!_vptr()) return;
        const auto _ = _hold();
        _start_impl();
        start_ = std::chrono::high_resolution_clock::now();
    }
    void Interpolator::update()
    {
        if (start_ != std::chrono::high_resolution_clock::time_point::min())
        {
            if (!_vptr()) return;
            const auto ptr = _hold();
            _update_impl(ptr, _progress());
        }
    }
    bool Interpolator::keep_alive()
    {
        return start_ != start_.max() && _keep_alive_impl(_progress()) && _vptr();
    }

    void Interpolator::setowner(std::size_t hash)
    {
        owner_ = hash;
    }
    std::size_t Interpolator::getowner()
    {
        return owner_;
    }
}
