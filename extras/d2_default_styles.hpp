#ifndef D2_DEFAULT_STYLES_HPP
#define D2_DEFAULT_STYLES_HPP

#include <d2_colors.hpp>
#include <d2_tree_construct.hpp>
#include <elements/d2_slider.hpp>
#include <extras/d2_default_theme.hpp>

namespace d2::ex::df
{
    using text = Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                            { dstylev(ForegroundColor, DefaultTheme::Text); }>;

    using muted_text = Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                                  { dstylev(ForegroundColor, DefaultTheme::TextMuted); }>;

    using disabled_text = Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                                     { dstylev(ForegroundColor, DefaultTheme::TextDisabled); }>;

    using inverse_text = Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                                    { dstylev(ForegroundColor, DefaultTheme::TextInverse); }>;

    using accent_text = Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                                   { dstylev(ForegroundColor, DefaultTheme::TextAccent); }>;

    using contrast_text = Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                                     { dstylev(ForegroundColor, DefaultTheme::TextContrast); }>;

    using bold_text = Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                                 {
                                     dstyledv(
                                         ForegroundColor,
                                         DefaultTheme::Text,
                                         value.stylize(d2::px::foreground::Bold)
                                     );
                                 }>;

    using bold_muted_text = Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                                       {
                                           dstyledv(
                                               ForegroundColor,
                                               DefaultTheme::TextMuted,
                                               value.stylize(d2::px::foreground::Bold)
                                           );
                                       }>;

    using bold_accent_text = Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                                        {
                                            dstyledv(
                                                ForegroundColor,
                                                DefaultTheme::TextAccent,
                                                value.stylize(d2::px::foreground::Bold)
                                            );
                                        }>;

    using bold_contrast_text = Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                                          {
                                              dstyledv(
                                                  ForegroundColor,
                                                  DefaultTheme::TextContrast,
                                                  value.stylize(d2::px::foreground::Bold)
                                              );
                                          }>;

    using surface = Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                               {
                                   dstylev(BackgroundColor, DefaultTheme::Surface);
                                   dstylev(ForegroundColor, DefaultTheme::Text);
                               }>;

    using surface_alt = Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                                   {
                                       dstylev(BackgroundColor, DefaultTheme::SurfaceAlt);
                                       dstylev(ForegroundColor, DefaultTheme::Text);
                                   }>;

    using surface_raised = Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                                      {
                                          dstylev(BackgroundColor, DefaultTheme::SurfaceRaised);
                                          dstylev(ForegroundColor, DefaultTheme::Text);
                                      }>;

    using surface_sunken = Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                                      {
                                          dstylev(BackgroundColor, DefaultTheme::SurfaceSunken);
                                          dstylev(ForegroundColor, DefaultTheme::Text);
                                      }>;

    using surface_overlay = Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                                       {
                                           dstylev(BackgroundColor, DefaultTheme::SurfaceOverlay);
                                           dstylev(ForegroundColor, DefaultTheme::Text);
                                       }>;

    using surface_disabled = Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                                        {
                                            dstylev(BackgroundColor, DefaultTheme::SurfaceDisabled);
                                            dstylev(ForegroundColor, DefaultTheme::TextDisabled);
                                        }>;

    using surface_contrast = Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                                        {
                                            dstylev(BackgroundColor, DefaultTheme::SurfaceContrast);
                                            dstylev(ForegroundColor, DefaultTheme::TextContrast);
                                        }>;

    using transparent_surface = Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                                           { dstyle(BackgroundColor, colors::w::transparent); }>;

    using focus = Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                             { dstylev(FocusedColor, DefaultTheme::AccentMuted); }>;

    using strong_focus = Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                                    { dstylev(FocusedColor, DefaultTheme::Accent); }>;

    using soft_focus =
        Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                   { dstyledv(FocusedColor, DefaultTheme::AccentMuted, value.alpha(0.7f)); }>;

    using border = Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                              {
                                  dstyle(ContainerBorder, true);
                                  dstylev(BorderHorizontalColor, DefaultTheme::BorderHoriz);
                                  dstylev(BorderVerticalColor, DefaultTheme::BorderVert);
                              }>;

