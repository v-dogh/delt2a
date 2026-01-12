#include <rdb_mapper.hpp>
#include <rdb_memunits.hpp>
#include <numeric>
#ifdef __unix__
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/uio.h>
#include <unistd.h>
#else
#error(platform unsupported)
#endif

namespace rdb
{
    namespace impl
    {
    #   ifdef __unix__
            unsigned int flagconv(handle handle, unsigned char mode) noexcept
            {
                const auto handle_mode = fcntl(handle, F_GETFL);
                if (handle_mode == -1)
                    return PROT_READ;
                const auto acc = handle_mode & O_ACCMODE;
                const auto masked = mode & (
                    (OpenMode::Write * !bool(acc == O_RDONLY)) |
                    ((OpenMode::Read | OpenMode::Execute) & mode)
                );
                return
                    ((masked & OpenMode::Write) ? PROT_WRITE : 0x00) |
                    ((masked & OpenMode::Read) ? PROT_READ : 0x00) |
                    (((masked & OpenMode::Execute)) ? PROT_EXEC : 0x00);
            }
            handle open(const std::filesystem::path& path, unsigned char mode) noexcept
            {
                return ::open(
                    path.c_str(),
                    (mode & OpenMode::Write ? O_RDWR | O_CREAT : O_RDONLY),
                    0666
                    );
            }
            std::pair<void*, std::size_t> vmap(handle handle, unsigned char mode) noexcept
            {
                const auto flags = flagconv(handle, mode);
                std::size_t peak = mem::GiB(10024);
                void* ptr = nullptr;
                while (
                    (ptr = ::mmap(nullptr, peak,
                            flags,
                            MAP_ANONYMOUS | MAP_PRIVATE | MAP_NORESERVE,
                            -1, 0
                        )) == MAP_FAILED
                    ) peak /= 2;
                return { ptr, peak };
            }
            void* map(handle handle, std::size_t size, unsigned char mode) noexcept
            {
                const auto flags = flagconv(handle, mode);
                const auto ptr = ::mmap(nullptr, size,
                    flags,
                    MAP_SHARED,
                    handle, 0
                );
                if (ptr == MAP_FAILED)
                    return nullptr;
                return ptr;
            }
            void* remap(handle handle, void* ptr, std::size_t old, std::size_t size) noexcept
            {
                return ::mremap(ptr, old, size, MAP_SHARED);
            }
            bool close(handle handle) noexcept
            {
                return ::close(handle) == 0;
            }
            bool unmap(void* ptr, std::size_t size) noexcept
            {
                return ::munmap(ptr, size) == 0;
            }

            bool flush(handle handle, void* ptr, std::size_t pos, std::size_t size) noexcept
            {
                if (::msync(
                        static_cast<char*>(ptr) + pos,
                        size,
                        MS_SYNC
                    ) == 0)
                {
                    hint(handle, ptr, pos, size, Access::Discard);
                    return true;
                }
                return false;
            }
            bool flush(handle handle) noexcept
            {
                return fsync(handle) == 0;
            }

            bool reserve_aligned(handle handle, std::size_t size) noexcept
            {
                struct statvfs vfs;
                if (::fstatvfs(handle, &vfs) != 0)
                    return reserve(handle, size);

                const auto page = ::sysconf(_SC_PAGESIZE);
                const auto block = vfs.f_frsize;
                const auto base = std::lcm(page, block);

                if (base == 0)
                    return reserve(handle, size);
                return reserve(handle, ((size + base - 1) / base) * base);
            }
            bool reserve(handle handle, std::size_t size) noexcept
            {
                ::ftruncate(handle, size);
                ::fsync(handle);
                return true;
            }

            bool write(handle handle, std::size_t off, std::span<const unsigned char> data) noexcept
            {
                return ::pwrite(handle, data.data(), data.size(), off) == 0;
            }
            bool write(handle handle, std::size_t off, std::span<std::span<const unsigned char>> datas) noexcept
            {
                std::vector<iovec> vec;
                vec.resize(datas.size());
                for (std::size_t i = 0; i < datas.size(); i++)
                {
                    vec[i].iov_base = const_cast<unsigned char*>(datas.begin()[i].data());
                    vec[i].iov_len = datas.begin()[i].size();
                }
                return ::pwritev(handle, vec.data(), vec.size(), off) == 0;
            }
            bool write(handle handle, std::size_t off, std::span<const View> datas) noexcept
            {
                std::vector<iovec> vec;
                vec.resize(datas.size());
                for (std::size_t i = 0; i < datas.size(); i++)
                {
                    vec[i].iov_base = const_cast<unsigned char*>(datas.begin()[i].data().data());
                    vec[i].iov_len = datas.begin()[i].size();
                }
                return ::pwritev(handle, vec.data(), vec.size(), off) == 0;
            }
            View read(handle handle, std::size_t off, std::size_t size) noexcept
            {
                View data = View::copy(size);
                if (::pread(handle, data.mutate().data(), size, off) != 0)
                    return View();
                return data;
            }

