#ifndef D2_FILE_EXPLORER_HPP
#define D2_FILE_EXPLORER_HPP

#include "../elements/d2_draggable_box.hpp"
#include <filesystem>

namespace d2::ctm
{
	class FilesystemExplorer : public style::UAIC<dx::VirtualBox, FilesystemExplorer, style::IResponsive>
	{
	public:
		using data = style::UAIC<VirtualBox, FilesystemExplorer, style::IResponsive>;
		using data::data;
		D2_UAI_CHAIN(FilesystemExplorer)
	protected:
		std::vector<std::string> history_{};
		std::string softselect_{ "" };
		std::filesystem::path path_{ "/" };
		int rcnt_{ 0 };

		virtual void _state_change_impl(State state, bool value) override;
		void _update_results();
	public:
		void forwards();
		void backwards();
		void refresh();
		void rselect(const std::string& filename);
		void sselect(const std::string& filename) noexcept;
		void setpath(std::filesystem::path path);
		std::filesystem::path getpath() const noexcept;

		void submit_soft();
		void submit();
	};
}

#endif // D2_FILE_EXPLORER_HPP
