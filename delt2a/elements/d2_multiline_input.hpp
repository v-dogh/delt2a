#ifndef D2_MULTILINE_INPUT_H
#define D2_MULTILINE_INPUT_H

#include "../d2_tree_element.hpp"
#include "../d2_screen.hpp"
#include <variant>

namespace d2::dx
{
	class FragmentedBuffer
	{
	public:
		enum class EraseMode
		{
			Insert,
			Overwrite,
			Delete,
		};
	private:
		struct Fragment
		{
			std::variant<string_view, string> data{ string_view("") };
			std::unique_ptr<Fragment> next{ nullptr };
			Fragment* prev{ nullptr };

			string_view str() const noexcept
			{
				return std::visit([](auto&& str) {
					return string_view(str.begin(), str.end());
				}, data);
			}
			string_view str() noexcept
			{
				return const_cast<const Fragment*>(this)->str();
			}
			string str(std::size_t off, std::size_t cnt = std::string::npos) const noexcept
			{
				const auto s = str();
				if (cnt == std::string::npos)
					return string(s.begin() + off, s.end());
				return string(s.begin() + off, s.begin() + off + cnt);
			}
			string str(std::size_t off, std::size_t cnt = std::string::npos) noexcept
			{
				return const_cast<const Fragment*>(this)->str(off, cnt);
			}

			void map(const string& value)
			{
				data = value;
			}
			void map(string&& value) noexcept
			{
				data = std::move(value);
			}
			void map(string_view value) noexcept
			{
				data = value;
			}
			void map(Pixel::value_type value) noexcept
			{
				data = string(1, value);
			}

			void append(Pixel::value_type ch)
			{
				if (!dynamic())
					throw std::logic_error{ "Attempt to append to a static fragment" };
				std::get<string>(data).push_back(ch);
			}
			void append(string_view str)
			{
				if (!dynamic())
					throw std::logic_error{ "Attempt to append to a static fragment" };
				auto& d = std::get<string>(data);
				d.insert(d.end(), str.begin(), str.end());
			}
			void append(Pixel::value_type ch, std::size_t pos)
			{
				if (!dynamic())
					throw std::logic_error{ "Attempt to append to a static fragment" };
				auto& d = std::get<string>(data);
				d.insert(d.begin() + pos, ch);
			}
			void append(string_view str, std::size_t pos)
			{
				if (!dynamic())
					throw std::logic_error{ "Attempt to append to a static fragment" };
				auto& d = std::get<string>(data);
				d.insert(d.begin() + pos, str.begin(), str.end());
			}

			bool dynamic() noexcept
			{
				return std::holds_alternative<string>(data);
			}
			void write(Pixel::value_type value, std::size_t off)
			{
				if (!dynamic())
					throw std::logic_error{ "Attempt to write to a static fragment" };
				std::get<string>(data)[off] = value;
			}
			void erase(std::size_t off, std::size_t cnt = 1)
			{
				if (!dynamic())
					throw std::logic_error{ "Attempt to erase a static fragment" };
				std::get<string>(data).erase(off, cnt);
			}

			std::size_t size() const noexcept
			{
				return str().size();
			}
			std::size_t size() noexcept
			{
				return const_cast<const Fragment*>(this)->size();
			}
		};
	private:
		// Data

		Fragment start_{};
		string source_{ "" };
		std::size_t max_fragment_size_{ 256 };
		mutable std::size_t cached_size_{ 0 };

		// Pointers

		// Fragment that holds the beginning of the line
		Fragment* line_frag_{ nullptr };
		// Fragment that ptr_/local_ptr_ points to
		Fragment* current_frag_{ nullptr };

		std::size_t line_width_{ 0 };
		std::size_t line_height_{ 0 };

		// Relative to the line beginning

		std::size_t lower_bound_{ 0 };
		std::size_t upper_bound_{ 0 };