            std::size_t size(handle handle) noexcept
            {
                struct stat info;
                if (::fstat(handle, &info) == 0)
                    return info.st_size;
                return 0;
            }

            void hint(handle handle, void* ptr, std::size_t off, std::size_t size, Access hint) noexcept
            {
                int flag;
                switch (hint)
                {
                    case Access::Default: flag = MADV_NORMAL; break;
                    case Access::Sequential: flag = MADV_SEQUENTIAL; break;
                    case Access::Random: flag = MADV_RANDOM; break;
                    case Access::Hot: flag = MADV_WILLNEED; break;
                    case Access::Cold: flag = MADV_COLD; break;
                    case Access::Huge: flag = MADV_HUGEPAGE; break;
                    case Access::Discard: flag = MADV_DONTNEED; break;
                }
                ::madvise(
                    static_cast<unsigned char*>(ptr) + off,
                    size,
                    flag
                );
            }
    #   endif
    }

    HandleView::HandleView(handle handle) noexcept :
        _handle(handle)
    {}
    HandleView::HandleView(handle handle, void* ptr, std::size_t length) noexcept :
        _handle(handle),
        _ptr(ptr),
        _length(length)
    {}

    bool HandleView::mapped() const noexcept
    {
        return _handle != -1 && _ptr != nullptr;
    }
    bool HandleView::opened() const noexcept
    {
        return _handle != -1;
    }

    std::size_t HandleView::size() const noexcept
    {
        return _length;
    }

    std::span<unsigned char> HandleView::memory() const noexcept
    {
        return {
            static_cast<unsigned char*>(_ptr),
            _length
        };
    }

    void HandleView::flush(std::size_t pos, std::size_t size, bool discard) const noexcept
    {
        impl::flush(_handle, _ptr, pos, size);
        if (discard)
            impl::hint(_handle, _ptr, 0, _length, Access::Discard);
    }
    void HandleView::flush(bool discard) const noexcept
    {
        impl::flush(_handle);
    }

    void HandleView::write(std::size_t offset, std::initializer_list<View> li) const noexcept
    {
        impl::write(_handle, offset, std::span(li.begin(), li.end()));
    }
    void HandleView::write(std::size_t offset, const View& data) const noexcept
    {
        impl::write(_handle, offset, data);
    }
    void HandleView::write(std::size_t offset, char ch) const noexcept
    {
        impl::write(_handle, offset, std::span<const unsigned char>(
            reinterpret_cast<const unsigned char*>(&ch), 1
        ));
    }
    void HandleView::write(const View& data) const noexcept
    {
        write(0, data);
    }
    void HandleView::write(std::size_t offset, const std::string& str) const noexcept
    {
        impl::write(
            _handle,
            offset,
            std::span<const unsigned char>(
                reinterpret_cast<const unsigned char*>(str.data()),
                str.size()
            )
        );
    }
    void HandleView::write(std::size_t offset, std::string_view str) const noexcept
    {
        impl::write(
            _handle,
            offset,
            std::span<const unsigned char>(
                reinterpret_cast<const unsigned char*>(str.data()),
                str.size()
            )
        );
    }
    View HandleView::read(std::size_t offset, std::size_t count) const noexcept
    {
        return impl::read(
            _handle,
            offset,
            count
        );
    }
    unsigned char HandleView::read(std::size_t off) const noexcept
    {
        return impl::read(
            _handle,
            off,
            1
        ).data()[0];
    }

    void HandleView::hint(Access acc, std::size_t off, std::size_t size) const noexcept
    {
        impl::hint(_handle, _ptr, off, size, acc);
    }

    // Reduced Mapper

    void ReducedMapper::_move(ReducedMapper&& copy) noexcept
    {
        _ptr = copy._ptr;
        _length = copy._length;
        _handle = copy._handle;
        copy._handle = -1;
        copy._ptr = nullptr;
        copy._length = 0;
    }