    using muted_border =
        Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                   {
                       dstyle(ContainerBorder, true);
                       dstylev(BorderHorizontalColor, DefaultTheme::BorderMutedHoriz);
                       dstylev(BorderVerticalColor, DefaultTheme::BorderMutedVert);
                   }>;

    using strong_border =
        Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                   {
                       dstyle(ContainerBorder, true);
                       dstylev(BorderHorizontalColor, DefaultTheme::BorderStrongHoriz);
                       dstylev(BorderVerticalColor, DefaultTheme::BorderStrongVert);
                   }>;

    using focus_border =
        Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                   {
                       dstyle(ContainerBorder, true);
                       dstylev(BorderHorizontalColor, DefaultTheme::BorderFocusHoriz);
                       dstylev(BorderVerticalColor, DefaultTheme::BorderFocusVert);
                   }>;

    using contrast_border =
        Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                   {
                       dstyle(ContainerBorder, true);
                       dstylev(BorderHorizontalColor, DefaultTheme::BorderContrastHoriz);
                       dstylev(BorderVerticalColor, DefaultTheme::BorderContrastVert);
                   }>;

    using rough_border = Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                                    {
                                        dstyle(ContainerBorder, true);
                                        dstylev(BorderHorizontalColor, DefaultTheme::BorderHoriz);
                                        dstylev(BorderVerticalColor, DefaultTheme::BorderVert);

                                        dstyle(CornerTopLeft, charset::box_tl_corner_rough);
                                        dstyle(CornerTopRight, charset::box_tr_corner_rough);
                                        dstyle(CornerBottomLeft, charset::box_bl_corner_rough);
                                        dstyle(CornerBottomRight, charset::box_br_corner_rough);
                                    }>;

    using muted_rough_border =
        Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                   {
                       dstyle(ContainerBorder, true);
                       dstylev(BorderHorizontalColor, DefaultTheme::BorderMutedHoriz);
                       dstylev(BorderVerticalColor, DefaultTheme::BorderMutedVert);

                       dstyle(CornerTopLeft, charset::box_tl_corner_rough);
                       dstyle(CornerTopRight, charset::box_tr_corner_rough);
                       dstyle(CornerBottomLeft, charset::box_bl_corner_rough);
                       dstyle(CornerBottomRight, charset::box_br_corner_rough);
                   }>;

    using strong_rough_border =
        Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                   {
                       dstyle(ContainerBorder, true);
                       dstylev(BorderHorizontalColor, DefaultTheme::BorderStrongHoriz);
                       dstylev(BorderVerticalColor, DefaultTheme::BorderStrongVert);

                       dstyle(CornerTopLeft, charset::box_tl_corner_rough);
                       dstyle(CornerTopRight, charset::box_tr_corner_rough);
                       dstyle(CornerBottomLeft, charset::box_bl_corner_rough);
                       dstyle(CornerBottomRight, charset::box_br_corner_rough);
                   }>;

    using focus_rough_border =
        Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                   {
                       dstyle(ContainerBorder, true);
                       dstylev(BorderHorizontalColor, DefaultTheme::BorderFocusHoriz);
                       dstylev(BorderVerticalColor, DefaultTheme::BorderFocusVert);

                       dstyle(CornerTopLeft, charset::box_tl_corner_rough);
                       dstyle(CornerTopRight, charset::box_tr_corner_rough);
                       dstyle(CornerBottomLeft, charset::box_bl_corner_rough);
                       dstyle(CornerBottomRight, charset::box_br_corner_rough);
                   }>;

    using contrast_rough_border =
        Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                   {
                       dstyle(ContainerBorder, true);
                       dstylev(BorderHorizontalColor, DefaultTheme::BorderContrastHoriz);
                       dstylev(BorderVerticalColor, DefaultTheme::BorderContrastVert);

                       dstyle(CornerTopLeft, charset::box_tl_corner_rough);
                       dstyle(CornerTopRight, charset::box_tr_corner_rough);
                       dstyle(CornerBottomLeft, charset::box_bl_corner_rough);
                       dstyle(CornerBottomRight, charset::box_br_corner_rough);
                   }>;