		// Absolute offset
		std::size_t ptr_{ 0 };
		// Relative to the current fragment
		std::size_t ptr_local_{ 0 };
		// Relative to the beginning of line
		std::size_t ptr_relative_{ 0 };
		// Absolute line count since beginning
		std::size_t line_ptr_{ 0 };

		// Pointer management

		void _reset_bounds() noexcept
		{
			lower_bound_ = 0;
			upper_bound_ = line_width_;
		}
		void _reset_ptrs() noexcept
		{
			current_frag_ = nullptr;

			_reset_bounds();

			line_ptr_ = 0;
			ptr_ = 0;
			ptr_local_ = 0;
			ptr_relative_ = 0;

			current_frag_ = _start();
			line_frag_ = current_frag_;
		}
		void _adjust_bounds() noexcept
		{
			const auto diff = upper_bound_ - lower_bound_;
			if (diff > line_width_)
			{
				const auto delta = diff - line_width_;
				upper_bound_ -= delta;
				if (diff <= lower_bound_)
					lower_bound_ -= diff;
			}
			else if (!upper_bound_)
			{
				_reset_bounds();
			}
		}

		// Searching

		const Fragment* _start() const noexcept
		{
			return start_.next.get();
		}
		Fragment* _start() noexcept
		{
			return start_.next.get();
		}

		bool _is_beg(const Fragment* frag) const noexcept
		{
			return frag->prev == &start_;
		}
		bool _is_beg(const Fragment* frag) noexcept
		{
			return frag->prev == &start_;
		}

		std::pair<Fragment*, std::size_t> _find_parent_of(std::size_t off, std::size_t rel, Fragment* fragment) noexcept
		{
			std::size_t idx = rel;
			while (fragment->next)
			{
				idx += fragment->next->size();
				if (idx >= off)
					break;
				fragment = fragment->next.get();
			}

			if (fragment->next)
			{
				const auto s = fragment->next->str();
				return std::make_pair(fragment, s.size() - (idx - off));
			}
			else
				return std::make_pair(fragment, ~0ull);
		}
		std::pair<Fragment*, std::size_t> _find_parent_of(std::size_t off) noexcept
		{
			return _find_parent_of(off, 0, &start_);
		}
		std::pair<Fragment*, std::size_t> _find_first_edl_reverse(std::size_t off, Fragment* fragment) noexcept
		{
			Fragment* current = fragment;
			std::size_t idx = 0;
			do
			{
				const auto s = current->str();
				const auto ssize = s.size();

				std::size_t l = 0;
				while (l < ssize)
				{
					if (s[ssize - l] == '\n')
					{
						return std::make_pair(current, idx);
					}
					else
					{
						l++;
						idx++;
					}
				}

				current = current->prev;
			} while (current->prev != nullptr);
			return std::make_pair(current, idx);
		}

		// Fragment operations

		template<typename Type>
		void _append_to_fragment(Fragment* fragment, Type&& v)
		{
			if (!fragment->size())
			{
				fragment->map(std::forward<Type>(v));
			}
			else if (
				fragment->dynamic() &&
				fragment->size() < max_fragment_size_
				)
			{
				fragment->append(v);
			}
			else
			{
				auto* f = _insert_at(fragment);
				f->map(std::forward<Type>(v));
			}
		}
		Fragment* _insert_at(Fragment* fragment)
		{
			auto saved = std::move(fragment->next);
			fragment->next = std::make_unique<Fragment>();
			if (saved != nullptr)
			{
				saved->prev = fragment->next.get();
				fragment->next->next = std::move(saved);
			}
			fragment->next->prev = fragment;
			return fragment->next.get();
		}
		Fragment* _split_fragment(Fragment* fragment, std::size_t split)
		{
			auto* nx = _insert_at(fragment);
			if (fragment->dynamic())
			{
				nx->map(fragment->str(split));
				fragment->map(fragment->str(0, split));
			}
			else
			{

				const auto s = fragment->str();
				nx->map(string_view{ s.begin() + split, s.end() });
				fragment->map(string_view{ s.begin(), s.begin() + split });
			}

			return fragment;
		}

