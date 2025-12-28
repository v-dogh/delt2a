#include "d2_file_explorer.hpp"
#include "d2_widget_theme_base.hpp"
#include "../elements/d2_std.hpp"
#include "../d2_colors.hpp"
#include "../d2_dsl.hpp"
#include "../d2_tree.hpp"
#include <regex>

namespace d2::ctm
{
    string _memory_units(std::size_t bytes)
	{
		constexpr auto gib = 1'073'741'824;
		constexpr auto mib = 1'048'576;
		constexpr auto kib = 1'024;
		if (bytes > gib)
            return std::format("{}GiB", bytes / gib);
		else if (bytes > mib)
            return std::format("{}MiB", bytes / mib);
		else if (bytes > kib)
            return std::format("{}KiB", bytes / kib);
		else
            return std::format("{}b", bytes);
	}
    FilesystemExplorerWindow::eptr<impl::FilesystemExplorerBase> _core(TreeState::ptr state)
	{
        return state->core()->traverse().as<impl::FilesystemExplorerBase>();
	}
    FilesystemExplorerWindow::eptr<ParentElement> _pcore(TreeState::ptr state)
    {
        return state->core()->traverse().asp();
    }

	class FolderView :
        public style::UAIC<
            dx::Box,
            FolderView,
            style::IKeyboardNav
		>,
		public dx::impl::TextHelper<FolderView>
	{
	public:
		friend class TextHelper;
        using data = style::UAIC<
            dx::Box,
			FolderView,
			style::IKeyboardNav
		>;
        D2_UAI_CHAIN(d2::dx::Box)
	protected:
		struct FileEntry
		{
            string path{};
            string value{};
		};

		std::shared_ptr<dx::VerticalSlider> scrollbar_{ nullptr };
		std::vector<FileEntry> entries_{};
		std::vector<FileEntry*> filtered_view_{};
		int focused_idx_{ -1 };
		int range_idx_{ 0 };
		bool use_filtered_{ false };

        virtual Unit _layout_impl(Element::Layout type) const override
        {
            switch (type)
            {
            case Element::Layout::X: return data::x;
            case Element::Layout::Y: return data::y;
            case Element::Layout::Width:
                if (data::width.getunits() == Unit::Auto)
                    return int(entries_.size());
                return data::width;
            case Element::Layout::Height:
                if (data::width.getunits() == Unit::Auto)
                {
                    return int(std::max_element(entries_.begin(), entries_.end(), [](const auto& v1, const auto& v2) {
                        return v1.value.size() < v2.value.size();
                    }) - entries_.begin());
                }
                return data::height;
            }
        }

		virtual bool _provides_input_impl() const override
		{
			return true;
		}

		virtual void _state_change_impl(State state, bool value) override
		{
            Box::_state_change_impl(state, value);
			if (state == State::Created && value)
			{
				if (scrollbar_ == nullptr)
				{
					using namespace dx;

                    scrollbar_ = std::static_pointer_cast<VerticalSlider>(
                        create(Element::make<dx::VerticalSlider>(
                            "",
                            this->state(),
                            std::static_pointer_cast<ParentElement>(shared_from_this())
                        )).shared()
                    );

                    D2_CONTEXT(scrollbar_, VerticalSlider)
                        D2_STYLE(X, 0.0_pxi)
                        D2_STYLE(Y, 0.0_center)
                        D2_STYLE(Width, 1.0_px)
                        D2_STYLE(Height, 1.0_pc)
                        D2_STYLE(OnSubmit, [this](auto, auto, auto abs) {
                            range_idx_ = abs;
                            _signal_write(WriteType::Masked);
                        })
                        D2_STYLE(SliderColor, D2_DYNAVAR(WidgetTheme, wg_fg_button(),
                            value.extend(d2::charset::slider_thumb_vertical)
                        ))
                        D2_STYLE(ForegroundColor, D2_DYNAVAR(WidgetTheme, wg_fg_button(),
                            value.extend(d2::charset::slider_vertical)
                        ))
                    D2_CONTEXT_END

                    internal::ElementView::from(scrollbar_)
                        .signal_write(Masked);
				}
			}
			else if (state == State::Clicked && value)
			{
				const auto src = scrollbar_->absolute_value();
				const auto [ width, height ] = box();
				const auto [ x, idx ] = mouse_object_space();
				if (x < width - 1 &&
					scrollbar_->absolute_value() + idx <
					(use_filtered_ ? filtered_view_.size() : entries_.size()))
				{
					if (idx + src != focused_idx_)
					{
						focused_idx_ = idx + src;
						_core(this->state())->sselect(
							_focused_entry()->path
						);
						_signal_write(Style);
					}
					else
					{
						_core(this->state())->rselect(
							_focused_entry()->path
						);
					}
				}
			}
			else if (state == State::Focused && !value)
			{
				if (focused_idx_ != -1)
				{
					focused_idx_ = -1;
					_signal_write(Style);
				}
			}
			else if (state == State::Keynavi)
			{
				if (value)
				{
					focused_idx_ = 0;
					_signal_write(Style);
				}
				else
				{
					focused_idx_ = -1;
					_signal_write(Style);
				}
			}
		}
		virtual void _event_impl(IOContext::Event ev) override
		{
            Box::_event_impl(ev);
			if (ev == IOContext::Event::KeyInput)
			{
				if (context()->input()->is_pressed(sys::SystemInput::ArrowDown) ||
					context()->input()->is_pressed(sys::SystemInput::ArrowRight))
				{
					if (focused_idx_ == -1)
						focused_idx_ = 0;
					if (focused_idx_ <
						(use_filtered_ ? filtered_view_.size() : entries_.size()) - 1)
					{
						focused_idx_++;
						const auto h = box().height;
						if (focused_idx_ > (scrollbar_->absolute_value() + h - 1))
                            scrollbar_->reset_relative(focused_idx_ - h + 1);
						_signal_write(Style);
					}
				}
				else if (context()->input()->is_pressed(sys::SystemInput::ArrowUp) ||
						 context()->input()->is_pressed(sys::SystemInput::ArrowLeft))
				{
					if (focused_idx_ == -1)
						focused_idx_ = 0;
					if (focused_idx_ > 0)
					{
						focused_idx_--;
						if (focused_idx_ < scrollbar_->absolute_value())
                            scrollbar_->reset_relative(focused_idx_);
						_signal_write(Style);
					}
				}
				else if (context()->input()->is_pressed(sys::SystemInput::Enter))
				{
					if (focused_idx_ != -1)
					{
						_core(this->state())->rselect(
							_focused_entry()->path
						);
						focused_idx_ = 0;
						_signal_write(Style);
					}
				}
			}
		}

		virtual void _update_layout_impl() override
		{
            Box::_update_layout_impl();
			const auto [ width, height ] = box();
			const auto full_height = use_filtered_ ? filtered_view_.size() : entries_.size();
            scrollbar_->set<dx::Slider::SliderWidth>(
                std::max<std::size_t>(1, (full_height && full_height > height) ?
                (float(height) / full_height) * height : height)
            );
            scrollbar_->set<dx::Slider::Max>(full_height > height ? full_height - height : 0);
            internal::ElementView::from(scrollbar_)
                .signal_write(WriteType::Style);
		}
		virtual void _frame_impl(PixelBuffer::View buffer) override
		{
            Box::_frame_impl(buffer);

            const auto [ width, height ] = box();
            if (use_filtered_)
            {
                const auto m = std::min(height + range_idx_, int(filtered_view_.size()));
                for (std::size_t i = range_idx_; i < m; i++)
                    _render_entry(filtered_view_[i], i - range_idx_, i == focused_idx_, buffer);
            }
            else
            {
                const auto m = std::min(height + range_idx_, int(entries_.size()));
                for (std::size_t i = range_idx_; i < m; i++)
                    _render_entry(&entries_[i], i - range_idx_, i == focused_idx_, buffer);
            }
		}

		FileEntry* _focused_entry()
		{
			return use_filtered_ ?
				filtered_view_[focused_idx_] : &entries_[focused_idx_];
		}
		void _render_entry(FileEntry* ptr, int offset, bool focused, PixelBuffer::View buffer)
		{
			const auto [ width, height ] = box();
			const auto px = Pixel::combine(
				data::foreground_color,
				data::focused_color.alpha(focused ? 0.8f : 0.f)
			);
			TextHelper::_render_text_simple(
				ptr->value,
				px,
				{ 0, offset }, { width - 1, 1 },
				buffer
			);
			if (ptr->value.size() < width)
			{
                buffer.fill(
                    px,
                    ptr->value.size(), offset,
                    width - ptr->value.size() - 1, 1
				);
			}
		}
	public:
        void filter(const string& reg)
		{
			scrollbar_->reset_absolute();
			range_idx_ = 0;
			focused_idx_ = -1;
			filtered_view_.clear();
			if (reg.empty())
			{
				use_filtered_ = false;
				filtered_view_.shrink_to_fit();
			}
			else
			{
				filtered_view_.reserve(entries_.size() * 0.5);
				for (decltype(auto) it : entries_)
				{
					if (std::smatch match;
						std::regex_search(it.path, match, std::regex(reg)))
					{
						filtered_view_.push_back(&it);
					}
				}
				use_filtered_ = true;
			}
			_signal_write();
		}
		void setpath(std::filesystem::path path)
		{
			scrollbar_->reset_absolute();
			focused_idx_ = -1;
			range_idx_ = 0;
			use_filtered_ = false;
			filtered_view_.clear();
			entries_.clear();

			try
			{
				entries_.reserve(
					std::distance(
						std::filesystem::directory_iterator(path),
						std::filesystem::directory_iterator{}
					)
				);

				for (decltype(auto) it : std::filesystem::directory_iterator(path))
				{
					try
					{
						bool has_wtime = false;
						bool has_fsize = false;
						std::filesystem::file_time_type wtime{};
						std::size_t fsize = 0;
						try
						{
							wtime = it.last_write_time();
							has_wtime = true;
							if (it.is_regular_file())
							{
								fsize = it.file_size();
								has_fsize = true;
							}
						}
						catch (...) {}

						const auto filename = it.path().filename().string();
						const bool is_dir = it.is_directory();
						entries_.push_back({
							.path = filename,
							.value = std::format(
							   "{:<12} : <{}>, <{}>",
							   filename,
							   (is_dir ? "dir" :
								   (has_fsize ? std::format("file:{}", _memory_units(fsize)) : "file")
							   ),
							   has_wtime ? std::format("{:%Y.%m.%d %X}", wtime) : "<unknown>"
						   )
						});
					}
					catch (...) {}
				}
			}
			catch (...) {}
			_signal_write();
		}
		int results() const
		{
			return use_filtered_ ? filtered_view_.size() : entries_.size();
		}
	};

