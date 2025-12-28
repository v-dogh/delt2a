#ifndef D2_THEME_SELECTOR_HPP
#define D2_THEME_SELECTOR_HPP

#include "../elements/d2_draggable_box.hpp"
#include <variant>

namespace d2
{
    namespace style
    {
        struct ThemeSelector
        {
            using accent_vec = std::vector<px::background>;
            using theme = std::variant<accent_vec, string>;
            std::vector<string> themes{};
            int accents{ 0 };

            template<uai_property Property>
            auto at_style()
            {
                D2_UAI_SETUP_EMPTY
                D2_UAI_VAR_START
                D2_UAI_GET_VAR(0, themes, Masked)
                D2_UAI_GET_VAR_A(1, accents)
                D2_UAI_VAR_END;
            }
        };

        template<std::size_t PropBase>
        struct IThemeSelector : ThemeSelector, InterfaceHelper<IThemeSelector, PropBase, 2>
        {
            using data = ThemeSelector;
            enum Property : uai_property
            {
                Themes = PropBase,
                MaxAccents,
            };
        };
        using IZThemeSelector = IThemeSelector<0>;
    }
    namespace ctm
    {
        class ThemeSelector : public style::UAIC<
                dx::VirtualBox,
                ThemeSelector,
                style::IThemeSelector,
                style::IResponsive<ThemeSelector, style::ThemeSelector::theme>::type
            >
        {
        public:
            using data = style::UAIC<
                VirtualBox,
                ThemeSelector,
                style::IThemeSelector,
                style::IResponsive<ThemeSelector, style::ThemeSelector::theme>::type
            >;
            D2_UAI_CHAIN(VirtualBox)
        protected:
            data::accent_vec _accents{};

            static eptr<ThemeSelector> _core(TreeState::ptr state);

            virtual void _state_change_impl(State state, bool value) override;
            virtual void _signal_write_impl(write_flag type, unsigned int prop, ptr element) override;

            void _update_theme_list();
        public:
            ThemeSelector(
                const std::string& name,
                TreeState::ptr state,
                pptr parent,
                std::function<void(TypedTreeIter<ThemeSelector>, theme)>
            );

            void submit(const std::string& name);
            void submit();
        };
    }
}

#endif // D2_THEME_SELECTOR_HPP