		// Operations

		template<typename Type>
		void _insert(Type&& v, std::size_t pos, Fragment* cached = nullptr, std::size_t rel = ~0ull)
		{
			if (empty())
			{
				if constexpr (std::is_same_v<Type, Pixel::value_type>)
					reset(string(1, v));
				else
					reset(string(v.begin(), v.end()));
			}

			Fragment* fragment = nullptr;
			std::size_t relative = 0;
			if (cached)
			{
				fragment = cached->prev;
				relative = rel;
			}
			else
			{
				const auto [ f, r ] = _find_parent_of(pos);
				fragment = f;
				relative = r;
			}

			const bool mvptr = pos <= ptr_;
			if (current_frag_ == nullptr)
				current_frag_ = _start()->next.get();

			std::size_t len = 0;
			if constexpr (std::is_same_v<Type, Pixel::value_type>)
			{
				len = 1;
			}
			else
			{
				len = v.size();
			}

			// Append the value
			if (fragment->next == nullptr)
			{
				_append_to_fragment(fragment->next.get(), std::forward<Type>(v));
			}
			// Split the buffer and append
			else
			{
				if (
					!relative &&
					fragment->next->dynamic() &&
					fragment->next->size() < max_fragment_size_
					)
				{
					fragment->next->append(std::forward<Type>(v), 0);
				}
				else if (relative == fragment->next->size())
				{
					_append_to_fragment(fragment->next.get(), std::forward<Type>(v));
				}
				else
				{
					auto* frag = _split_fragment(fragment->next.get(), relative);
					_append_to_fragment(frag, std::forward<Type>(v));
				}
			}

			cached_size_ += len;
			if (mvptr) move_right(len);
		}
		void _overwrite(Pixel::value_type ch, std::size_t pos, Fragment* cached = nullptr, std::size_t rel = ~0ull)
		{
			if (empty())
				return;

			Fragment* fragment = nullptr;
			std::size_t relative = 0;
			if (cached)
			{
				fragment = cached->prev;
				relative = rel;
			}
			else
			{
				const auto [ f, r ] = _find_parent_of(pos);
				fragment = f;
				relative = r;
			}

			if (fragment->next->dynamic())
			{
				fragment->next->write(ch, relative);
			}
			else
			{
				auto* nx = _insert_at(fragment);
				nx->map(ch);
			}
		}

		void _erase(std::size_t pos, Fragment* cached = nullptr, std::size_t rel = ~0ull)
		{
			if (empty())
				return;

			Fragment* fragment = nullptr;
			std::size_t relative = 0;
			if (cached)
			{
				fragment = cached->prev;
				relative = rel;
			}
			else
			{
				const auto [ f, r ] = _find_parent_of(pos);
				fragment = f;
				relative = r;
			}

			const auto s = fragment->next->str();
			if (s.size() == 1)
			{
				auto nx = std::move(fragment->next->next);
				fragment->next = std::move(nx);
			}
			else if (fragment->next->dynamic())
			{
				fragment->next->erase(relative);
			}
			else
			{
				auto f1 = fragment->next.get();
				auto f2 = _insert_at(f1);
				f1->map(string_view{ s.begin(), s.begin() + relative });
				f2->map(string_view{ s.begin() + relative + 1, s.end() });
			}

			cached_size_--;
			if (pos <= ptr_) move_left();
		}
		void _erase(std::size_t min, std::size_t max)
		{
			if (empty())
				return;

			auto [ fragment, relative ] = _find_parent_of(min);

			const auto s = fragment->next->str();
			const auto len = max - min;

			if (relative == 0 && s.size() == len)
			{
				auto nx = std::move(fragment->next->next);
				fragment->next = std::move(nx);
			}
			else if (fragment->next->dynamic())
			{
				fragment->next->erase(min, len);
			}
			else
			{
				auto f1 = fragment->next.get();
				auto f2 = _insert_at(f1);
				f1->map(string_view{ s.begin(), s.begin() + relative });
				f2->map(string_view{ s.begin() + relative + len, s.end() });
			}

			cached_size_ -= len;
		}
	public:
		FragmentedBuffer() = default;
		explicit FragmentedBuffer(string_view str)
		{
			reset(str);
		}
		explicit FragmentedBuffer(const string& str)
		{
			reset(str);
		}
		explicit FragmentedBuffer(string&& str)
		{
			reset(std::move(str));
		}
		FragmentedBuffer(const FragmentedBuffer&) = delete;
		FragmentedBuffer(FragmentedBuffer&&) = default;
		~FragmentedBuffer() = default;

