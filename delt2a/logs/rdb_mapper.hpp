#ifndef RDB_MAPPER_HPP
#define RDB_MAPPER_HPP

#include <filesystem>
#include <span>
#include <rdb_utils.hpp>

namespace rdb
{
    namespace impl
    {
        using handle = int;
        enum class Access
        {
            Default,
            Sequential,
            Random,
            Hot,
            Cold,
            Huge,
            Discard,
        };
        enum OpenMode : unsigned char
        {
            Read = 1 << 0,
            Write = 1 << 1,
            Execute = 1 << 2,

            RW = Read | Write,
            RWE = Read | Write | Execute,
            RO = Read
        };

        handle open(const std::filesystem::path& path, unsigned char mode) noexcept;
        std::pair<void*, std::size_t> vmap(handle handle, unsigned char mode) noexcept;
        void* map(handle handle, std::size_t size, unsigned char mode) noexcept;
        void* remap(handle handle, void* ptr, std::size_t old, std::size_t size) noexcept;
        bool close(handle handle) noexcept;
        bool unmap(void* ptr, std::size_t size) noexcept;

        bool flush(handle handle, void* ptr, std::size_t pos, std::size_t size) noexcept;
        bool flush(handle handle) noexcept;

        bool reserve_aligned(handle handle, std::size_t size) noexcept;
        bool reserve(handle handle, std::size_t size) noexcept;

        bool write(handle handle, std::size_t off, void* vec) noexcept;
        bool write(handle handle, std::size_t off, std::span<const unsigned char> data) noexcept;
        bool write(handle handle, std::size_t off, std::span<std::span<const unsigned char>> datas) noexcept;
        bool write(handle handle, std::size_t off, std::span<const View> datas) noexcept;
        View read(handle handle, std::size_t off, std::size_t size) noexcept;

        std::size_t size(handle handle) noexcept;

        void hint(handle handle, void* ptr, std::size_t off, std::size_t size, Access hint) noexcept;
    }

    class HandleView
    {
    public:
        using Access = impl::Access;
        using OpenMode = impl::OpenMode;
        using handle = impl::handle;
    protected:
        handle _handle{ -1 };
        void* _ptr{ nullptr };
        std::size_t _length{ 0 };
    public:
        HandleView() = default;
        HandleView(handle handle) noexcept;
        HandleView(handle handle, void* ptr, std::size_t length) noexcept;
        HandleView(const HandleView&) = default;
        HandleView(HandleView&& copy) = default;

        bool mapped() const noexcept;
        bool opened() const noexcept;

        std::size_t size() const noexcept;

        std::span<unsigned char> memory() const noexcept;

        void flush(std::size_t pos, std::size_t size, bool discard = true) const noexcept;
        void flush(bool discard = true) const noexcept;

        void write(std::size_t offset, std::initializer_list<View> li) const noexcept;
        void write(std::size_t offset, const View& data) const noexcept;
        void write(std::size_t offset, char ch) const noexcept;
        void write(const View& data) const noexcept;
        void write(std::size_t offset, const std::string& str) const noexcept;
        void write(std::size_t offset, std::string_view str) const noexcept;
        View read(std::size_t offset, std::size_t count) const noexcept;
        unsigned char read(std::size_t off) const noexcept;

        void hint(Access acc, std::size_t off = 0, std::size_t size = ~0ull) const noexcept;

        HandleView& operator=(const HandleView&) = default;
        HandleView& operator=(HandleView&& copy) noexcept = default;
    };

    class ReducedMapper : public HandleView
    {
    private:
        void _move(ReducedMapper&& copy) noexcept;
    public:
        ReducedMapper() = default;
        ReducedMapper(const ReducedMapper&) = delete;
        ReducedMapper(ReducedMapper&& copy) { _move(std::move(copy)); }
        ~ReducedMapper() { close(); }

        void reserve(std::size_t size) noexcept;
        void reserve_aligned(std::size_t required) noexcept;

        void map(const std::filesystem::path& path, std::size_t length, unsigned char flags = OpenMode::RW) noexcept;
        void map(const std::filesystem::path& path, unsigned char flags = OpenMode::RW) noexcept;
        void map(std::size_t length, unsigned char flags = OpenMode::RW) noexcept;
        void map(unsigned char flags = OpenMode::RW) noexcept;
        void unmap(bool full = false) noexcept;

        void open(const std::filesystem::path& path, std::size_t reserve, unsigned char flags = OpenMode::RW) noexcept;
        void open(const std::filesystem::path& path, unsigned char flags = OpenMode::RW) noexcept;
        void close() noexcept;

        ReducedMapper& operator=(const ReducedMapper&) = delete;
        ReducedMapper& operator=(ReducedMapper&& copy) noexcept { _move(std::move(copy)); return *this; }
    };
    class Mapper : public ReducedMapper
    {
    public:
        using Access = impl::Access;
        using OpenMode = impl::OpenMode;
    private:
        std::size_t _vmap_offset{ false };
        std::size_t _flush_offset{ 0 };
        std::filesystem::path _filepath{ "" };

        void _move(Mapper&& copy) noexcept;
    public:
        using ReducedMapper::map;

        Mapper() noexcept = default;
        Mapper(const Mapper&) noexcept = delete;
        Mapper(Mapper&& copy) noexcept { _move(std::move(copy)); }
        ~Mapper() noexcept { close(); }

        std::filesystem::path path() const noexcept;

        unsigned char* append() const noexcept;

        void map(const std::filesystem::path& path, std::size_t length, unsigned char flags = OpenMode::RW) noexcept;
        void map(const std::filesystem::path& path, unsigned char flags = OpenMode::RW) noexcept;

        void open(const std::filesystem::path& path, std::size_t reserve, unsigned char flags = OpenMode::RW) noexcept;
        void open(const std::filesystem::path& path, unsigned char flags = OpenMode::RW) noexcept;

        void vmap(unsigned char flags = OpenMode::RW) noexcept;
        void vmap_reserve(std::size_t size) noexcept;
        void vmap_reset(std::size_t size = 0) noexcept;
        void vmap_increment(std::size_t size) noexcept;
        void vmap_decrement(std::size_t size) noexcept;
        void vmap_flush() noexcept;
        std::size_t vmap_offset() noexcept;

        void remove() noexcept;

        Mapper& operator=(const Mapper&) = delete;
        Mapper& operator=(Mapper&& copy) noexcept { _move(std::move(copy)); return *this; }
    };
}

#endif // RDB_MAPPER_HPP