    D2_TREE_DEFINE(impl::FilesystemExplorerBase::interface, bool window)
        // Controls
        if (window)
            D2_ELEM(Button, exit)
                D2_STYLE(Value, "<X>")
                D2_STYLE(X, 1.0_pxi)
                D2_STYLE(ForegroundColor, D2_VAR(WidgetTheme, wg_text()))
                D2_STYLE(FocusedColor, D2_VAR(WidgetTheme, wg_hbg_button()))
                D2_STYLE(OnSubmit, [state](TreeIter ptr) {
                    std::static_pointer_cast<FilesystemExplorerWindow>(_core(state))
                        ->close();
                })
            D2_ELEM_END
        D2_ELEM(Button, submit)
            D2_STYLE(Value, "<Ok>")
            D2_STYLE(X, window ? 5.0_pxi : 1.0_pxi)
            D2_STYLE(ForegroundColor, D2_VAR(WidgetTheme, wg_text()))
            D2_STYLE(FocusedColor, D2_VAR(WidgetTheme, wg_hbg_button()))
            D2_STYLE(OnSubmit, [state](TreeIter ptr) {
                _core(state)->submit_soft();
            })
        D2_ELEM_END
        D2_ELEM(Button)
            D2_STYLE(Value, "<-")
            D2_STYLE(X, 1.0_px)
            D2_STYLE(ForegroundColor, D2_VAR(WidgetTheme, wg_text()))
            D2_STYLE(FocusedColor, D2_VAR(WidgetTheme, wg_hbg_button()))
            D2_STYLE(OnSubmit, [state](TreeIter ptr) {
                _core(state)->backwards();
            })
        D2_ELEM_END
        D2_ELEM(Button)
            D2_STYLE(Value, "->")
            D2_STYLE(X, 4.0_px)
            D2_STYLE(ForegroundColor, D2_VAR(WidgetTheme, wg_text()))
            D2_STYLE(FocusedColor, D2_VAR(WidgetTheme, wg_hbg_button()))
            D2_STYLE(OnSubmit, [state](TreeIter ptr) {
                _core(state)->forwards();
            })
        D2_ELEM_END
        D2_ELEM(Text, info)
            D2_STYLE(ForegroundColor, D2_VAR(WidgetTheme, wg_text()))
            D2_STYLE(X, 0.0_center)
        D2_ELEM_END
        D2_ELEM(Separator)
            D2_STYLE(Width, 1.0_pc)
            D2_STYLE(Height, 1.0_px)
            D2_STYLE(Y, 1.0_px)
            D2_STYLE(ForegroundColor, D2_VAR(WidgetTheme, wg_text()))
            D2_STYLE(CornerLeft, '>')
            D2_STYLE(CornerRight, '<')
            D2_STYLE(OverrideCorners, true)
            D2_STYLE(HideBody, true)
        D2_ELEM_END
        // Search Controls
        D2_ELEM(Input, search)
            D2_STYLE(Pre, "<search> ")
            D2_STYLE(ForegroundColor, D2_VAR(WidgetTheme, wg_text()))
            D2_STYLE(PtrColor, D2_VAR(WidgetTheme, wg_text_ptr()))
            D2_STYLE(X, 1.0_relative)
            D2_STYLE(Y, 2.0_px)
            D2_STYLE(Width, 9.0_pxi)
            D2_STYLE(OnSubmit, [state](TreeIter ptr, const string& filename) {
                _core(state)->rselect(filename);
            })
        D2_ELEM_END
        D2_ELEM(Button)
            D2_STYLE(Value, "filter")
            D2_STYLE(X, 1.0_relative)
            D2_STYLE(Y, 2.0_px)
            D2_STYLE(OnSubmit, [state](TreeIter ptr) {
                const auto path = (*(_pcore(state)->traverse()/"search")
                    .as<Input>())
                    .get<Input::Value>();
                auto folder = (_pcore(state)->traverse()/"folder")
                    .as<FolderView>();
                folder->filter(path);
                _core(state)->_update_results();
            })
            D2_STYLES_APPLY(button_react)
        D2_ELEM_END
        D2_ELEM(Separator)
            D2_STYLE(Width, 1.0_pc)
            D2_STYLE(Height, 1.0_px)
            D2_STYLE(Y, 3.0_px)
            D2_STYLE(ForegroundColor, D2_VAR(WidgetTheme, wg_text()))
            D2_STYLE(CornerLeft, '>')
            D2_STYLE(CornerRight, '<')
            D2_STYLE(OverrideCorners, true)
            D2_STYLE(HideBody, true)
        D2_ELEM_END
        // Display
        D2_ELEM(FolderView, folder)
            D2_STYLE(Width, 1.0_pxi)
            D2_STYLE(Height, 5.0_pxi)
            D2_STYLE(X, 1.0_px)
            D2_STYLE(Y, 4.0_px)
            D2_STYLE(ForegroundColor, D2_VAR(WidgetTheme, wg_text()))
            D2_STYLE(BackgroundColor, d2::colors::w::transparent)
            D2_STYLE(FocusedColor, D2_VAR(WidgetTheme, wg_hbg_button()))
        D2_ELEM_END
    D2_TREE_END