		// Pointers

		void set_line_width(std::size_t len) noexcept
		{
			line_width_ = len;
		}
		void set_line_height(std::size_t len) noexcept
		{
			line_height_ = len;
		}

		void move_left(std::size_t step = 1) noexcept
		{
			auto find_prev_edl = [this]() -> std::pair<Fragment*, std::size_t> {
				auto soff = 0ull;
				auto* sfrag = current_frag_;
				if (current_frag_->size() == 1)
				{
					soff = ptr_local_ - 1;
				}
				else
				{
					if (_is_beg(current_frag_))
						return std::make_pair(current_frag_, ptr_local_);
					sfrag = sfrag->prev;
					soff = sfrag->size() - 1;
				}
				return _find_first_edl_reverse(soff, sfrag);
			};

			if (empty())
				return;
			while (step)
			{
				bool is_pline = false;

				auto s = current_frag_->str();
				if (ptr_local_ == 0)
				{
					if (_is_beg(current_frag_))
						return;

					current_frag_ = current_frag_->prev;
					s = current_frag_->str();

					if (s[s.size() - 1] == '\n')
					{
						ptr_local_ = s.size() - 1;
						const auto [ frag, idx ] = find_prev_edl();
						ptr_--;
						ptr_relative_ = idx;
						line_frag_ = frag;
						is_pline = true;
					}
					else
					{
						ptr_--;
						ptr_relative_--;
						ptr_local_ = s.size() - 1;
					}
				}
				else
				{
					if (s[ptr_local_ - 1] == '\n')
					{
						ptr_local_--;
						const auto [ frag, idx ] = find_prev_edl();
						ptr_--;
						ptr_relative_ = idx;
						line_frag_ = frag;
						is_pline = true;
					}
					else
					{
						ptr_--;
						ptr_relative_--;
						ptr_local_--;
					}
				}

				if (is_pline)
				{
					upper_bound_ = ptr_relative_ + 1;
					lower_bound_ = line_width_ > ptr_relative_ ? 0 : ptr_relative_ - line_width_;
					line_ptr_--;
				}
				else if (ptr_relative_ < lower_bound_ && lower_bound_)
				{
					lower_bound_--;
					upper_bound_--;
				}

				step--;
			}
		}
		void move_right(std::size_t step = 1) noexcept
		{
			if (empty())
				return;
			while (step)
			{
				bool is_nline = false;
				const auto s = current_frag_->str();
				if (ptr_local_ + 1 >= s.size())
				{
					if (current_frag_->next == nullptr)
						return;

					current_frag_ = current_frag_->next.get();
					if (s[ptr_local_] == '\n')
					{
						ptr_++;
						ptr_relative_ = 0;
						ptr_local_ = 0;
						line_frag_ = current_frag_;
						is_nline = true;
					}
					else
					{
						ptr_++;
						ptr_relative_++;
						ptr_local_ = 0;
					}
				}
				else
				{
					if (s[ptr_local_] == '\n')
					{
						ptr_++;
						ptr_relative_ = 0;
						ptr_local_++;
						line_frag_ = current_frag_;
						is_nline = true;
					}
					else
					{
						ptr_++;
						ptr_relative_++;
						ptr_local_++;
					}
				}

				if (is_nline)
				{
					_reset_bounds();
					line_ptr_++;
				}
				else
				{
					_adjust_bounds();
					if (ptr_relative_ > upper_bound_)
					{
						upper_bound_++;
						lower_bound_++;
					}
				}

				step--;
			}
		}
		void move_up(std::size_t step = 1) noexcept
		{
			if (empty())
				return;
			// while (step && line_ptr_)
			// {
			// 	auto s = line_->str();
			// 	std::size_t f = 0;
			// 	const auto saved = ptr_relative_;
			// 	while ((f = s.rfind('\n', ptr_relative_)) == std::string::npos)
			// 	{
			// 		if (_is_beg(line))
			// 		{
			// 			ptr_relative_ = 0;
			// 			ptr_ = 0;
			// 			return;
			// 		}

			// 		line_ = line_->prev;
			// 		s = line_->str();

			// 		ptr_ -= ptr_relative_;
			// 		ptr_relative_ = s.size() - 1;
			// 	}

			// 	ptr_ -= (ptr_relative_ - f);
			// 	ptr_relative_ = f;

			// 	upper_bound_ = ptr_relative_;
			// 	lower_bound_ = upper_bound_ - line_width_;

			// 	line_ptr_--;
			// 	step--;
			// }
		}
		void move_down(std::size_t step = 1) noexcept
		{
			if (empty())
				return;
			// while (step)
			// {
			// 	auto s = line_->str();
			// 	std::size_t f = 0;
			// 	while ((f = s.find('\n', ptr_relative_)) == std::string::npos)
			// 	{
			// 		if (line_->next == nullptr)
			// 		{
			// 			ptr_relative_ = 0;
			// 			ptr_ = 0;
			// 			return;
			// 		}

			// 		line_ = line_->prev;
			// 		s = line_->str();

			// 		ptr_ -= ptr_relative_;
			// 		ptr_relative_ = s.size() - 1;
			// 	}
			// 	ptr_ -= (ptr_relative_ - f) + 1;
			// 	ptr_relative_ = f - 1;

			// 	upper_bound_ = ptr_relative_;
			// 	lower_bound_ = upper_bound_ - line_width_;

			// 	line_ptr_--;
			// 	step--;
			// }
		}
		void move_to(std::size_t pos)
		{
			_reset_ptrs();
			move_right(pos);
		}

