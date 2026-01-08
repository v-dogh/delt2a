#ifndef D2_GRAPH_HPP
#define D2_GRAPH_HPP

#include "../d2_tree_element.hpp"
#include "../d2_styles.hpp"
#include "../d2_colors.hpp"
#include "d2_element_utils.hpp"

namespace d2
{
    namespace style
    {
        D2_UAI_INTERFACE(RangeGraph,
            D2_UAI_OPTS(),
            D2_UAI_FIELDS(
                Pixel max_color{ .a = 0, .v = '.' };
                Pixel min_color{ .a = 0, .v = '.' };
                int resolution{ 0 };
                bool automatic_resolution{ true };
                bool high_resolution{ true };
                bool invert{ false };
            ),
            D2_UAI_PROPS(
                MaxColor,
                MinColor,
                Resolution,
                AutomaticResolution,
                HighResolution,
                Invert
            ),
            D2_UAI_LINK(
                D2_UAI_PROP(MaxColor, max_color, Style)
                D2_UAI_PROP(MinColor, min_color, Style)
                D2_UAI_PROP(Resolution, resolution, Style)
                D2_UAI_PROP(AutomaticResolution, automatic_resolution, Style)
                D2_UAI_PROP(HighResolution, high_resolution, Style)
                D2_UAI_PROP(Invert, invert, Style)
            )
        )
        D2_UAI_INTERFACE(NodeWaveGraph,
            D2_UAI_OPTS(),
            D2_UAI_FIELDS(
                std::chrono::milliseconds resolution{ 1000 };
                std::chrono::milliseconds frequency{ 1000 };
                PixelForeground node_color_i{ PixelForeground(colors::g::green) };
                PixelForeground node_color_a{ PixelForeground(colors::b::blue) };
                PixelForeground node_color_b{ PixelForeground(colors::r::red) };
                PixelForeground node_color_focused{ PixelForeground(colors::g::olive) };
            ),
            D2_UAI_PROPS(
                Resolution,
                Frequency,
                NodeColorI,
                NodeColorA,
                NodeColorB,
                NodeColorFocused
            ),
            D2_UAI_LINK(
                D2_UAI_PROP(Resolution, resolution, Style)
                D2_UAI_PROP(Frequency, frequency, Style)
                D2_UAI_PROP(NodeColorI, node_color_i, Style)
                D2_UAI_PROP(NodeColorA, node_color_a, Style)
                D2_UAI_PROP(NodeColorB, node_color_b, Style)
                D2_UAI_PROP(NodeColorFocused, node_color_focused, Style)
            )
        )
    }
    namespace dx::graph
    {
        class CyclicVerticalBar :
            public style::UAI<
                CyclicVerticalBar,
                style::ILayout,
                style::IContainer,
                style::IColors,
                style::IRangeGraph
            >,
            public impl::ContainerHelper<CyclicVerticalBar>
        {
        public:
            friend class ContainerHelper<CyclicVerticalBar>;
            using data = style::UAI<
                         CyclicVerticalBar,
                         style::ILayout,
                         style::IContainer,
                         style::IColors,
                         style::IRangeGraph
                         >;
            using data::data;

        protected:
            std::vector<float> _data{};
            int _start_offset = 0;

            virtual void _frame_impl(PixelBuffer::View buffer) override;

            std::pair<int, int> _resolution() const;
        public:
            void clear();
            void push(float data_point);
            void rescale(float scale);
            void rescale(float scale, float trans);
        };

        class NodeWaveGraph :
            public d2::style::UAI<
                NodeWaveGraph,
                d2::style::ILayout,
                d2::style::IColors,
                d2::style::INodeWaveGraph
            >
        {
        public:
            using data = d2::style::UAI<
                NodeWaveGraph,
                d2::style::ILayout,
                d2::style::IColors,
                d2::style::INodeWaveGraph
            >;
            struct NodePair
            {
                std::size_t id{ 0 };
                float start{ 0.f };
                float end{ 1.f };
                float phase{ 0.f };
                float frequency{ 1.f };
                std::vector<float> parameters;
                std::function<float(float, float, float, std::span<const float>)> interpolator{
                    [](float phase, float frequency, float pos, auto) {
                        return 0.f;
                    }
                };
            };
        private:
            NodePair* _focused_node{ nullptr };
            std::size_t _start_point{ ~0ull };
            std::size_t _end_point{ ~0ull };
            std::size_t _id{ 0 };
            std::list<NodePair> _nodes{};
            float _offset{ 0.f };

            static float _lerp(float t, float a, float b);
        protected:
            virtual void _state_change_impl(State state, bool value) override;
            virtual void _frame_impl(d2::PixelBuffer::View buffer) override;
        public:
            using data::data;

            std::size_t push();
            std::size_t push(float start, float end);
            void pop(std::size_t id);
            void set_parameters(std::size_t id, const NodePair& node);
        };
    }
}

#endif // D2_GRAPH_HPP