    namespace impl
    {
        void FilesystemExplorerBase::forwards()
        {
            if (!history_.empty())
            {
                rselect(history_.back());
                history_.pop_back();
                rcnt_--;
            }
        }
        void FilesystemExplorerBase::backwards()
        {
            if (path_.has_parent_path())
            {
                history_.push_back(path_.filename());
                setpath(path_.parent_path());
                rcnt_++;
            }
        }
        void FilesystemExplorerBase::refresh()
        {
            setpath(path_);
        }
        void FilesystemExplorerBase::rselect(const string& filename)
        {
            if (!std::filesystem::exists(path_/filename) ||
                !std::filesystem::is_directory(path_/filename))
            {
                path_ /= filename;
                submit();
            }
            else
            {
                if (!history_.empty() && filename != history_.back())
                {
                    history_.erase(
                        history_.begin() + (history_.size() - rcnt_),
                        history_.end()
                        );
                    rcnt_ = 0;
                }
                setpath(path_/filename);
            }
        }
        void FilesystemExplorerBase::sselect(const string& filename)
        {
            softselect_ = filename;
        }
        void FilesystemExplorerBase::setpath(std::filesystem::path path)
        {
            softselect_ = {};
            path_ = path;
            const auto max = path == "/" ? ~0ull : _path_max_width();
            if (auto pstr = path.string();
                pstr.size() >= max - 8)
            {
                auto trunc = std::move(pstr);
                do trunc = trunc.substr(trunc.find('/') + 1);
                while (trunc.size() >= max - 8 && !trunc.empty());
                _update_path_indicator(std::format(".../{}", trunc));
            }
            else
            {
                _update_path_indicator(pstr);
            }
        }
        std::filesystem::path FilesystemExplorerBase::path() const
        {
            return path_;
        }