		std::size_t ptr() const noexcept
		{
			return ptr_;
		}
		std::size_t ptr() noexcept
		{
			return ptr_;
		}

		// Output

		string concatenate() noexcept
		{
			string buffer;
			buffer.reserve(length());

			Fragment* fragment = _start();
			while (fragment->next)
			{
				const auto data = fragment->str();
				buffer.insert(buffer.end(), data.begin(), data.end());
				fragment = fragment->next.get();
			}

			return buffer;
		}
		string concatenate(std::size_t min, std::size_t max) noexcept
		{
			string buffer;
			buffer.reserve(max - min);

			auto [ parent, relative ] = _find_parent_of(min);
			auto [ last_parent, relative_last ] = _find_parent_of(max, min - relative, parent);

			auto* fragment = parent->next.get();
			auto* last = last_parent->next.get();
			while (fragment != last)
			{
				const auto s = fragment->str();
				buffer.insert(buffer.end(), s.begin(), s.end());
				fragment = fragment->next.get();
			}
			{
				const auto s = fragment->str();
				buffer.insert(buffer.end(), s.begin(), s.begin() + relative_last);
			}

			return buffer;
		}

		// State

		void reset(string_view str) noexcept
		{
			cached_size_ = str.size();
			source_.clear();
			start_.next = std::make_unique<Fragment>();
			start_.next->map(str);
			start_.next->prev = &start_;
			_reset_ptrs();
		}
		void reset(const string& str) noexcept
		{
			cached_size_ = str.size();
			source_ = str;
			start_.next = std::make_unique<Fragment>();
			start_.next->map(string_view(source_.begin(), source_.end()));
			start_.next->prev = &start_;
			_reset_ptrs();
		}
		void reset(string&& str) noexcept
		{
			cached_size_ = str.size();
			source_ = std::move(str);
			start_.next = std::make_unique<Fragment>();
			start_.next->map(string_view(source_.begin(), source_.end()));
			start_.next->prev = &start_;
			_reset_ptrs();
		}
		void set_max(std::size_t size) noexcept
		{
			max_fragment_size_ = size;
		}

