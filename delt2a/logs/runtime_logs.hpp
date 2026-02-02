#ifndef RUNTIME_LOGS_HPP
#define RUNTIME_LOGS_HPP

#include <source_location>
#include <filesystem>
#include <algorithm>
#include <streambuf>
#include <exception>
#include <cstring>
#include <fstream>
#include <vector>
#include <format>
#include <atomic>
#include <memory>
#include <array>
#include <span>

namespace rs
{
    enum class Severity : unsigned char
    {
        // Internally used for linking logs to data
        Reserved = 1 << 0,
        // Fine grained information regarding more complex operations
        Verbose = 1 << 1,
        // Regular information regarding the function of operations
        Info = 1 << 2,
        // Used for debug messages
        Debug = 1 << 3,
        // Module events (e.g. during loading or shutdown)
        Module = 1 << 4,
        // Information that signals not an error but a possible misconfiguration or unintended effect
        Warning = 1 << 5,
        // A controlled error during the lifetime of an operation
        Error = 1 << 6,
        // An uncontrolled error during the lifetime of an operation (e.g. an out of memory exception)
        Critical = 1 << 7,
    };
    enum class Filter : unsigned char
    {
        // Filters to only output
        Important = unsigned(Severity::Verbose) | unsigned(Severity::Info),
        // Filters to discard verbose output
        Info = unsigned(Severity::Verbose),
        // Clears all filters
        Clear = 0x00
    };

    std::string_view signal_to_str(int sig) noexcept;
    std::string_view severity_to_str(Severity severity) noexcept;

    struct Config
    {
        std::filesystem::path root{};
        std::size_t max_log_data_memory{ 2 * 1024 };
        std::size_t max_log_memory{ 10 * 1024 };
        std::size_t min_sync_count{ std::numeric_limits<std::size_t>::max() };
        bool handle_signals{ true };
        bool synchronize{ false };
    };
    struct LogEntry
    {
        Severity severity{};
        std::chrono::system_clock::time_point timestamp{};
        std::uint32_t code{ 0x00 };
        std::string_view module{};
        std::string_view msg{};
        std::string_view data{};
    };

    class FlushOp {} inline flush;
    class LogOp
    {
    public:
        Severity severity{ Severity::Debug };
        std::string_view value;
        LogOp operator()(Severity severity, std::string_view module)
        {
            return LogOp(severity, module);
        }
        LogOp operator()(std::string_view module)
        {
            return LogOp(Severity::Debug, module);
        }
    } inline log;
    class DataOp
    {
    public:
        std::string_view value;
        DataOp operator()(std::string_view v)
        {
            return DataOp(v);
        }
    } inline dat;
    class CodeOp
    {
    public:
        std::uint32_t value;
        CodeOp operator()(std::uint32_t v)
        {
            return CodeOp(v);
        }
    } inline code;

    class RuntimeLogSink;
    class RuntimeLogs : std::enable_shared_from_this<RuntimeLogs>
    {
    public:
        static constexpr auto invalid_code = std::numeric_limits<std::uint32_t>::max();
        using code = std::uint32_t;
        using ptr = std::shared_ptr<RuntimeLogs>;
    private:
        template<typename Type>
        static void _write(void* buffer, Type&& value)
        {
            std::memcpy(
                buffer,
                reinterpret_cast<const unsigned char*>(&value),
                sizeof(Type)
            );
        }
        template<typename Type>
        static void _write(void* buffer, Type&& value, std::size_t& ctr)
        {
            std::memcpy(
                reinterpret_cast<unsigned char*>(buffer) + ctr,
                reinterpret_cast<const unsigned char*>(&value),
                sizeof(Type)
            );
            ctr += sizeof(Type);
        }

        struct DataHeader
        {
            std::uint64_t length{ 0 };
        };
        struct LogHeader
        {
            union
            {
                struct
                {
                    std::chrono::steady_clock::time_point timestamp{}; // timestamp relative to baseline
                    std::uint32_t code{ 0x00 }; // optional code
                    std::uint16_t length{ 0 }; // message length
                    std::uint8_t md_length{ 0 }; // module length
                };
                std::uint64_t data_offset;
            };
            Severity severity{}; // severity of the log

            void bin(unsigned char* buffer);
            static std::size_t size();
        };
        struct DumpHeader
        {
            std::uint16_t endian{ 0xFF00 };
            std::chrono::system_clock::time_point::rep baseline_system{};
            std::chrono::steady_clock::time_point::rep baseline_mono{};

            std::string bin();
        };