    void ReducedMapper::reserve(std::size_t size) noexcept
    {
        impl::reserve(_handle, size);
    }
    void ReducedMapper::reserve_aligned(std::size_t required) noexcept
    {
        impl::reserve_aligned(_handle, required);
    }

    void ReducedMapper::map(const std::filesystem::path& path, std::size_t length, unsigned char flags) noexcept
    {
        if (std::filesystem::exists(path))
            open(path, flags);
        else
            open(path, length, flags);
        map(length, flags);
    }
    void ReducedMapper::map(const std::filesystem::path& path, unsigned char flags) noexcept
    {
        open(path, flags);
        map(flags);
    }
    void ReducedMapper::map(std::size_t length, unsigned char flags) noexcept
    {
        _ptr = impl::map(_handle, length, flags);
        _length = length;
    }
    void ReducedMapper::map(unsigned char flags) noexcept
    {
        map(impl::size(_handle));
    }
    void ReducedMapper::unmap(bool full) noexcept
    {
        if (mapped())
        {
            impl::unmap(_ptr, _length);
            _ptr = nullptr;
            _length = 0;
        }
        if (full)
            close();
    }

    void ReducedMapper::open(const std::filesystem::path& path, std::size_t reserve, unsigned char flags) noexcept
    {
        if (opened())
            close();
        open(path, flags);
        if (opened())
            impl::reserve(_handle, reserve);
    }
    void ReducedMapper::open(const std::filesystem::path& path, unsigned char flags) noexcept
    {
        if (opened())
            close();
        _handle = impl::open(path, flags);
    }
    void ReducedMapper::close() noexcept
    {            
        if (opened())
        {
            if (mapped())
                unmap(false);
            impl::close(_handle);
            _handle = -1;
        }
    }

    // Full Mapper

    void Mapper::_move(Mapper&& copy) noexcept
    {
        _ptr = copy._ptr;
        _length = copy._length;
        _filepath = std::move(copy._filepath);
        _vmap_offset = copy._vmap_offset;
        _handle = copy._handle;
        copy._handle = -1;
        copy._ptr = nullptr;
        copy._length = 0;
    }

    std::filesystem::path Mapper::path() const noexcept
    {
        return _filepath;
    }

    unsigned char* Mapper::append() const noexcept
    {
        return static_cast<unsigned char*>(_ptr) + _vmap_offset;
    }

    void Mapper::map(const std::filesystem::path& path, std::size_t length, unsigned char flags) noexcept
    {
        _filepath = path;
        ReducedMapper::map(path, length, flags);
    }
    void Mapper::map(const std::filesystem::path& path, unsigned char flags) noexcept
    {
        _filepath = path;
        ReducedMapper::map(path, flags);
    }

    void Mapper::open(const std::filesystem::path& path, std::size_t reserve, unsigned char flags) noexcept
    {
        _filepath = path;
        ReducedMapper::open(path, reserve, flags);
    }
    void Mapper::open(const std::filesystem::path& path, unsigned char flags) noexcept
    {
        _filepath = path;
        ReducedMapper::open(path, flags);
    }

    void Mapper::vmap(unsigned char flags) noexcept
    {
        if (mapped())
            unmap();

        const auto [ ptr, size ] = impl::vmap(_handle, flags);
        _ptr = ptr;
        _length = size;
        _vmap_offset = 0;
        _flush_offset = 0;
    }
    void Mapper::vmap_reserve(std::size_t size) noexcept
    {
        _vmap_offset = size;
    }
    void Mapper::vmap_reset(std::size_t size) noexcept
    {
        _vmap_offset = size;
    }
    void Mapper::vmap_increment(std::size_t size) noexcept
    {
        _vmap_offset += size;
    }
    void Mapper::vmap_decrement(std::size_t size) noexcept
    {
        _vmap_offset -= size;
    }
    void Mapper::vmap_flush() noexcept
    {
        if (_vmap_offset == _flush_offset)
            return;

        impl::write(
            _handle,
            _flush_offset,
            memory().subspan(_flush_offset, _vmap_offset)
        );
        impl::hint(_handle, _ptr, _flush_offset, _vmap_offset, Access::Discard);
        _flush_offset = _vmap_offset;
    }
    std::size_t Mapper::vmap_offset() noexcept
    {
        return _vmap_offset;
    }

    void Mapper::remove() noexcept
    {
        close();
        std::filesystem::remove(_filepath);
    }
}



