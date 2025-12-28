#ifndef D2_FILE_EXPLORER_HPP
#define D2_FILE_EXPLORER_HPP

#include "../elements/d2_flow_box.hpp"
#include <delt2a/d2_dsl.hpp>
#include <filesystem>

namespace d2::ctm
{
    namespace impl
    {
        class FilesystemExplorerBase
        {
        private:
            std::vector<string> history_{};
            string softselect_{};
            std::filesystem::path path_{ "/" };
            int rcnt_{ 0 };
        protected:
            D2_STATELESS_TREE_FORWARD(interface, bool window)

            void _move_to(const std::string& filename);

            virtual std::size_t _path_max_width() = 0;
            virtual void _update_path_indicator(const std::string& path) = 0;
            virtual bool _is_submit_callback() = 0;
            virtual void _submit_path() = 0;
            virtual void _update_results() = 0;
        public:
            void forwards();
            void backwards();
            void refresh();
            void rselect(const string& filename);
            void sselect(const string& filename);
            void setpath(std::filesystem::path path);
            std::filesystem::path path() const;

            void submit_soft();
            void submit();
        };
    }

    class FilesystemExplorerWindow :
        public style::UAIC<
            dx::VirtualFlowBox,
            FilesystemExplorerWindow,
            style::IResponsive<FilesystemExplorerWindow, std::filesystem::path>::type
        >,
        public impl::FilesystemExplorerBase
	{
	public:
        using data = style::UAIC<
            VirtualFlowBox,
            FilesystemExplorerWindow,
            style::IResponsive<FilesystemExplorerWindow, std::filesystem::path>::type
        >;
        D2_UAI_CHAIN(FilesystemExplorerWindow)
	protected:
		virtual void _state_change_impl(State state, bool value) override;
        virtual bool _is_submit_callback() override;
        virtual void _submit_path() override;
        virtual std::size_t _path_max_width() override;
        virtual void _update_path_indicator(const std::string& path) override;
        virtual void _update_results() override;
    };

    class FilesystemExplorer :
        public style::UAIC<
            dx::FlowBox,
            FilesystemExplorer,
            style::IResponsive<FilesystemExplorer, std::filesystem::path>::type
        >,
        public impl::FilesystemExplorerBase
    {
    public:
        using data = style::UAIC<
            FlowBox,
            FilesystemExplorer,
            style::IResponsive<FilesystemExplorer, std::filesystem::path>::type
        >;
        D2_UAI_CHAIN(FilesystemExplorer)
    protected:
        virtual void _state_change_impl(State state, bool value) override;
        virtual bool _is_submit_callback() override;
        virtual void _submit_path() override;
        virtual std::size_t _path_max_width() override;
        virtual void _update_path_indicator(const std::string& path) override;
        virtual void _update_results() override;
    };
}

#endif // D2_FILE_EXPLORER_HPP
