#ifndef D2_THEME_SELECTOR_HPP
#define D2_THEME_SELECTOR_HPP

#include "../elements/d2_flow_box.hpp"
#include <variant>

namespace d2
{
    namespace style
    {
        D2_UAI_INTERFACE(ThemeSelector,
            D2_UAI_OPTS(
                using accent_vec = std::vector<px::background>;
                using theme = std::variant<accent_vec, string>;
            ),
            D2_UAI_FIELDS(
                std::vector<std::pair<string, accent_vec>> themes{};
                int max_accents{ 0 };
            ),
            D2_UAI_PROPS(
                Themes,
                MaxAccents
            ),
            D2_UAI_LINK(
                D2_UAI_PROP(Themes, themes, Masked)
                D2_UAI_PROP(MaxAccents, max_accents, None)
            )
        )
    }
    namespace ctm
    {
        namespace impl
        {
            class ThemeSelectorBase
            {
            protected:
                style::IZThemeSelector::accent_vec _accents{};
            public:
                virtual ~ThemeSelectorBase() = default;

                void push(px::background accent);
                void clear();

                virtual void submit() = 0;
                virtual void submit(const string& name) = 0;
            };
        }

        class ThemeSelectorWindow :
            public style::UAIC<
                dx::VirtualFlowBox,
                ThemeSelectorWindow,
                style::IThemeSelector,
                style::IResponsive<ThemeSelectorWindow, style::IZThemeSelector::theme>::type
            >,
            public impl::ThemeSelectorBase
        {
        public:
            using data = style::UAIC<
                VirtualFlowBox,
                ThemeSelectorWindow,
                style::IThemeSelector,
                style::IResponsive<ThemeSelectorWindow, style::IZThemeSelector::theme>::type
            >;
            D2_UAI_CHAIN(VirtualFlowBox)
        protected:
            virtual void _state_change_impl(State state, bool value) override;
            virtual void _signal_write_impl(write_flag type, unsigned int prop, ptr element) override;

            void _update_theme_list();
        public:
            virtual void submit(const std::string& name) override;
            virtual void submit() override;
        };
        class ThemeSelector :
            public style::UAIC<
                dx::FlowBox,
                ThemeSelector,
                style::IThemeSelector,
                style::IResponsive<ThemeSelector, style::IZThemeSelector::theme>::type
            >,
            public impl::ThemeSelectorBase
        {
        public:
            using data = style::UAIC<
                FlowBox,
                ThemeSelector,
                style::IThemeSelector,
                style::IResponsive<ThemeSelector, style::IZThemeSelector::theme>::type
            >;
            D2_UAI_CHAIN(FlowBox)
        protected:
            virtual void _state_change_impl(State state, bool value) override;
            virtual void _signal_write_impl(write_flag type, unsigned int prop, ptr element) override;

            void _update_theme_list();
        public:
            virtual void submit(const std::string& name) override;
            virtual void submit() override;
        };
    }
}

#endif // D2_THEME_SELECTOR_HPP
