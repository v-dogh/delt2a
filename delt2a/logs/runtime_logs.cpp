#ifndef RUNTIME_LOGS_CPP
#define RUNTIME_LOGS_CPP

#include "runtime_logs.hpp"
#include <cmath>
#include <format>
#include <cstring>
#include <charconv>
#include <iostream>
#include <signal.h>
#include <shared_mutex>
#include <absl/debugging/stacktrace.h>

namespace rs
{
    RuntimeLogs::InputStream::State::State() {}
    RuntimeLogs::InputStream& RuntimeLogs::InputStream::operator<<(LogOp data)
    {
        state.module = data.value;
        state.severity = data.severity;
        if (unsigned(state.severity) & unsigned(_ptr->_filter.load()))
            state.filter = true;
        return *this;
    }
    RuntimeLogs::InputStream& RuntimeLogs::InputStream::operator<<(DataOp md)
    {
        state.data = md.value;
        return *this;
    }
    RuntimeLogs::InputStream& RuntimeLogs::InputStream::operator<<(FlushOp)
    {
        _ptr->_log(state.severity, state.code, state.module, state.data, state.stream.buffer());
        state.stream.reset();
        state.severity = Severity::Debug;
        state.data = "";
        state.filter = false;
        state.code = std::numeric_limits<std::uint32_t>::max();
        return *this;
    }

    std::string_view signal_to_str(int sig) noexcept
    {
        switch (sig)
        {
        case SIGSEGV: return "SIGSEGV (Segmentation Fault)";
        case SIGABRT: return "SIGABRT (Abort)";
        case SIGFPE: return "SIGFPE (Floating Point Exception)";
        case SIGILL: return "SIGILL (Illegal Instruction)";
        case SIGBUS: return "SIGBUS (Bus Error)";
        case SIGTERM: return "SIGTERM (Termination Request)";
        case SIGINT: return "SIGINT (Exit)";
        case SIGQUIT: return "SIGQUIT (Quit)";
        default: return "Unknown signal";
        }
    }
    std::string_view severity_to_str(Severity severity) noexcept
    {
        switch (severity)
        {
        case Severity::Info: return "Info";
        case Severity::Verbose: return "Verbose";
        case Severity::Warning: return "Warning";
        case Severity::Error: return "Error";
        case Severity::Critical: return "Critical";
        case Severity::Module: return "Module";
        case Severity::Reserved: return "Reserved";
        case Severity::Debug: return "Debug";
        default: return "Unknown";
        };
    }

    RuntimeLogs::LogStream::LogStream(std::span<unsigned char> buffer) : _buffer(buffer)
    {
        setp(
            reinterpret_cast<char*>(_buffer.data()),
            reinterpret_cast<char*>(_buffer.data()) + _buffer.size()
        );
    }

    int RuntimeLogs::LogStream::sync()
    {
        return 0;
    }
    int RuntimeLogs::LogStream::overflow(int ch)
    {
        if (ch == EOF || pptr() == epptr())
            return EOF;
        *pptr() = static_cast<char>(ch);
        pbump(1);
        return ch;
    }

    std::size_t RuntimeLogs::LogStream::size() const noexcept
    {
        return pptr() - pbase();
    }
    std::string_view RuntimeLogs::LogStream::buffer() const noexcept
    {
        return std::string_view(
            reinterpret_cast<const char*>(_buffer.data()),
            size()
        );
    }
    void RuntimeLogs::LogStream::reset() noexcept
    {
        setp(pbase(), epptr());
    }

    std::shared_mutex global_logs_mtx{};
    std::vector<std::shared_ptr<RuntimeLogs>> global_logs{};
    std::array<void(*)(int), NSIG> global_logs_prev_handlers{};
    std::atomic<bool> hooked{ false };

    RuntimeLogs::ptr RuntimeLogs::make(Config cfg)
    {
        auto ptr = std::shared_ptr<RuntimeLogs>(new RuntimeLogs{ std::move(cfg) });
        if (ptr->_cfg.handle_signals)
        {
            std::lock_guard lock(global_logs_mtx);
            global_logs.push_back(ptr);
            _hook_signal();
        }
        return ptr;
    }
    RuntimeLogs::ptr RuntimeLogs::make()
    {
        return make(Config());
    }

    std::string RuntimeLogs::DumpHeader::bin()
    {
        std::string out;
        out.resize(
            sizeof(endian) +
            sizeof(baseline_system) +
            sizeof(baseline_mono)
        );
        std::size_t off = 0;
        _write(out.data(), endian, off);
        _write(out.data(), baseline_system, off);
        _write(out.data(), baseline_mono, off);
        return out;
    }
    void RuntimeLogs::LogHeader::bin(unsigned char* buffer)
    {
        std::size_t off = 0;
        _write(buffer, timestamp, off);
        _write(buffer, code, off);
        _write(buffer, length, off);
        _write(buffer, md_length, off);
        _write(buffer, severity, off);
    }
    std::size_t RuntimeLogs::LogHeader::size()
    {
        return
            sizeof(timestamp) +
            sizeof(code) +
            sizeof(length) +
            sizeof(md_length) +
            sizeof(severity);
    }

