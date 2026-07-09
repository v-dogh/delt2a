#pragma once

#include "core/utils/d2_exceptions.hpp"
#include <chrono>
#include <core/types/d2_pixel.hpp>
#include <core/tree/d2_styles_base.hpp>
#include <core/tree/d2_tree_element_frwd.hpp>
#include <limits>

namespace d2
{
    namespace impl
    {
        template<typename Type, style::uai_property Property>
        using style_type = decltype(std::declval<Type>().template get<Property>());

        template<typename Type> class LinearInterpolationWriter
        {
        private:
            Type _source{};
            Type _dest{};
        public:
            LinearInterpolationWriter(Type source, Type dest) : _source(source), _dest(dest) {}

            void reset(const Type& source, const Type& dest)
            {
                _source = source;
                _dest = dest;
            }
            Type write(float progress)
            {
                return _source + progress * (_dest - _source);
            }
        };
        template<> class LinearInterpolationWriter<PixelBackground>
        {
        private:
            PixelBackground _source{};
            PixelBackground _dest{};
        public:
            LinearInterpolationWriter(PixelBackground source, PixelBackground dest) :
                _source(source), _dest(dest)
            {
            }

            void reset(const PixelBackground& source, const PixelBackground& dest)
            {
                _source = source;
                _dest = dest;
            }
            PixelBackground write(float progress)
            {
                return Pixel::interpolate(_source, _dest, progress);
            }
        };
        template<> class LinearInterpolationWriter<PixelForeground>
        {
        private:
            PixelForeground _source{};
            PixelForeground _dest{};
        public:
            LinearInterpolationWriter(PixelForeground source, PixelForeground dest) :
                _source(source), _dest(dest)
            {
            }

            void reset(const PixelForeground& source, const PixelForeground& dest)
            {
                _source = source;
                _dest = dest;
            }
            PixelForeground write(float progress)
            {
                PixelForeground value{};
                value.r = _source.r + progress * (_dest.r - _source.r);
                value.g = _source.g + progress * (_dest.g - _source.g);
                value.b = _source.b + progress * (_dest.b - _source.b);
                value.a = _source.a + progress * (_dest.a - _source.a);
                return value;
            }
        };
        template<> class LinearInterpolationWriter<Pixel>
        {
        private:
            Pixel _source{};
            Pixel _dest{};
        public:
            LinearInterpolationWriter(Pixel source, Pixel dest) : _source(source), _dest(dest) {}

            void reset(const Pixel& source, const Pixel& dest)
            {
                _source = source;
                _dest = dest;
            }
            Pixel write(float progress)
            {
                return Pixel::interpolate(_source, _dest, progress);
            }
        };
    } // namespace impl

    class Animation : public std::enable_shared_from_this<Animation>
    {
        D2_TAG_MODULE(anim)
    public:
        struct Wake
        {
            static constexpr auto now = std::numeric_limits<float>::infinity();
            static constexpr auto never = std::numeric_limits<float>::max();
            static constexpr auto refresh = std::chrono::milliseconds{-1};
        };
        using ptr = std::shared_ptr<Animation>;
    private:
        std::chrono::steady_clock::time_point _start{};
        std::chrono::steady_clock::time_point _deadline{};
        std::chrono::milliseconds _duration{};
        std::weak_ptr<Element> _ptr{};
        style::uai_property _prop{};

        bool _nptr();
        bool _vptr();
        std::pair<std::shared_ptr<Element>, std::shared_ptr<Animation>> _hold();
    protected:
        float _progress() const;
        virtual void _start_impl() {}
        virtual float _update_impl(std::shared_ptr<Element> ptr, float progress) = 0;
        virtual bool _keep_alive_impl(float progress)
        {
            return false;
        }
    public:
        Animation(
            std::chrono::milliseconds ms, std::shared_ptr<Element> ptr, style::uai_property prop
        );
        virtual ~Animation() = default;

        std::pair<std::weak_ptr<Element>, style::uai_property> target() const;

        void stop();
        void mute();
        void unmute();
        void start();
        bool keep_alive();
        std::chrono::milliseconds update();
        std::chrono::milliseconds duration() const noexcept;
    };

    namespace anim
    {
        template<typename Type, style::uai_property Property>
        class Linear : public Animation,
                       public impl::LinearInterpolationWriter<impl::style_type<Type, Property>>
        {
        private:
            using dest_type = impl::style_type<Type, Property>;
        protected:
            virtual bool _keep_alive_impl(float progress) override
            {
                return progress < 1.f;
            }
            virtual float _update_impl(std::shared_ptr<Element> ptr, float progress) override
            {
                std::static_pointer_cast<Type>(ptr)->template set<Property, true>(
                    impl::LinearInterpolationWriter<dest_type>::write(std::min(progress, 1.f))
                );
                return Wake::now;
            }
        public:
            Linear(std::chrono::milliseconds ms, std::shared_ptr<Element> ptr, dest_type dest) :
                Animation(ms, ptr, Property),
                impl::LinearInterpolationWriter<dest_type>(
                    std::static_pointer_cast<Type>(ptr)->template get<Property>(), dest
                )
            {
            }
            virtual ~Linear()
            {
                update();
            }
        };

        template<typename Type, style::uai_property Property> class Sequential : public Animation
        {
        private:
            using dest_type = impl::style_type<Type, Property>;
        private:
            std::vector<dest_type> _stages{};
            std::size_t _idx{~0ull};
        protected:
            virtual bool _keep_alive_impl(float progress) override
            {
                return !_stages.empty();
            }
            virtual float _update_impl(std::shared_ptr<Element> ptr, float progress) override
            {
                if (progress >= 1.f)
                {
                    progress = 0.f;
                    start();
                }

                const auto count = _stages.size();
                const auto nidx =
                    std::min<std::size_t>(static_cast<std::size_t>(count * progress), count - 1);

                const auto changed = nidx != _idx;
                if (changed)
                {
                    _idx = nidx;
                    std::static_pointer_cast<Type>(ptr)->template set<Property, true>(
                        _stages[_idx]
                    );
                }
                return float(nidx + 1) / float(count);
            }
        public:
            Sequential(
                std::chrono::milliseconds ms,
                std::shared_ptr<Element> ptr,
                std::initializer_list<dest_type> stages
            ) : _stages(std::move(stages)), Animation(ms, ptr, Property)
            {
            }
            template<typename Other>
            Sequential(std::chrono::milliseconds ms, std::shared_ptr<Element> ptr, Other&& stages) :
                _stages(std::forward<Other>(stages)), Animation(ms, ptr, Property)
            {
            }
        };
    } // namespace anim
} // namespace d2