    using button = Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                              {
                                  dstylev(BackgroundColor, DefaultTheme::SurfaceRaised);
                                  dstylev(ForegroundColor, DefaultTheme::Text);
                                  dstylev(FocusedColor, DefaultTheme::AccentMuted);

                                  // interp_twoway_auto(Hovered, 500, Linear, BackgroundColor,
                                  // DefaultTheme::AccentHover); interp_twoway_auto(Clicked, 100,
                                  // Linear, BackgroundColor, DefaultTheme::AccentActive);
                                  // interp_twoway_auto(Clicked, 100, Linear, ForegroundColor,
                                  // DefaultTheme::AccentText);
                              }>;

    using accent_button = Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                                     {
                                         dstylev(BackgroundColor, DefaultTheme::Accent);
                                         dstylev(ForegroundColor, DefaultTheme::AccentText);
                                         dstylev(FocusedColor, DefaultTheme::AccentMuted);

                                         // interp_twoway_auto(Hovered, 500, Linear,
                                         // BackgroundColor, DefaultTheme::AccentHover);
                                         // interp_twoway_auto(Clicked, 100, Linear,
                                         // BackgroundColor, DefaultTheme::AccentActive);
                                     }>;

    using muted_button = Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                                    {
                                        dstylev(BackgroundColor, DefaultTheme::AccentMuted);
                                        dstylev(ForegroundColor, DefaultTheme::Text);
                                        dstylev(FocusedColor, DefaultTheme::Accent);
                                    }>;

    using contrast_button = Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                                       {
                                           dstylev(BackgroundColor, DefaultTheme::Surface);
                                           dstylev(ForegroundColor, DefaultTheme::Text);
                                           dstylev(FocusedColor, DefaultTheme::AccentMuted);

                                           // interp_twoway_auto(Hovered, 500, Linear,
                                           // BackgroundColor, DefaultTheme::SurfaceAlt);
                                           // interp_twoway_auto(Clicked, 100, Linear,
                                           // BackgroundColor, DefaultTheme::SurfaceSunken);
                                       }>;

    using flat_button = Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                                   {
                                       dstyle(BackgroundColor, colors::w::transparent);
                                       dstylev(ForegroundColor, DefaultTheme::Text);
                                       dstylev(FocusedColor, DefaultTheme::AccentMuted);

                                       // interp_twoway_auto(Hovered, 500, Linear, BackgroundColor,
                                       // DefaultTheme::SurfaceAlt); interp_twoway_auto(Clicked,
                                       // 100, Linear, BackgroundColor,
                                       // DefaultTheme::SurfaceSunken);
                                   }>;

    using contrast_flat_button =
        Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                   {
                       dstyle(BackgroundColor, colors::w::transparent);
                       dstylev(ForegroundColor, DefaultTheme::TextContrast);
                       dstylev(FocusedColor, DefaultTheme::AccentMuted);

                       // interp_twoway_auto(Hovered, 500, Linear,
                       // BackgroundColor, DefaultTheme::Surface);
                       // interp_twoway_auto(Clicked, 100, Linear,
                       // BackgroundColor, DefaultTheme::SurfaceAlt);
                   }>;

    using disabled_button = Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                                       {
                                           dstylev(BackgroundColor, DefaultTheme::SurfaceDisabled);
                                           dstylev(ForegroundColor, DefaultTheme::TextDisabled);
                                           dstylev(FocusedColor, DefaultTheme::AccentMuted);
                                       }>;

    using icon = Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                            {
                                dstyledv(
                                    ForegroundColor,
                                    DefaultTheme::Text,
                                    value.stylize(d2::px::foreground::Bold)
                                );

                                dstylev(FocusedColor, DefaultTheme::AccentMuted);

                                // interp_twoway_auto(
                                //     Hovered,
                                //     500,
                                //     Linear,
                                //     ForegroundColor,
                                //     DefaultTheme::AccentHover
                                // );
                                //
                                // interp_twoway_auto(
                                //     Clicked,
                                //     100,
                                //     Linear,
                                //     ForegroundColor,
                                //     DefaultTheme::AccentActive
                                // );
                            }>;

