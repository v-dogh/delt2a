#ifndef D2_FILE_EXPLORER_HPP
#define D2_FILE_EXPLORER_HPP

#include "../elements/d2_std.hpp"
#include "../d2_colors.hpp"
#include "../d2_dsl.hpp"
#include "d2_popup_theme_base.hpp"
#include <filesystem>
#include <regex>

namespace d2::templ
{
	class FilesystemExplorer :
		public dx::VirtualBox,
		public style::UAIC<dx::VirtualBox::data, FilesystemExplorer, style::IResponsive>
	{
	public:
		using data = style::UAIC<VirtualBox::data, FilesystemExplorer, style::IResponsive>;
		D2_UAI_CHAIN(FilesystemExplorer)
	protected:
		class FolderView :
			public MetaParentElement,
			public style::UAI<FolderView, style::ILayout, style::IColors, style::IKeyboardNav>,
			public dx::impl::UnitUpdateHelper<FolderView>,
			public dx::impl::TextHelper<FolderView>
		{
		public:
			friend class UnitUpdateHelper;
			friend class TextHelper;
			using data = style::UAI<FolderView, style::ILayout, style::IColors, style::IKeyboardNav>;
		protected:
			struct FileEntry
			{
				std::string path{ "" };
				std::string value{ "" };
			};

			std::shared_ptr<dx::VerticalSlider> scrollbar_{ nullptr };
			std::vector<FileEntry> entries_{};
			std::vector<FileEntry*> filtered_view_{};
			int focused_idx_{ -1 };
			int range_idx_{ 0 };
			bool use_filtered_{ false };

			virtual BoundingBox _dimensions_impl() const noexcept override
			{
				return {
					(data::width.getunits() == Unit::Auto ?
						int(entries_.size()) : _resolve_units(data::width)),
					(data::height.getunits() == Unit::Auto ?
						int(std::max_element(entries_.begin(), entries_.end(), [](const auto& v1, const auto& v2) {
							return v1.value.size() < v2.value.size();
						}) - entries_.begin()) :
						_resolve_units(data::height))
				};
			}
			virtual Position _position_impl() const noexcept override
			{
				return {
					_resolve_units(data::x),
					_resolve_units(data::y)
				};
			}

			virtual unit_meta_flag _unit_report_impl() const noexcept override
			{
				return UnitUpdateHelper::_report_units();
			}
			virtual char _index_impl() const noexcept override
			{
				return data::zindex;
			}
			virtual bool _provides_input_impl() const noexcept override
			{
				return true;
			}

			virtual void _state_change_impl(State state, bool value) override
			{
				if (state == State::Swapped && !value)
				{
					if (scrollbar_ == nullptr)
					{
						using namespace dx;

						scrollbar_ = std::make_shared<dx::VerticalSlider>(
							"", this->state(), shared_from_this()
						);
						scrollbar_->setstate(State::Swapped, false);

						scrollbar_->x = 0.0_pxi;
						scrollbar_->y = 0.0_center;
						scrollbar_->width = 1.0_px;
						scrollbar_->height = 1.0_pc;
						scrollbar_->slider_background_color = PixelForeground{ .v = '|' };
						scrollbar_->on_change = [this](auto, auto, auto abs) {
							range_idx_ = abs;
							_signal_write(WriteType::Masked);
						};

						auto& theme = this->state()->screen()->theme<PopupTheme>();
						(*scrollbar_)
							.set<VerticalSlider::SliderColor>(theme.pt_bg_button())
							.set<VerticalSlider::BackgroundColor>(theme.pt_bg_button());

						scrollbar_->_signal_write(Masked);
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
								scrollbar_->reset_absolute(focused_idx_ - h + 1);
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
								scrollbar_->reset_absolute(focused_idx_);
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

			virtual void _update_layout_impl() noexcept override
			{
				const auto [ width, height ] = box();
				const auto full_height = use_filtered_ ? filtered_view_.size() : entries_.size();
				scrollbar_->set<dx::VerticalSlider::SliderWidth>(
					full_height ? ((float(height) / full_height) * height) : 0
				);
				scrollbar_->set<dx::VerticalSlider::Max>(
					(use_filtered_ ? filtered_view_.size() : entries_.size()) -
					box().height
				);
			}
			virtual void _frame_impl(PixelBuffer::View buffer) noexcept override
			{
				const auto [ width, height ] = box();

				buffer.fill(data::background_color);

				const auto f = scrollbar_->frame();
				buffer.inscribe(width - 1, 0, f.buffer());

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
						px, ptr->value.size(), offset,
						width - ptr->value.size(), 1
					);
				}
			}
		public:
			using MetaParentElement::MetaParentElement;

			void filter(const std::string& reg) noexcept
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
			void setpath(std::filesystem::path path) noexcept
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
			int results() const noexcept
			{
				return use_filtered_ ? filtered_view_.size() : entries_.size();
			}

			virtual void foreach_internal(foreach_internal_callback callback) const override
			{
				callback(scrollbar_);
			}
			virtual void foreach(foreach_callback callback) const override
			{
				callback(scrollbar_);
			}
		};
	protected:
		std::vector<std::string> history_{};
		std::string softselect_{ "" };
		std::filesystem::path path_{ "/" };
		int rcnt_{ 0 };

		static std::string _memory_units(std::size_t bytes) noexcept
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
		static eptr<FilesystemExplorer> _core(TreeState::ptr state)
		{
			return
				state->core()->traverse().as<FilesystemExplorer>();
		}

		virtual void _state_change_impl(State state, bool value) override
		{
			VirtualBox::_state_change_impl(state, value);
			if (state == State::Swapped && !value && empty())
			{
				VirtualBox::width = 52.0_px;
				VirtualBox::height = 16.0_px;
				VirtualBox::zindex = 120;
				VirtualBox::container_options =
					ContainerOptions::EnableBorder | ContainerOptions::TopFix;
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

				auto& src = this->state()->screen()->theme<PopupTheme>();
				data::set<VirtualBox::BorderHorizontalColor>(src.pt_border_horizontal());
				data::set<VirtualBox::BorderVerticalColor>(src.pt_border_vertical());
				data::set<VirtualBox::FocusedColor>(src.pt_hbg_button());
				data::set<VirtualBox::BarColor>(style::dynavar<[](const auto& value) {
					return value.extend('.');
				}>(src.pt_border_horizontal()));

				using namespace dx;
				D2_STATELESS_TREE(filesystem)
					// Controls
					D2_ELEM(Button, exit)
						D2_STYLE(Value, "<X>")
						D2_STYLE(X, 1.0_pxi)
						D2_STYLE(ForegroundColor, D2_VAR(PopupTheme, pt_text()))
						D2_STYLE(FocusedColor, D2_VAR(PopupTheme, pt_hbg_button()))
						D2_STYLE(OnSubmit, [state](d2::Element::TraversalWrapper ptr) {
							_core(state)->close();
						})
					D2_ELEM_END(exit)
					D2_ELEM(Button, submit)
						D2_STYLE(Value, "<Ok>")
						D2_STYLE(X, 5.0_pxi)
						D2_STYLE(ForegroundColor, D2_VAR(PopupTheme, pt_text()))
						D2_STYLE(FocusedColor, D2_VAR(PopupTheme, pt_hbg_button()))
						D2_STYLE(OnSubmit, [state](d2::Element::TraversalWrapper ptr) {
							_core(state)->submit_soft();
						})
					D2_ELEM_END(submit)
					D2_ELEM_UNNAMED(Button)
						D2_STYLE(Value, "<-")
						D2_STYLE(X, 1.0_px)
						D2_STYLE(ForegroundColor, D2_VAR(PopupTheme, pt_text()))
						D2_STYLE(FocusedColor, D2_VAR(PopupTheme, pt_hbg_button()))
						D2_STYLE(OnSubmit, [state](d2::Element::TraversalWrapper ptr) {
							_core(state)->backwards();
						})
					D2_UELEM_END
					D2_ELEM_UNNAMED(Button)
						D2_STYLE(Value, "->")
						D2_STYLE(X, 4.0_px)
						D2_STYLE(ForegroundColor, D2_VAR(PopupTheme, pt_text()))
						D2_STYLE(FocusedColor, D2_VAR(PopupTheme, pt_hbg_button()))
						D2_STYLE(OnSubmit, [state](d2::Element::TraversalWrapper ptr) {
							_core(state)->forwards();
						})
					D2_UELEM_END
					D2_ELEM(Text, info)
						D2_STYLE(ForegroundColor, D2_VAR(PopupTheme, pt_text()))
						D2_STYLE(X, 0.0_center)
					D2_ELEM_END(info)
					// Separators
					D2_ELEM_UNNAMED(Text)
						D2_STYLE(Value, ">")
						D2_STYLE(ForegroundColor, D2_VAR(PopupTheme, pt_text()))
						D2_STYLE(Y, 1.0_px)
					D2_UELEM_END
					D2_ELEM_UNNAMED(Text)
						D2_STYLE(Value, "<")
						D2_STYLE(ForegroundColor, D2_VAR(PopupTheme, pt_text()))
						D2_STYLE(X, 0.0_pxi)
						D2_STYLE(Y, 1.0_px)
					D2_UELEM_END
					// Search Controls
					D2_ELEM(Input, search)
						D2_STYLE(Pre, "<search> ")
						D2_STYLE(ForegroundColor, D2_VAR(PopupTheme, pt_text()))
						D2_STYLE(PtrColor, D2_VAR(PopupTheme, pt_text_ptr()))
						D2_STYLE(X, 1.0_relative)
						D2_STYLE(Y, 2.0_px)
						D2_STYLE(Width, 27.0_pxi)
						D2_STYLE(OnSubmit, [state](d2::Element::TraversalWrapper ptr, const std::string& filename) {
							_core(state)->rselect(filename);
						})
					D2_ELEM_END(search)
					D2_ELEM_UNNAMED(Button)
						D2_STYLE(Value, "filter")
						D2_STYLE(X, 1.0_relative)
						D2_STYLE(Y, 2.0_px)
						D2_STYLE(OnSubmit, [state](d2::Element::TraversalWrapper ptr) {
							const auto path = (*(_core(state)->traverse()/"search")
								.as<Input>())
								.get<Input::Value>();
							auto folder = (_core(state)->traverse()/"folder")
								.as<FolderView>();
							folder->filter(path);
							_core(state)->_update_results();
						})
						D2_STYLES_APPLY(impl::button_react)
					D2_UELEM_END
					D2_ELEM_UNNAMED(Button)
						D2_STYLE(Value, "set path")
						D2_STYLE(X, 1.0_relative)
						D2_STYLE(Y, 2.0_px)
						D2_STYLE(OnSubmit, [state](d2::Element::TraversalWrapper ptr) {
							const auto path = (*(_core(state)->traverse()/"search")
								.as<Input>())
								.get<Input::Value>();
							if (std::filesystem::exists(path))
								_core(state)->setpath(path);
						})
						D2_STYLES_APPLY(impl::button_react)
					D2_UELEM_END
					// Display
					D2_ELEM(FolderView, folder)
						D2_STYLE(Width, 1.0_pc)
						D2_STYLE(Height, 5.0_pxi)
						D2_STYLE(Y, 4.0_px)
						D2_STYLE(ForegroundColor, D2_VAR(PopupTheme, pt_text()))
						D2_STYLE(BackgroundColor, d2::colors::w::transparent)
						D2_STYLE(FocusedColor, D2_VAR(PopupTheme, pt_hbg_button()))
					D2_ELEM_END(folder)
				D2_TREE_END(filesystem)

				filesystem::create_at(
					traverse(),
					TreeState::make<TreeState>(
						this->state()->screen(),
						this->state()->context(),
						parent()->traverse().asp(),
						traverse().asp()
					)
				);
				setpath("/");
			}
		}

		void _update_results()
		{
			auto folder = at("folder").as<FolderView>();
			at("info").as<dx::Text>()->set<dx::Text::Value>(
				(!folder->results()) ? "no results" : std::format("results - {}", folder->results())
			);
		}
	public:
		using VirtualBox::VirtualBox;

		void forwards()
		{
			if (!history_.empty())
			{
				rselect(history_.back());
				history_.pop_back();
				rcnt_--;
			}
		}
		void backwards()
		{
			if (path_.has_parent_path())
			{
				history_.push_back(path_.filename());
				setpath(path_.parent_path());
				rcnt_++;
			}
		}

		void refresh()
		{
			setpath(path_);
		}
		void rselect(const std::string& filename)
		{
			if (!std::filesystem::exists(path_/filename) ||
				!std::filesystem::is_directory(path_/filename))
			{
				path_/=filename;
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
		void sselect(const std::string& filename) noexcept
		{
			softselect_ = filename;
		}
		void setpath(std::filesystem::path path)
		{
			// We manually calculate offset (instead of relative)
			// for preformance reasons

			const auto [ width, height ] = box();
			if (auto pstr = path.string();
				pstr.size() >= width - 8)
			{
				auto trunc = std::move(pstr);
				do trunc = trunc.substr(trunc.find('/') + 1);
				while (trunc.size() >= width - 8 && !trunc.empty());
				set<VirtualBox::Title>(
					std::format("<.../{}>", trunc)
				);
			}
			else
			{
				set<VirtualBox::Title>(
					std::format("<{}>", pstr)
				);
			}

			softselect_ = "";
			path_ = path;
			at("folder").as<FolderView>()
				->setpath(path_);
			_update_results();
		}
		std::filesystem::path getpath() const noexcept
		{
			return path_;
		}

		void submit_soft()
		{
			if (data::on_submit != nullptr)
			{
				path_ = path_/softselect_;
				data::on_submit(shared_from_this());
			}
			close();
		}
		void submit()
		{
			if (data::on_submit != nullptr)
				data::on_submit(shared_from_this());
			close();
		}
	};
}

#endif // D2_FILE_EXPLORER_HPP