        struct ThreadNode
        {
            std::atomic<std::shared_ptr<ThreadNode>> next{ nullptr };
            std::atomic<std::shared_ptr<ThreadNode>> prev{ nullptr };
            std::atomic<unsigned char*> buffer{ nullptr };
            std::atomic<unsigned char*> ring{ nullptr };
            std::atomic<unsigned char*> data{ nullptr };
            std::atomic<std::size_t> data_head{ 0 };
            std::atomic<std::size_t> head{ 0 };
            std::atomic<std::size_t> ctr{ 0 };
        };

        class LogStream : public std::streambuf
        {
        private:
            std::span<unsigned char> _buffer{};
        public:
            LogStream(std::span<unsigned char> buffer);

            virtual int sync() override;
            virtual int overflow(int ch) override;

            std::size_t size() const noexcept;
            std::string_view buffer() const noexcept;
            void reset() noexcept;
        };
        class InputStream
        {
        private:
            struct State
            {
                std::array<unsigned char, 256> buffer{};
                LogStream stream{ buffer };
                std::ostream out{ &stream };
                std::string_view data{};
                std::string_view module{};
                code code{ std::numeric_limits<std::uint32_t>::max() };
                Severity severity{ Severity::Debug };
                bool filter{ false };

                State();
            };
            static inline thread_local State state{};
            RuntimeLogs* _ptr{ nullptr };
        public:
            InputStream(RuntimeLogs* ptr) : _ptr(ptr) {}

            template<typename Type> InputStream& operator<<(Type&& value)
            {
                if (!state.filter)
                    state.out << std::forward<Type>(value);
                return *this;
            }
            InputStream& operator<<(LogOp data);
            InputStream& operator<<(DataOp md);
            InputStream& operator<<(FlushOp);
        };

        // stacktrace_type length
        // '#XXX: '  - 6 bytes (max stacktrace depth)
        // '0x'      - 2 bytes
        // 'FF...FF' - 16 bytes (for max uint64)
        // * max stacktrace
        // total 3072 bytes

        static constexpr auto max_strace_depth = 128;
        static constexpr auto max_strace_line_width = 6 + 2 + 16 + 1;

        using stacktrace_data = std::array<void*, max_strace_depth>;
        using stacktrace_print = std::array<char, max_strace_depth * max_strace_line_width>;

        static inline std::size_t _stacktrace(stacktrace_print& out) noexcept;
        static void _signal_handler(int sig) noexcept;
        static void _hook_signal() noexcept;
    private:
        const Config _cfg{};

        std::fstream _out{};

        std::chrono::system_clock::time_point _baseline_sys{};
        std::chrono::steady_clock::time_point _baseline_mono{};

        std::vector<std::unique_ptr<RuntimeLogSink>> _sinks{};
        std::atomic<Filter> _filter{ Filter::Clear };
        std::atomic<Filter> _sink_filter{ Filter::Clear };
        std::atomic<std::shared_ptr<ThreadNode>> _root{};
    private:
        void _log(Severity severity, std::uint32_t code, std::string_view module, std::string_view data, std::string_view msg) noexcept;

        explicit RuntimeLogs(Config cfg);
        RuntimeLogs() : RuntimeLogs(Config{}) {}
    public:
        friend class context;

        InputStream in{ this };

        static ptr make(Config cfg);
        static ptr make();

        ~RuntimeLogs() { sync(); }

        void sync() noexcept;
        void filter(Filter filter) noexcept;
        void sink_filter(Filter filter) noexcept;

        template<typename Type, typename... Argv>
        Type& sink(Argv&&... args)
        {
            auto f = std::find_if(_sinks.begin(), _sinks.end(), [](const auto& v) {
                return typeid(v.get()) == typeid(Type);
            });
            if (f != _sinks.end())
            {
                *f = std::make_unique<Type>(
                    std::forward<Argv>(args)...
                );
                return static_cast<Type&>(*f->get());
            }
            else
            {
                _sinks.push_back(std::make_unique<Type>(
                    std::forward<Argv>(args)...
                ));
                return static_cast<Type&>(*_sinks.back());
            }
        }
        template<typename Type>
        void unsink()
        {
            auto f = std::find_if(_sinks.begin(), _sinks.end(), [](const auto& v) {
                return typeid(v.get()) == typeid(Type);
            });
            if (f != _sinks.end())
                _sinks.erase(f);
        }

        std::vector<LogEntry> page(std::size_t cnt);
        std::vector<LogEntry> page_epoch(std::chrono::system_clock::time_point epoch);