    using accent_icon = Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                                   {
                                       dstyledv(
                                           ForegroundColor,
                                           DefaultTheme::TextAccent,
                                           value.stylize(d2::px::foreground::Bold)
                                       );

                                       dstylev(FocusedColor, DefaultTheme::AccentMuted);

                                       // interp_twoway_auto(Hovered, 500, Linear, ForegroundColor,
                                       // DefaultTheme::AccentHover); interp_twoway_auto(Clicked,
                                       // 100, Linear, ForegroundColor, DefaultTheme::AccentActive);
                                   }>;

    using contrast_icon = Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                                     {
                                         dstyledv(
                                             ForegroundColor,
                                             DefaultTheme::TextContrast,
                                             value.stylize(d2::px::foreground::Bold)
                                         );

                                         dstylev(FocusedColor, DefaultTheme::AccentMuted);
                                     }>;

    using input = Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                             {
                                 dstylev(PtrColor, DefaultTheme::Cursor);
                                 dstylev(MarkedMask, DefaultTheme::TextAccent);
                                 dstylev(ForegroundColor, DefaultTheme::Text);
                                 dstyle(BackgroundColor, colors::w::transparent);
                             }>;

    using filled_input = Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                                    {
                                        dstylev(PtrColor, DefaultTheme::Cursor);
                                        dstylev(MarkedMask, DefaultTheme::TextAccent);
                                        dstylev(ForegroundColor, DefaultTheme::Text);
                                        dstylev(BackgroundColor, DefaultTheme::SurfaceSunken);
                                    }>;

    using contrast_input = Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                                      {
                                          dstylev(PtrColor, DefaultTheme::Cursor);
                                          dstylev(MarkedMask, DefaultTheme::Text);
                                          dstylev(ForegroundColor, DefaultTheme::TextContrast);
                                          dstylev(BackgroundColor, DefaultTheme::SurfaceContrast);
                                      }>;

    using disabled_input = Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                                      {
                                          dstylev(PtrColor, DefaultTheme::Cursor);
                                          dstylev(MarkedMask, DefaultTheme::Text);
                                          dstylev(ForegroundColor, DefaultTheme::TextDisabled);
                                          dstylev(BackgroundColor, DefaultTheme::SurfaceDisabled);
                                      }>;

    using switch_control =
        Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                   {
                       dstylev(EnabledForegroundColor, DefaultTheme::TextAccent);
                       dstyle(EnabledBackgroundColor, colors::w::transparent);

                       dstylev(DisabledForegroundColor, DefaultTheme::TextMuted);
                       dstyle(DisabledBackgroundColor, colors::w::transparent);

                       dstylev(SeparatorColor, DefaultTheme::TextMuted);
                       dstyledv(FocusedColor, DefaultTheme::AccentMuted, value.alpha(0.7f));
                   }>;

    using contrast_switch =
        Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                   {
                       dstylev(EnabledForegroundColor, DefaultTheme::TextContrast);
                       dstyle(EnabledBackgroundColor, colors::w::transparent);

                       dstylev(DisabledForegroundColor, DefaultTheme::TextDisabled);
                       dstyle(DisabledBackgroundColor, colors::w::transparent);

                       dstylev(SeparatorColor, DefaultTheme::TextContrast);
                       dstyledv(FocusedColor, DefaultTheme::AccentMuted, value.alpha(0.7f));
                   }>;

    using filled_switch =
        Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                   {
                       dstylev(EnabledForegroundColor, DefaultTheme::AccentText);
                       dstylev(EnabledBackgroundColor, DefaultTheme::AccentActive);

                       dstylev(DisabledForegroundColor, DefaultTheme::TextDisabled);
                       dstylev(DisabledBackgroundColor, DefaultTheme::SurfaceDisabled);

                       dstylev(SeparatorColor, DefaultTheme::TextMuted);
                       dstyledv(FocusedColor, DefaultTheme::AccentMuted, value.alpha(0.7f));
                   }>;

    using selector =
        Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                   {
                       dstylev(EnabledForegroundColor, DefaultTheme::SelectionText);
                       dstylev(EnabledBackgroundColor, DefaultTheme::Selection);

                       dstylev(DisabledForegroundColor, DefaultTheme::TextDisabled);
                       dstyle(DisabledBackgroundColor, colors::w::transparent);

                       dstyledv(
                           SliderBackgroundColor,
                           DefaultTheme::AccentMuted,
                           value.extend(charset::slider_vertical)
                       );

                       dstyledv(
                           SliderColor,
                           DefaultTheme::AccentHover,
                           value.extend(charset::slider_thumb_vertical)
                       );

                       dstyledv(FocusedColor, DefaultTheme::AccentMuted, value.alpha(0.7f));
                   }>;