		void foreach(auto callback)
		{
			Fragment* fragment = _start();
			std::size_t idx = 0;
			while (fragment->next)
			{
				auto data = fragment->str();
				for (decltype(auto) it : data)
				{
					callback(it, idx);
					idx++;
				}
				fragment = fragment->next.get();
			}
		}
		void foreach_scoped(auto callback, int window_offset)
		{
			if (empty())
				return;

			Fragment* fragment = line_frag_;
			if (fragment)
			{
				std::size_t idx = 0;
				std::size_t lidx = 0;
				std::size_t rel = 0;
				do
				{
					const auto data = fragment->str();
					std::size_t min = 0;
					std::size_t max = 0;
					if (fragment == line_frag_)
					{
						idx += lower_bound_;
						min = lower_bound_;
						max = std::min(upper_bound_, data.size());
					}
					else
					{
						min = 0;
						max = std::min(line_width_, data.size());
					}

					for (std::size_t i = 0; i < data.size(); i++, idx++, rel++)
					{
						callback(data[i], idx, rel, lidx);
						if (data[i] == '\n')
						{
							lidx++;
							if (lidx == line_height_)
							{
								rel = 0;
								return;
							}
							rel = -1;
							continue;
						}
					}
					fragment = fragment->next.get();
				} while (fragment);
			}
		}

		// Access

		template<typename Type>
		void insert(Type&& v, std::size_t pos)
		{
			_insert(std::forward<Type>(v), pos);
		}
		template<typename Type>
		void insert(Type&& v)
		{
			_insert(std::forward<Type>(v), ptr_, current_frag_, ptr_local_);
		}
		void overwrite(Pixel::value_type ch, std::size_t pos)
		{
			_overwrite(ch, pos);
		}
		void overwrite(Pixel::value_type ch)
		{
			_overwrite(ch, ptr_, current_frag_, ptr_local_);
		}

		void erase(std::size_t pos)
		{
			_erase(pos);
		}
		void erase(EraseMode mode)
		{
			switch (mode)
			{
			case EraseMode::Insert:
			{
				auto poff = 0ull;
				auto* pfrag = current_frag_;
				if (ptr_local_)
				{
					poff = ptr_local_ - 1;
				}
				else
				{
					if (!_is_beg(current_frag_))
					{
						pfrag = current_frag_->prev;
						poff = pfrag->size() - 1;
					}
					else
						return;
				}
				_erase(ptr_ - 1, pfrag, poff);
				break;
			}
			case EraseMode::Overwrite:
				_erase(ptr_, current_frag_, ptr_local_);
				break;
			case EraseMode::Delete:
			{
				// Not implemented yet
				throw std::logic_error{ "Not implemented" };
				break;
			}
			}
		}
		void erase(std::size_t min, std::size_t max)
		{
			_erase(min, max);
		}
		void clear() noexcept
		{
			source_.clear();
			cached_size_ = 0;
			start_.next.reset();
			_reset_ptrs();
		}

		// Data

		std::size_t length() const noexcept
		{
			// const Fragment* fragment = _start();
			// std::size_t len = 0;
			// while (fragment->next)
			// {
			// 	len += fragment->size();
			// 	fragment = fragment->next.get();
			// }
			// return len;
			return cached_size_;
		}
		std::size_t length() noexcept
		{
			return const_cast<const FragmentedBuffer*>(this)->length();
		}

		std::size_t size() const noexcept
		{
			return length();
		}
		std::size_t size() noexcept
		{
			return length();
		}