        template<typename... Argv>
        RuntimeLogs& logd(Severity severity, std::string_view module, std::string_view data, Argv&&... args) noexcept
        {
            if (!(unsigned(severity) & unsigned(_filter.load())))
            {
                ((in
                    << rs::log(severity, module)
                    << rs::dat(data))
                    << ...
                    << std::forward<Argv>(args)
                ) << rs::flush;
            }
            return *this;
        }
        template<typename... Argv>
        RuntimeLogs& log(Severity severity, std::string_view module, Argv&&... args) noexcept
        {
            return logd(severity, module, "", std::forward<Argv>(args)...);
        }
        template<typename... Argv>
        RuntimeLogs& dbg(std::string_view module, Argv&&... args) noexcept
        {
            return logd(Severity::Debug, module, "", std::forward<Argv>(args)...);
        }
    };

    class context
    {
    private:
        static inline thread_local RuntimeLogs::ptr _logs{ nullptr };
    public:
        static void set(RuntimeLogs::ptr ptr);
        static RuntimeLogs::ptr ptr();
        static RuntimeLogs& get();
        static RuntimeLogs::InputStream& in();
    };

#   define LOG(_severity_, _mod_, ...)           ::rs::context::get().log(::rs::Severity::_severity_, #_mod_ __VA_OPT__(,) __VA_ARGS__);
#   define LOGD(_severity_, _mod_, _data_, ...)  ::rs::context::get().log(::rs::Severity::_severity_, #_mod_, _data_ __VA_OPT__(,) __VA_ARGS__);

    class RuntimeLogSink
    {
    public:
        virtual ~RuntimeLogSink() = default;
        virtual void accept(LogEntry entry) noexcept {}
    };
    class ConsoleSink : public RuntimeLogSink
    {
    public:
        virtual void accept(LogEntry entry) noexcept override;
    };
    class ColoredConsoleSink : public RuntimeLogSink
    {
    public:
        virtual void accept(LogEntry entry) noexcept override;
    };

    namespace ex
    {
        struct Dummy {};
        struct BaseException : std::exception
        {
            std::string msg{ "" };
            std::string description{ "" };
            std::string module{ "" };
            RuntimeLogs::code code{ RuntimeLogs::invalid_code };
            std::source_location loc{};

            BaseException(const BaseException&) = default;
            BaseException(BaseException&&) = default;
            BaseException(const std::string& module, const std::string& msg, const std::string& description, std::uint32_t code, std::source_location loc) :
                module(module), msg(msg), description(description), code(code), loc(loc) {}

            virtual const char* what() const noexcept override
            {
                return msg.empty() ? description.c_str() : msg.c_str();
            }
        };

        template<typename Exception = BaseException>
        [[ noreturn ]] void thrw(
            std::string_view module,
            const std::string& msg,
            const std::string& description = "",
            std::uint32_t code = RuntimeLogs::invalid_code,
            std::source_location loc = std::source_location::current()
        )
        {
            throw Exception(std::string(module), msg, description, code, loc);
        }

        template<typename Func, typename... Argv>
        auto safe(std::string_view module, Func&& callback, Argv&&... args, std::source_location loc = std::source_location::current())
            -> std::optional<std::conditional_t<std::is_same_v<decltype(callback(args...)), void>, Dummy, decltype(callback(args...))>>
        {
            try
            {
                if constexpr (std::is_same_v<decltype(callback(args...)), void>)
                {
                    callback(std::forward<Argv>(args)...);
                    return Dummy();
                }
                else
                    return callback(std::forward<Argv>(args)...);
            }
            catch (const BaseException& ex)
            {
                context::get().log(
                    Severity::Critical,
                    ex.module.empty() ? module : ex.module,
                    "Critical Error Occured; Reason: '", ex.msg, "', '",
                    ex.description, "'",
                    ex.code == RuntimeLogs::invalid_code ? "" : std::format(" [{}]", ex.code),
                    " Caught: (", loc.file_name(), ":", loc.line(), ".", loc.column(),
                    " Thrown: (", ex.loc.file_name(), ":", ex.loc.line(), ".", ex.loc.column()
                );
            }
            catch (const std::exception& ex)
            {
                context::get().log(
                    Severity::Critical,
                    module,
                    "Critical Error Occured; Reason: '", ex.what(), '\'',
                    " (", loc.file_name(), ":", loc.line(), ".", loc.column()
                );
            }
            catch (...)
            {
                context::get().log(
                    Severity::Critical,
                    module,
                    "Unknown Error Occured",
                    " (", loc.file_name(), ":", loc.line(), ".", loc.column()
                );
            }
            return std::nullopt;
        }
    }
}

#endif // RUNTIME_LOGS_HPP