    using arrow_selector =
        Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                   {
                       dstyle(SelectPre, std::format("{} ", charset::icon::arrow_right));
                       dstylev(PreColor, DefaultTheme::TextAccent);

                       dstylev(EnabledForegroundColor, DefaultTheme::SelectionText);
                       dstylev(EnabledBackgroundColor, DefaultTheme::Selection);

                       dstylev(DisabledForegroundColor, DefaultTheme::TextDisabled);
                       dstyle(DisabledBackgroundColor, colors::w::transparent);

                       dstyledv(
                           SliderBackgroundColor,
                           DefaultTheme::AccentMuted,
                           value.extend(charset::slider_vertical)
                       );

                       dstyledv(
                           SliderColor,
                           DefaultTheme::AccentHover,
                           value.extend(charset::slider_thumb_vertical)
                       );

                       dstyledv(FocusedColor, DefaultTheme::AccentMuted, value.alpha(0.7f));
                   }>;

    using contrast_arrow_selector =
        Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                   {
                       dstyle(SelectPre, std::format("{} ", charset::icon::arrow_right));
                       dstylev(PreColor, DefaultTheme::TextContrast);

                       dstylev(EnabledForegroundColor, DefaultTheme::Text);
                       dstylev(EnabledBackgroundColor, DefaultTheme::Surface);

                       dstylev(DisabledForegroundColor, DefaultTheme::TextDisabled);
                       dstyle(DisabledBackgroundColor, colors::w::transparent);

                       dstyledv(
                           SliderBackgroundColor,
                           DefaultTheme::TextMuted,
                           value.extend(charset::slider_vertical)
                       );

                       dstyledv(
                           SliderColor,
                           DefaultTheme::Text,
                           value.extend(charset::slider_thumb_vertical)
                       );

                       dstyledv(FocusedColor, DefaultTheme::AccentMuted, value.alpha(0.7f));
                   }>;

    using slider = Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                              {
                                  dstyledv(
                                      SliderColor,
                                      DefaultTheme::TextAccent,
                                      value.extend(d2::charset::slider_thumb_horizontal)
                                  );

                                  dstyledv(
                                      ForegroundColor,
                                      DefaultTheme::TextMuted,
                                      value.extend(d2::charset::slider_horizontal)
                                  );
                              }>;

    using vertical_slider = Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                                       {
                                           dstyledv(
                                               SliderColor,
                                               DefaultTheme::TextAccent,
                                               value.extend(d2::charset::slider_thumb_vertical)
                                           );

                                           dstyledv(
                                               ForegroundColor,
                                               DefaultTheme::TextMuted,
                                               value.extend(d2::charset::slider_vertical)
                                           );
                                       }>;

    using muted_slider = Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                                    {
                                        dstyledv(
                                            SliderColor,
                                            DefaultTheme::TextMuted,
                                            value.extend(d2::charset::slider_thumb_horizontal)
                                        );

                                        dstyledv(
                                            ForegroundColor,
                                            DefaultTheme::TextDisabled,
                                            value.extend(d2::charset::slider_horizontal)
                                        );
                                    }>;

    using muted_vertical_slider =
        Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                   {
                       dstyledv(
                           SliderColor,
                           DefaultTheme::TextMuted,
                           value.extend(d2::charset::slider_thumb_vertical)
                       );

                       dstyledv(
                           ForegroundColor,
                           DefaultTheme::TextDisabled,
                           value.extend(d2::charset::slider_vertical)
                       );
                   }>;

    using contrast_slider = Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                                       {
                                           dstyledv(
                                               SliderColor,
                                               DefaultTheme::TextContrast,
                                               value.extend(d2::charset::slider_thumb_horizontal)
                                           );

                                           dstyledv(
                                               ForegroundColor,
                                               DefaultTheme::TextContrast,
                                               value.extend(d2::charset::slider_horizontal)
                                           );
                                       }>;