        void FilesystemExplorerBase::submit_soft()
        {
            if (_is_submit_callback())
                path_ = path_/softselect_;
            _submit_path();
        }
        void FilesystemExplorerBase::submit()
        {
            _submit_path();
        }
    }

    void FilesystemExplorerWindow::_state_change_impl(State state, bool value)
	{
		VirtualBox::_state_change_impl(state, value);
		if (state == State::Created && value && empty())
		{
			VirtualBox::width = 52.0_px;
			VirtualBox::height = 16.0_px;
			VirtualBox::zindex = 120;
            VirtualBox::container_options |= ContainerOptions::EnableBorder;
            VirtualBox::title = "<->";

			if (screen()->is_keynav())
			{
				const auto [ swidth, sheight ] = context()->input()->screen_size();
				const auto [ width, height ] = box();
				VirtualBox::x = ((swidth - width) / 2);
				VirtualBox::y = ((sheight - height) / 2);
			}
			else
			{
				const auto [ x, y ] = context()->input()->mouse_position();
				VirtualBox::x = x - (box().width / 2);
				VirtualBox::y = y;
			}

			auto& src = this->state()->screen()->theme<WidgetTheme>();
            data::set<VirtualBox::BorderHorizontalColor>(src.wg_border_horizontal());
            data::set<VirtualBox::BorderVerticalColor>(src.wg_border_vertical());
            data::set<VirtualBox::FocusedColor>(src.wg_hbg_button());
            data::set<VirtualBox::BarColor>(style::dynavar<[](const px::foreground& value) {
                return value.extend('.');
            }>(src.wg_border_horizontal()));

            interface::create_at(
				traverse(),
				TreeState::make<TreeState>(
					this->state()->screen(),
					this->state()->context(),
					parent()->traverse().asp(),
                    traverse().asp()
                ),
                true
			);
			setpath("/");
		}
    }
    void FilesystemExplorerWindow::_update_results()
    {
        auto folder = at("folder").as<FolderView>();
        at("info").as<dx::Text>()->set<dx::Text::Value>(
            (!folder->results()) ? "no results" : std::format("results - {}", folder->results())
        );
    }
    bool FilesystemExplorerWindow::_is_submit_callback()
    {
        return data::on_submit != nullptr;
    }
    void FilesystemExplorerWindow::_submit_path()
    {
        if (data::on_submit)
            data::on_submit(
                std::static_pointer_cast<FilesystemExplorerWindow>(shared_from_this()),
                path()
            );
        close();
    }
    std::size_t FilesystemExplorerWindow::_path_max_width()
    {
        return box().width;
    }
    void FilesystemExplorerWindow::_update_path_indicator(const std::string& pstr)
    {
        set<VirtualBox::Title>(
            std::format("<{}>", pstr)
        );
        at("folder").as<FolderView>()
            ->setpath(path());
        _update_results();
    }

