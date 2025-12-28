#ifndef D2_INTERPOLATOR_HPP
#define D2_INTERPOLATOR_HPP

#include "d2_tree_element.hpp"
#include <chrono>

namespace d2::interp
{
    namespace impl
    {
        template<typename Type, style::uai_property Property>
        using style_type = decltype(std::declval<Type>().template get<Property>());

        template<typename Type>
        class LinearInterpolationWriter
        {
        private:
            Type source_{};
            Type dest_{};
        public:
            LinearInterpolationWriter(Type source, Type dest)
                : source_(source), dest_(dest) {}

            void reset(const Type& source, const Type& dest)
            {
                source_ = source;
                dest_ = dest;
            }
            Type write(float progress)
            {
                return source_ + progress * (dest_ - source_);
            }
        };
        template<> class LinearInterpolationWriter<PixelBackground>
        {
        private:
            PixelBackground source_{};
            PixelBackground dest_{};
        public:
            LinearInterpolationWriter(PixelBackground source, PixelBackground dest)
                : source_(source), dest_(dest) {}

            void reset(const PixelBackground& source, const PixelBackground& dest)
            {
                source_ = source;
                dest_ = dest;
            }
            PixelBackground write(float progress)
            {
                return Pixel::interpolate(source_, dest_, progress);
            }
        };
        template<> class LinearInterpolationWriter<PixelForeground>
        {
        private:
            PixelForeground source_{};
            PixelForeground dest_{};
        public:
            LinearInterpolationWriter(PixelForeground source, PixelForeground dest)
                : source_(source), dest_(dest) {}

            void reset(const PixelForeground& source, const PixelForeground& dest)
            {
                source_ = source;
                dest_ = dest;
            }
            PixelForeground write(float progress)
            {
                PixelForeground value{};
                value.r = source_.r + progress * (dest_.r - source_.r);
                value.g = source_.g + progress * (dest_.g - source_.g);
                value.b = source_.b + progress * (dest_.b - source_.b);
                value.a = source_.a + progress * (dest_.a - source_.a);
                return value;
            }
        };
        template<> class LinearInterpolationWriter<Pixel>
        {
        private:
            Pixel source_{};
            Pixel dest_{};
        public:
            LinearInterpolationWriter(Pixel source, Pixel dest)
                : source_(source), dest_(dest) {}

            void reset(const Pixel& source, const Pixel& dest)
            {
                source_ = source;
                dest_ = dest;
            }
            Pixel write(float progress)
            {
                return Pixel::interpolate(source_, dest_, progress);
            }
        };
    }

    class Interpolator
    {
    public:
        using ptr = std::shared_ptr<Interpolator>;
    private:
        std::chrono::high_resolution_clock::time_point start_{};
        std::chrono::milliseconds duration_{};
        std::weak_ptr<Element> ptr_{};
        std::size_t owner_{};

        bool _nptr()
        {
            return
                !ptr_.owner_before(std::weak_ptr<Element> {}) &&
                !std::weak_ptr<Element> {}.owner_before(ptr_);
        }
        bool _vptr()
        {
            return _nptr() || !ptr_.expired();
        }
        auto _hold()
        {
            return ptr_.lock();
        }
    protected:
        float _progress() const
        {
            return
                duration_.count() == 0 ? 1.f :
                float(std::chrono::duration_cast<std::chrono::milliseconds>((std::chrono::high_resolution_clock::now() -
                        start_)).count()) /
                float(duration_.count());
        }

        virtual bool _keep_alive_impl(float progress) { return false; }
        virtual void _start_impl() {}
        virtual void _update_impl(Element::ptr ptr, float progress) {}
        // Has to return std::hash of the property that is the target of the interpolation
        virtual std::size_t _hash_code_impl() = 0;
    public:
        Interpolator() = default;
        Interpolator(std::chrono::milliseconds ms, Element::ptr ptr)
            : duration_(ms), ptr_(ptr) {}
        virtual ~Interpolator() = default;

        Element* target() const
        {
            return ptr_.lock().get();
        }

        std::size_t hash_code()
        {
            return _hash_code_impl() ^ std::hash<Element*>()(ptr_.lock().get());
        }
        void stop()
        {
            start_ = start_.max();
        }
        void mute()
        {
            start_ = start_.min();
        }
        void unmute()
        {
            start_ = std::chrono::high_resolution_clock::now();
        }
        void start()
        {
            if (!_vptr()) return;
            const auto _ = _hold();
            _start_impl();
            start_ = std::chrono::high_resolution_clock::now();
        }
        void update()
        {
            if (start_ != std::chrono::high_resolution_clock::time_point::min())
            {
                if (!_vptr()) return;
                const auto ptr = _hold();
                _update_impl(ptr, _progress());
            }
        }
        bool keep_alive()
        {
            return start_ != start_.max() && _keep_alive_impl(_progress()) && _vptr();
        }

        void setowner(std::size_t hash)
        {
            owner_ = hash;
        }
        std::size_t getowner()
        {
            return owner_;
        }
    };

    template<typename Type, style::uai_property Property>
    class Linear :
        public Interpolator,
        public impl::LinearInterpolationWriter<impl::style_type<Type, Property>>
    {
    protected:
        using dest_type = impl::style_type<Type, Property>;
    protected:
        virtual bool _keep_alive_impl(float progress) override
        {
            return progress < 1.f;
        }
        virtual void _update_impl(Element::ptr ptr, float progress) override
        {
            std::static_pointer_cast<Type>(ptr)->template set<Property>(
                impl::LinearInterpolationWriter<dest_type>::write(std::min(progress, 1.f)),
                true
            );
        }
        virtual std::size_t _hash_code_impl() override
        {
            return std::hash<style::uai_property>()(Property);
        }
    public:
        Linear(std::chrono::milliseconds ms, Element::ptr ptr, dest_type dest) :
            Interpolator(ms, ptr),
            impl::LinearInterpolationWriter<dest_type>(
                std::static_pointer_cast<Type>(ptr)->template get<Property>(), dest
            )
        {}
        virtual ~Linear()
        {
            update();
        }
    };

    template<typename Type, style::uai_property Property>
    class Sequential : public Interpolator
    {
    protected:
        using dest_type = impl::style_type<Type, Property>;
    private:
        std::vector<dest_type> stages_{};
        std::size_t idx{ ~0ull };
    protected:
        virtual bool _keep_alive_impl(float progress) override
        {
            return !stages_.empty();
        }
        virtual void _update_impl(Element::ptr ptr, float progress) override
        {
            if (progress > 1.f)
            {
                progress = 0.f;
                start();
            }

            const auto nidx = std::min(
                std::size_t(stages_.size() * progress),
                stages_.size() - 1
            );
            if (nidx == idx) return;
            idx = nidx;
            std::static_pointer_cast<Type>(ptr)
            ->template set<Property>(stages_[idx], true);
        }
        virtual std::size_t _hash_code_impl() override
        {
            return std::hash<style::uai_property>()(Property);
        }
    public:
        Sequential(std::chrono::milliseconds ms, Element::ptr ptr, std::initializer_list<dest_type> stages)
            : stages_(std::move(stages)), Interpolator(ms, ptr)
        {}
        template<typename Other>
        Sequential(std::chrono::milliseconds ms, Element::ptr ptr, Other&& stages)
            : stages_(std::forward<Other>(stages)), Interpolator(ms, ptr)
        {}
    };
}

#endif // D2_INTERPOLATOR_HPP