    using contrast_vertical_slider =
        Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                   {
                       dstyledv(
                           SliderColor,
                           DefaultTheme::TextContrast,
                           value.extend(d2::charset::slider_thumb_vertical)
                       );

                       dstyledv(
                           ForegroundColor,
                           DefaultTheme::TextContrast,
                           value.extend(d2::charset::slider_vertical)
                       );
                   }>;

    using checkbox = Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                                {
                                    dstylev(ColorOnForeground, DefaultTheme::Text);
                                    dstylev(ColorOffForeground, DefaultTheme::TextMuted);
                                    dstylev(FocusedColor, DefaultTheme::AccentMuted);
                                }>;

    using accent_checkbox = Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                                       {
                                           dstylev(ColorOnForeground, DefaultTheme::TextAccent);
                                           dstylev(ColorOffForeground, DefaultTheme::TextMuted);
                                           dstylev(FocusedColor, DefaultTheme::AccentMuted);
                                       }>;

    using contrast_checkbox =
        Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                   {
                       dstylev(ColorOnForeground, DefaultTheme::TextContrast);
                       dstylev(ColorOffForeground, DefaultTheme::TextDisabled);
                       dstylev(FocusedColor, DefaultTheme::AccentMuted);
                   }>;

    using selection = Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                                 {
                                     dstylev(BackgroundColor, DefaultTheme::Selection);
                                     dstylev(ForegroundColor, DefaultTheme::SelectionText);
                                 }>;

    using inactive_selection =
        Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                   {
                       dstylev(BackgroundColor, DefaultTheme::SelectionInactive);
                       dstylev(ForegroundColor, DefaultTheme::Text);
                   }>;

    using highlight = Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                                 {
                                     dstylev(BackgroundColor, DefaultTheme::Highlight);
                                     dstylev(ForegroundColor, DefaultTheme::Text);
                                 }>;

    using inactive_highlight =
        Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                   {
                       dstylev(BackgroundColor, DefaultTheme::HighlightInactive);
                       dstylev(ForegroundColor, DefaultTheme::TextMuted);
                   }>;

    using info = Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                            {
                                dstylev(BackgroundColor, DefaultTheme::Info);
                                dstylev(ForegroundColor, DefaultTheme::InfoText);
                            }>;

    using success = Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                               {
                                   dstylev(BackgroundColor, DefaultTheme::Success);
                                   dstylev(ForegroundColor, DefaultTheme::SuccessText);
                               }>;

    using warning = Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                               {
                                   dstylev(BackgroundColor, DefaultTheme::Warning);
                                   dstylev(ForegroundColor, DefaultTheme::WarningText);
                               }>;

    using error = Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                             {
                                 dstylev(BackgroundColor, DefaultTheme::Error);
                                 dstylev(ForegroundColor, DefaultTheme::ErrorText);
                             }>;

    using info_text = Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                                 { dstylev(ForegroundColor, DefaultTheme::Info); }>;

    using success_text = Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                                    { dstylev(ForegroundColor, DefaultTheme::Success); }>;

    using warning_text = Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                                    { dstylev(ForegroundColor, DefaultTheme::Warning); }>;

    using error_text = Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                                  { dstylev(ForegroundColor, DefaultTheme::Error); }>;

    using scrollbox =
        Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                   {
                       {
                           ctx.clone(
                               ctx->scrollbar(),
                               [](TreeCtx<dx::VerticalSlider> ctx) { ctx.apply<vertical_slider>(); }
                           );
                       }
                   }>;

    using muted_scrollbox = Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                                       {
                                           {
                                               ctx.clone(
                                                   ctx->scrollbar(),
                                                   [](TreeCtx<dx::VerticalSlider> ctx)
                                                   { ctx.apply<muted_vertical_slider>(); }
                                               );
                                           }
                                       }>;

    using contrast_scrollbox = Stylesheet<[]<typename Type>(TreeCtx<Type> ctx)
                                          {
                                              {
                                                  ctx.clone(
                                                      ctx->scrollbar(),
                                                      [](TreeCtx<dx::VerticalSlider> ctx)
                                                      { ctx.apply<contrast_vertical_slider>(); }
                                                  );
                                              }
                                          }>;
} // namespace d2::ex::df

#endif