    void FilesystemExplorer::_state_change_impl(State state, bool value)
    {
        FlowBox::_state_change_impl(state, value);
        if (state == State::Created && value && empty())
        {
            auto& src = this->state()->screen()->theme<WidgetTheme>();
            data::set<FlowBox::BorderHorizontalColor>(src.wg_border_horizontal());
            data::set<FlowBox::BorderVerticalColor>(src.wg_border_vertical());
            interface::create_at(
                traverse(),
                TreeState::make<TreeState>(
                    this->state()->screen(),
                    this->state()->context(),
                    parent()->traverse().asp(),
                    traverse().asp()
                ),
                false
            );
            setpath("/");
        }
    }
    void FilesystemExplorer::_update_results()
    {
        auto folder = at("folder").as<FolderView>();
        at("info").as<dx::Text>()->set<dx::Text::Value>(
            (!folder->results()) ? "no results" : std::format("results - {}", folder->results())
        );
    }
    bool FilesystemExplorer::_is_submit_callback()
    {
        return data::on_submit != nullptr;
    }
    void FilesystemExplorer::_submit_path()
    {
        if (data::on_submit)
            data::on_submit(
                std::static_pointer_cast<FilesystemExplorer>(shared_from_this()),
                path()
            );
    }
    std::size_t FilesystemExplorer::_path_max_width()
    {
        return box().width;
    }
    void FilesystemExplorer::_update_path_indicator(const std::string& pstr)
    {
        at("search").as<dx::Input>()
            ->set<dx::Input::Hint>(pstr);
        at("folder").as<FolderView>()
            ->setpath(path());
        _update_results();
    }
}