    RuntimeLogs::RuntimeLogs(Config cfg)
        : _cfg(std::move(cfg))
    {
        if (!_cfg.root.empty())
        {
            std::filesystem::create_directories(_cfg.root);

            std::filesystem::remove(_cfg.root/"logs.dat");
            std::filesystem::remove(_cfg.root/"data.dat");
            std::filesystem::remove(_cfg.root/"dump.log");

            std::ofstream(_cfg.root/"data.dat");

            _baseline_sys = std::chrono::system_clock::now();
            _baseline_mono = std::chrono::steady_clock::now();

            if (_cfg.min_sync_count != std::numeric_limits<std::size_t>::max())
            {
                _out.open(_cfg.root/"logs.dat",
                    std::ios::out |
                    std::ios::app |
                    std::ios::trunc |
                    std::ios::binary
                );
                _out << DumpHeader{
                    .baseline_system = _baseline_sys.time_since_epoch().count(),
                    .baseline_mono = _baseline_mono.time_since_epoch().count()
                }.bin();
            }
        }

        _root = std::make_shared<ThreadNode>();
        _root.load()->next = _root.load();
        _root.load()->prev = _root.load();
    }

    inline std::size_t RuntimeLogs::_stacktrace(stacktrace_print& out) noexcept
    {
        stacktrace_data data{};
        absl::GetStackTrace(data.data(), max_strace_depth, 0);

        std::size_t off = 0;
        for (std::size_t i = 0; i < data.size() && data[i] != 0x00; i++)
        {
            out[off++] = '#';
            {
                const auto len = std::size_t(i == 0 ? 0.0 : std::log10(i)) + 1;
                const auto beg = out.data() + off + (3 - len);
                const auto end = beg + len;
                std::to_chars(beg, end, i);
                std::memset(out.data() + off, '0', 3 - len);
                off += 3;
            }
            out[off++] = ':';
            out[off++] = ' ';
            out[off++] = '0';
            out[off++] = 'x';
            {
                const auto max_len = 16;
                const auto val = std::uintptr_t(data[i]);
                const auto len = static_cast<std::size_t>(std::log10(val) + 1);
                const auto set = max_len - len;
                const auto beg = out.begin() + off + set;
                const auto end = beg + len;
                std::to_chars(beg, end, val);
                std::memset(out.data() + off, '0', set);
                off += 16;
            }
            out[off++] = '\n';
        }

        return off;
    }
    void RuntimeLogs::_signal_handler(int sig) noexcept
    {
        std::lock_guard lock(global_logs_mtx);

        stacktrace_print stacktrace;
        const auto signal = signal_to_str(sig);
        const auto strace = std::string_view(
            stacktrace.begin(),
            stacktrace.begin() + _stacktrace(stacktrace)
        );

        for (decltype(auto) it : global_logs)
        {
            if (!it->_cfg.root.empty())
            {
                const auto path = it->_cfg.root/"dump.txt";
                const auto file = std::fopen(path.c_str(), "w");
                if (file != nullptr)
                {
                    std::fwrite(signal.data(), 1, signal.size(), file);
                    std::fwrite("\n", 1, 1, file);
                    std::fwrite(strace.data(), 1, strace.size(), file);
                    std::fclose(file);
                }
            }
        }
        for (decltype(auto) it : global_logs)
            it->sync();

        global_logs_prev_handlers[sig](sig);
    }
    void RuntimeLogs::_hook_signal() noexcept
    {
        if (!hooked.load(std::memory_order::acquire))
        {
            global_logs_prev_handlers[SIGSEGV] = signal(SIGSEGV, _signal_handler);
            global_logs_prev_handlers[SIGFPE] = signal(SIGFPE, _signal_handler);
            global_logs_prev_handlers[SIGILL] = signal(SIGILL, _signal_handler);
            global_logs_prev_handlers[SIGBUS] = signal(SIGBUS, _signal_handler);
            global_logs_prev_handlers[SIGTERM] = signal(SIGTERM, _signal_handler);
            global_logs_prev_handlers[SIGABRT] = signal(SIGABRT, _signal_handler);
            global_logs_prev_handlers[SIGQUIT] = signal(SIGQUIT, _signal_handler);
            hooked.store(true, std::memory_order::release);
        }
    }

