#include <d2_std.hpp>
#include <elements/d2_box.hpp>
#include <elements/d2_text.hpp>

namespace e0
{
    using hello = d2::Tree<
        d2::TreeState,
        d2::dx::Box,
        [](d2::TreeCtx<d2::dx::Box> ctx)
        {
            using namespace d2::dx;
            ctx.elem(
                [](d2::TreeCtx<Text> ctx)
                {
                    dstyle(Value, "Hello World!");
                    dstyle(X, 0.0_center);
                    dstyle(Y, 0.0_center);
                }
            );
        }
    >;

    inline void run()
    {
        try
        {
            d2::IOContext::make<
                //
                d2::sys::input,
                d2::sys::output,
                d2::sys::screen
            >()
                ->run<d2::NamedTree<"Hello", hello>>();
        }
        catch (const std::exception& ex)
        {
            std::cerr << ex.what() << std::endl;
        }
    }
} // namespace e0
