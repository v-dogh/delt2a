#ifndef D2_FILE_EXPLORER_HPP
#define D2_FILE_EXPLORER_HPP

#include "../elements/d2_flow_box.hpp"
#include <filesystem>

namespace d2::ctm
{
    class FilesystemExplorer : public style::UAIC<dx::VirtualFlowBox, FilesystemExplorer, style::IResponsive>
	{
	public:
        using data = style::UAIC<VirtualFlowBox, FilesystemExplorer, style::IResponsive>;
		using data::data;
		D2_UAI_CHAIN(FilesystemExplorer)
	protected:
        std::vector<string> history_{};
        string softselect_{};
		std::filesystem::path path_{ "/" };
		int rcnt_{ 0 };

		virtual void _state_change_impl(State state, bool value) override;
		void _update_results();
	public:
		void forwards();
		void backwards();
		void refresh();
        void rselect(const string& filename);
        void sselect(const string& filename) noexcept;
		void setpath(std::filesystem::path path);
		std::filesystem::path getpath() const noexcept;

		void submit_soft();
		void submit();
	};
}

#endif // D2_FILE_EXPLORER_HPP
