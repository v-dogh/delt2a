#ifndef D2_THEME_SELECTOR_HPP
#define D2_THEME_SELECTOR_HPP

#include <elements/d2_flow_box.hpp>
#include <variant>

namespace d2
{
    namespace style
    {
        D2_UAI_INTERFACE(
            ThemeSelector,
            D2_UAI_OPTS(
                using accent_vec = std::vector<px::background>;
                using theme = std::variant<accent_vec, string>;
                using theme_list = std::vector<std::pair<std::pair<string, string>, accent_vec>>;
            ),
            D2_UAI_FIELDS(
                // Name, Collection, Accents
                theme_list themes{}; int max_accents{0};
            ),
            D2_UAI_PROPS(Themes, MaxAccents),
            D2_UAI_LINK(D2_UAI_PROP(Themes, themes, Masked)
                            D2_UAI_PROP(MaxAccents, max_accents, None))
        )
    }
    namespace ex
    {
        namespace impl
        {
            class ThemeSelectorBase
            {
            protected:
                style::IZThemeSelector::accent_vec _accents{};
            public:
                virtual ~ThemeSelectorBase() = default;

                void swap(std::size_t a, std::size_t b);
                void remove(std::size_t idx);
                void push(px::background accent);
                void push(std::size_t idx, px::background accent);
                void clear();
                const style::IZThemeSelector::accent_vec& accents();

                virtual void submit() = 0;
                virtual void submit(const string& name) = 0;
            };

            style::IZThemeSelector::theme_list default_themes();
        } // namespace impl

        class ThemeSelectorWindow
            : public style::UAIC<
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
            virtual void
            _signal_write_impl(write_flag type, unsigned int prop, ptr element) override;

            void _update_theme_list();
        public:
            ThemeSelectorWindow(const std::string& name, TreeState::ptr state);

            virtual void submit(const std::string& name) override;
            virtual void submit() override;
        };
        class ThemeSelector
            : public style::UAIC<
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
            virtual void
            _signal_write_impl(write_flag type, unsigned int prop, ptr element) override;

            void _update_theme_list();
        public:
            ThemeSelector(const std::string& name, TreeState::ptr state);

            virtual void submit(const std::string& name) override;
            virtual void submit() override;
        };
    } // namespace ex
} // namespace d2

#endif // D2_THEME_SELECTOR_HPP