		bool empty() const noexcept
		{
			return _start() == nullptr || _start()->str().empty();
		}
		bool empty() noexcept
		{
			return const_cast<const FragmentedBuffer*>(this)->empty();
		}

		FragmentedBuffer& operator=(const FragmentedBuffer&) = delete;
		FragmentedBuffer& operator=(FragmentedBuffer&&) = default;
		FragmentedBuffer& operator=(string_view str) noexcept
		{
			reset(str);
			return *this;
		}
		FragmentedBuffer& operator=(const string& str) noexcept
		{
			reset(str);
			return *this;
		}
		FragmentedBuffer& operator=(string&& str) noexcept
		{
			reset(std::move(str));
			return *this;
		}
	};

	struct MultilineTextData
	{
		FragmentedBuffer value{};
		string pre{};

		Pixel background_color{};
		Pixel foreground_color{};

		Unit max_width{ 1, Unit::Pc, Unit::Horizontal };
		Unit max_height{};
		Unit x{};
		Unit y{};

		char zindex{ 0 };

		bool draw_ptr{ true };
		bool blink{ true };
		Pixel ptr_color{ .v = '_' };
	};

	class MultilineText :
		public Element,
		public MultilineTextData
	{
	public:
		using data = MultilineTextData;
	protected:
		mt::future<void> btask_{};
		IOContext::EventListener seq_ev_{};
		IOContext::EventListener key_ev_{};
		bool ptr_blink_{ true };
		bool insert_mode_{ true };

		void _write_ptr(Pixel& px)
		{
			if (data::blink && !ptr_blink_)
			{
				px.v = ' ';
			}
			else
				px = data::ptr_color;
		}
		void _start_blink()
		{
			if (data::blink && getstate(State::Focused) && btask_ == nullptr)
			{
				btask_ = _context()->scheduler()->launch_cyclic_task([e = ElementResource<MultilineText>(handle())]() mutable {
					dx.write([](MultilineText& dx) {
						e.ptr_blink_ = !e.ptr_blink_;
					});
				}, std::chrono::milliseconds(500));
			}
		}
		void _stop_blink()
		{
			if (btask_ != nullptr)
				btask_.stop();
			ptr_blink_ = false;
		}

		virtual void _set_unit_context_impl() override
		{
			data::x.setcontext(Unit::Horizontal);
			data::y.setcontext(Unit::Vertical);
			data::max_width.setcontext(Unit::Horizontal);
			data::max_height.setcontext(Unit::Vertical);
		}
		virtual void _state_change_impl(State state, bool value) override
		{
			if (state == State::Focused)
			{
				if (value)
				{
					_start_blink();
				}
				else
				{
					_stop_blink();
				}
			}
			else if (state == State::Swapped)
			{
				if (value)
				{
					if (seq_ev_)
						_context()->mute(seq_ev_);
					if (key_ev_)
						_context()->mute(key_ev_);
					_stop_blink();
				}
				else
				{
					key_ev_ = _context()->listen<d2::IOContext::Event::KeyInput>([
						this, ictx = _context()
					](auto, auto, auto) {
						ElementResource<MultilineText> elem = handle();
						if (!elem.getstate(State::Focused))
							return;

						if (ictx->input()->is_pressed(d2::SystemInput::Backspace))
						{
							const auto lock = elem.handle().lock_for_write();
							if (!data::value.empty())
								data::value.erase(insert_mode_ ? FragmentedBuffer::EraseMode::Insert : FragmentedBuffer::EraseMode::Overwrite);
						}
						else if (ictx->input()->is_pressed(d2::SystemInput::Delete))
						{
							const auto lock = elem.handle().lock_for_write();
							if (data::value.ptr() < data::value.size())
								data::value.erase(FragmentedBuffer::EraseMode::Delete);
						}
						else if (ictx->input()->is_pressed(d2::SystemInput::Enter))
						{
							const auto lock = elem.handle().lock_for_write();
							data::value.insert('\n', data::value.ptr());
						}
						else if (ictx->input()->is_pressed(d2::SystemInput::Escape))
						{
							if (elem.getstate(Focused))
								ctx_->src->focus(nullptr);
						}
						else if (ictx->input()->is_pressed(d2::SystemInput::Insert))
						{
							const auto lock = elem.handle().lock_for_write();
							insert_mode_ = true;
						}
						else if (ictx->input()->is_pressed(d2::SystemInput::ArrowLeft))
						{
							const auto lock = elem.handle().lock_for_write();
							data::value.move_left();
						}
						else if (ictx->input()->is_pressed(d2::SystemInput::ArrowRight))
						{
							const auto lock = elem.handle().lock_for_write();
							data::value.move_right();
						}
						else if (ictx->input()->is_pressed(d2::SystemInput::ArrowDown))
						{
							const auto lock = elem.handle().lock_for_write();
							data::value.move_down();
						}
						else if (ictx->input()->is_pressed(d2::SystemInput::ArrowUp))
						{
							const auto lock = elem.handle().lock_for_write();
							data::value.move_up();
						}
					});
					seq_ev_ = _context()->listen<d2::IOContext::Event::KeySequenceInput>([
						this, ictx = _context()
					](auto, auto, auto) {
						ElementResource<MultilineText> elem = handle();
						if (!elem.getstate(State::Focused))
							return;

						auto seq = ictx->input()->key_sequence();
						if (ictx->input()->is_pressed(d2::SystemInput::Shift))
						{
							std::transform(seq.cbegin(), seq.cend(), seq.begin(), [](char ch) -> char {
								return std::toupper(ch);
							});
						}

						const auto lock = elem.handle().lock_for_write();
						data::value.insert(std::move(seq));
					});
					_start_blink();
				}
			}
		}
		virtual void _frame_impl() override
		{
			const auto bbox = box();
			auto ctx = _context();

			// Draw the pre-value
			{
				buffer_.fill(data::background_color);
				if (!data::pre.empty())
				{
					const auto h = resolve_units(data::max_height, true);
					for (std::size_t j = 0; j < h; j++)
					{
						for (std::size_t i = 0; i < data::pre.size(); i++)
						{
							auto& p = buffer_.at(i, j);
							p.v = data::pre[i];
						}
					}
				}
			}

			// Draw the buffer
			{
				int d = 0;
				const auto off = data::pre.size();
				data::value.set_line_width(resolve_units(data::max_width, true) - data::pre.size());
				data::value.set_line_height(bbox.height);
				data::value.foreach_scoped([this, &d, &off](
					auto ch,
					const auto idx,
					const auto rel,
					const auto line
				) {
					if (btask_ != nullptr && idx == data::value.ptr())
					{
						Pixel& px = buffer_.at(rel + off, line);
						_write_ptr(px);
						d = 1;
					}
					if (ch != '\n')
					{
						Pixel& px = buffer_.at(rel + d + off, line);
						px.v = ch;
					}
					else
						d = 0;
				}, 0);
			}
		}
		virtual void _box_dimensions_impl() noexcept override
		{
			const auto ctx = _context().get();
			bounding_box_.width = resolve_units(data::max_width, true);
			bounding_box_.height = resolve_units(data::max_height, true);
		}
		virtual void _position_impl() noexcept override
		{
			const auto ctx = _context().get();
			bounding_box_.x = resolve_units(data::x);
			bounding_box_.y = resolve_units(data::y);
		}
	public:
		MultilineText(
			const std::string& str,
			Immutable* parent,
			MountSharedResources* res,
			TreeState::ptr ctx,
			MultilineTextData data
			) :
			Element(str, parent, res, ctx),
			MultilineTextData(std::move(data)) {}

		virtual char getzindex() const noexcept override
		{
			return data::zindex;
		}
	};
} // d2::elem

#endif // D2_MULTILINE_INPUT_H