    void RuntimeLogs::_log(Severity severity, std::uint32_t code, std::string_view module, std::string_view data, std::string_view msg) noexcept
    {
        thread_local std::shared_ptr<ThreadNode> state = [this]() {
            auto ptr = std::make_shared<ThreadNode>();
            auto ex = _root.load()->prev.exchange(ptr);
            ptr->prev = ex;
            ex->next = ptr;
            state->ring = new unsigned char[_cfg.max_log_memory + _cfg.max_log_data_memory];
            state->data = state->ring.load();
            state->data = state->ring + _cfg.max_log_memory;
            return ptr;
        }();

        state->ctr.fetch_add(1);

        const auto timestamp = std::chrono::steady_clock::now();
        const auto delta = _baseline_mono - timestamp;
        const auto timestamp_sys = _baseline_sys + delta;
        const auto base = LogHeader::size() + module.size() + msg.size() + sizeof(std::uint16_t);
        const auto total = base + (!data.empty() ? LogHeader::size() + sizeof(std::uint16_t) : 0);
        if (state->head + total > _cfg.max_log_memory)
            state->head = 0;

        // Message
        {
            const auto size = LogHeader::size() + module.size() + msg.size();
            LogHeader{
                .timestamp = timestamp,
                .code = code,
                .length = static_cast<std::uint16_t>(msg.size()),
                .md_length = static_cast<std::uint8_t>(module.size()),
                .severity = severity
            }.bin(state->ring + state->head);
            std::copy(module.begin(), module.end(), state->ring + LogHeader::size());
            std::copy(msg.begin(), msg.end(), state->ring + LogHeader::size() + module.size());
            _write(
                state->ring + state->head + size,
                std::uint16_t(size)
            );
        }
        // Data
        {
            const auto size = LogHeader::size() + module.size() + msg.size();
            LogHeader{
                .data_offset = state->data_head,
                .severity = Severity::Reserved
            }.bin(state->ring + state->head + base);
            _write(
                state->ring + state->head + size + base,
                std::uint16_t(size)
            );
            state->data_head += data.size();
        }
        state->head += total;

        if (!(unsigned(severity) & unsigned(_sink_filter.load())))
        {
            const auto entry = LogEntry{
                .severity = severity,
                .timestamp = timestamp_sys,
                .code = code,
                .module = module,
                .msg = msg,
                .data = data
            };
            for (decltype(auto) it : _sinks)
                it->accept(entry);
        }
    }

    std::vector<LogEntry> RuntimeLogs::page(std::size_t cnt)
    {
        return {};
    }
    std::vector<LogEntry> RuntimeLogs::page_epoch(std::chrono::system_clock::time_point epoch)
    {
        return {};
    }

    void RuntimeLogs::sync() noexcept
    {

    }
    void RuntimeLogs::filter(Filter filter) noexcept
    {
        _filter = filter;
    }
    void RuntimeLogs::sink_filter(Filter filter) noexcept
    {
        _sink_filter = filter;
    }

    void context::set(RuntimeLogs::ptr ptr)
    {
        _logs = ptr;
    }
    RuntimeLogs::ptr context::ptr()
    {
        return _logs;
    }
    RuntimeLogs& context::get()
    {
        return *_logs;
    }
    RuntimeLogs::InputStream& context::in()
    {
        return _logs->in;
    }

    void ConsoleSink::accept(LogEntry entry) noexcept
    {
        // std::clog << std::format(
        //     "[{0:%D} - {0:%H}:{0:%M}:{0:%S}]({1:})<{2:}> : {3:}\n",
        //     std::chrono::time_point_cast<std::chrono::seconds>(entry.timestamp),
        //     severity_to_str(entry.severity),
        //     entry.module,
        //     entry.msg
        // );
    }
    void ColoredConsoleSink::accept(LogEntry entry) noexcept
    {
        constexpr std::string_view reset = "\033[0m";
        constexpr std::string_view slate_blue = "\033[38;2;75;0;130m";
        constexpr std::string_view silver = "\033[38;2;192;192;192m";
        constexpr std::string_view orange = "\033[38;2;255;165;0m";
        constexpr std::string_view yellow = "\033[93m";
        constexpr std::string_view olive = "\033[38;2;128;128;0m";
        constexpr std::string_view red = "\033[91m";
        constexpr std::string_view cyan = "\033[38;2;0;159;159m";
        constexpr std::array<std::string_view, std::countr_zero(static_cast<unsigned char>(Severity::Critical))> table{
            silver,
            silver,
            orange,
            cyan,
            yellow,
            olive,
            red
        };

        // [{3:%D} - {3:%H}:{3:%M}:{3:%S}]
        const auto color = table[std::countr_zero(static_cast<unsigned char>(entry.severity)) - 1];
        std::clog << std::format(
            "{1:}{3:}{0:}({2:}{4:}{0:})<{2:}{5:}{0:}> : {2:}{6:}{0:}\n",
            reset,
            slate_blue,
            color,
            "", // std::chrono::time_point_cast<std::chrono::seconds>(entry.timestamp),
            severity_to_str(entry.severity),
            entry.module,
            entry.msg
        );
    }
}

#endif // RUNTIME_LOGS_CPP
