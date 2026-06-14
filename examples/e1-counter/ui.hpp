#include <d2_std.hpp>

#include <elements/d2_button.hpp>
#include <elements/d2_flow_box.hpp>
#include <elements/d2_text.hpp>

namespace e1
{
    using counter = d2::Tree<
        d2::TreeState,
        d2::dx::FlowBox,
        [](d2::TreeCtx<d2::dx::FlowBox> ctx)
        {
            using namespace d2::dx;

            auto& ctr = ctx.dep<int>("ctr");
            ctr = 0;

            ctx.elem(
                [&](d2::TreeCtx<FlowBox> ctx)
                {
                    dstyle(X, 0.0_center);
                    dstyle(Y, 0.0_center);
                    dstyle(Height, 1.0_auto);
                    dstyle(ContainerBorder, true);

                    ctx.elem(
                        [](d2::TreeCtx<Text> ctx)
                        {
                            dstyle(Value, "Counter  Demo");
                            dstyle(Width, 2.0_auto);
                            dstyle(X, 0.0_center);
                            dstyle(Y, 1.0_relative);
                        }
                    );
                    ctx.elem(
                        [&](d2::TreeCtx<Text> ctx)
                        {
                            dstyle(Value, "Count: <0>");
                            dstyle(X, 0.0_center);
                            dstyle(Y, 1.0_relative);

                            ctx.subscribe(
                                   ctr,
                                   [=](d2::TreeIter<Text>, int value)
                                   { ctx->set<Text::Value>(std::format("Count: <{}>", value)); }
                            ).release();
                        }
                    );
                    ctx.elem(
                        [&](d2::TreeCtx<FlowBox> ctx)
                        {
                            using button = d2::Stylesheet<
                                []<typename Type>(d2::TreeCtx<Type> ctx, char value, bool first)
                                {
                                    dstyle(Value, std::format("[{}]", value));
                                    dstyle(X, first ? 0.0_relative : 1.0_relative);
                                    dstyle(ForegroundColor, d2::colors::w::white);

                                    ctx.onv(
                                        d2::Element::State::Clicked,
                                        [=](d2::Element::EventListener, d2::TreeIter<Type> ptr)
                                        {
                                            d2::TreeCtx ctx(ptr);
                                            if (ptr->getstate(d2::Element::Clicked))
                                                dstyle(Value, std::format(">{}<", value));
                                            else
                                                dstyle(Value, std::format("[{}]", value));
                                        }
                                    );
                                },
                                char,
                                bool
                            >;

                            dstyle(X, 0.0_center);
                            dstyle(Y, 1.0_relative);

                            ctx.elem(
                                [&](d2::TreeCtx<Button> ctx)
                                {
                                    dapply(button, '+', true);
                                    dstyle(OnSubmit, [&ctr](d2::TreeIter<Button>) { ctr++; });
                                }
                            );
                            ctx.elem(
                                [&](d2::TreeCtx<Button> ctx)
                                {
                                    dapply(button, '=', false);
                                    dstyle(OnSubmit, [&ctr](d2::TreeIter<Button>) { ctr = 0; });
                                }
                            );
                            ctx.elem(
                                [&](d2::TreeCtx<Button> ctx)
                                {
                                    dapply(button, '-', false);
                                    dstyle(OnSubmit, [&ctr](d2::TreeIter<Button>) { ctr--; });
                                }
                            );
                        }
                    );
                }
            );
        }
    >;

    inline void run()
    {
        try
        {
            d2::IOContext::make<d2::sys::input, d2::sys::output, d2::sys::screen>()->run<counter>();
        }
        catch (const std::exception& ex)
        {
            std::cerr << ex.what() << std::endl;
        }
    }
} // namespace e1
