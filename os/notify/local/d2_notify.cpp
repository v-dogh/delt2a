#include "d2_notify.hpp"

#include <elements/d2_std.hpp>
#include <extras/d2_default_styles.hpp>
#include <extras/d2_default_theme.hpp>

namespace d2::sys
{
    LocalSystemNotify::Status LocalSystemNotify::_unload_impl()
    {
        if (_display != nullptr)
            context()->sync([this]() { _display->remove(); });
        return Status::Ok;
    }

    void LocalSystemNotify::_create_display_if()
    {
        if (_display == nullptr)
        {
            using namespace d2::dx;
            d2::TreeCtx ctx(context()->screen()->root());
            _display = ctx.elem(
                [&](d2::TreeCtx<FlowBox> ctx)
                {
                    dstyle(Width, 0.25_pc);
                    dstyle(Height, 0.0_auto);
                    dstyle(X, 0.0_pxi);
                    dstyle(Y, 0.0_px);

                    ctx.persist();
                }
            );
        }
    }
    void LocalSystemNotify::_handle_prompt(
        Level level, std::string title, std::string content, Element::ptr view, bool soft
    )
    {
    }
    void LocalSystemNotify::_handle_message(
        Level level, std::string title, std::string content, Element::ptr view
    )
    {
        context()->sync(
            [=, this, ptr = ptr()]()
            {
                _create_display_if();

                using namespace d2::dx;
                using namespace d2::ex;

                d2::TreeCtx ctx(_display);
                auto root = ctx.anchor<FlowBox>();
                auto msg = ctx.elem(
                    root,
                    [&](d2::TreeCtx<FlowBox> ctx)
                    {
                        dapply(df::border);
                        dstyle(Width, 1.0_pc);
                        dstyle(Y, 0.0_relative);

                        ctx.elem(
                            [&](d2::TreeCtx<Text> ctx)
                            {
                                dapply(df::bold_text);
                                dstyle(Value, title);
                                dstyle(TextAlignment, Text::Alignment::Left);
                                dstyle(Y, 0.0_relative);
                            }
                        );
                        if (view)
                        {
                            ctx->create(view);
                            view->set_for<style::ILayout, style::IZLayout::Y>(0.0_relative);
                        }
                        else
                        {
                            ctx.elem(
                                [&](d2::TreeCtx<Text> ctx)
                                {
                                    dapply(df::text);
                                    dstyle(Value, content);
                                    dstyle(TextAlignment, Text::Alignment::Left);
                                    dstyle(Y, 0.0_relative);
                                }
                            );
                        }

                        auto discard = ctx.anchor<Button>();

                        ctx.elem(
                            discard,
                            [&](d2::TreeCtx<Button> ctx)
                            {
                                dapply(df::button);
                                dstyle(Value, d2::charset::icon::cross);
                                dstyle(X, 1.0_pxi);
                                dstyle(Y, 0.0_px);
                                dstyle(OnSubmit, [=](d2::TreeIter<Button>) { root->remove(); });

                                ctx->setstate(d2::Element::Display, false);
                            }
                        );

                        ctx.onv(
                            d2::Element::State::RcHover,
                            [=](d2::Element::EventListener, d2::TreeIter<> ptr)
                            {
                                discard->setstate(
                                    d2::Element::Display, ptr->getstate(d2::Element::RcHover)
                                );
                            }
                        );
                    }
                );

                std::chrono::milliseconds delay;
                switch (level)
                {
                case Level::Info:
                    delay = std::chrono::seconds(4);
                    break;
                case Level::Warning:
                    delay = std::chrono::seconds(8);
                    break;
                case Level::Error:
                    delay = std::chrono::seconds(16);
                    break;
                };

                ctx.defer(
                    delay,
                    [=]()
                    {
                        if (msg != nullptr)
                            msg->remove();
                    }
                );
            }
        );
    }

    bool LocalSystemNotify::supports(unsigned char mode)
    {
        return !(mode & Mode::System);
    }

    void LocalSystemNotify::notify(
        Level level, std::string_view title, std::string_view content, unsigned char targets
    )
    {
        if (!supports(targets))
            D2_THRW("Mode not supported");
        if (targets & Mode::Passive)
            _handle_message(level, std::string(title), std::string(content), nullptr);
        if ((targets & Mode::Prompt) || (targets & Mode::SoftPrompt))
            _handle_prompt(
                level, std::string(title), std::string(content), nullptr, targets & Mode::SoftPrompt
            );
    }
    void LocalSystemNotify::notify(
        Level level,
        std::string_view title,
        std::string_view content,
        Element::ptr view,
        unsigned char targets
    )
    {
        if (!supports(targets))
            D2_THRW("Mode not supported");
        if (targets & Mode::Passive)
            _handle_message(level, std::string(title), "", view);
        if ((targets & Mode::Prompt) || (targets & Mode::SoftPrompt))
            _handle_prompt(level, std::string(title), "", view, targets & Mode::SoftPrompt);
    }

    void LocalSystemNotify::clear() {}
} // namespace d2::sys
