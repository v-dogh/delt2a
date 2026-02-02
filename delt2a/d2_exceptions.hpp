#ifndef D2_EXCEPTIONS_HPP
#define D2_EXCEPTIONS_HPP

#include <exception>
#include <format>
#include <string>
#include "logs/runtime_logs.hpp"

namespace d2
{
#   ifdef D2_SECURITY_ASSERT
#       define D2_EXPECT(...) (((__VA_ARGS__) == false) ? \
            [&]() { \
                D2_LOG(Warning, assert, "Expectation failed: '", #__VA_ARGS__, "'") \
            }() : \
            void());
#       define D2_ASSERT(...) (((__VA_ARGS__) == false) ? \
            [&]() { \
                D2_LOG(Warning, assert, "Assertion failed: '", #__VA_ARGS__, "'") \
                D2_LOG(Debug, assert, std::source_location::current().file_name(), ":", std::source_location::current().line()) \
                ::rs::ex::thrw("assert", "Assertion failed", #__VA_ARGS__); \
            }() : \
        void());
#   else
#       define D2_EXPECT(...) (void());
#       define D2_ASSERT(...) (void());
#   endif
#   ifdef D2_SECURITY_LOG
#       define D2_LOG(_severity_, _mod_, ...) LOG(_severity_, _mod_, __VA_ARGS__)
#       define D2_TLOG(_severity_, ...) ::rs::context::get().log(::rs::Severity::_severity_, __module__ __VA_OPT__(,) __VA_ARGS__);
#   else
#       define D2_LOG(...) (void());
#       define D2_TLOG(...) (void());
#   endif
#   define D2_THRW(_msg_) ::rs::ex::thrw(__module__, _msg_, "")
#   define D2_THRW_EX(_msg_, _description_) ::rs::ex::thrw(__module__, _msg_, _description_)
#   define D2_TAG_MODULE(_name_) private: static inline constexpr auto __module__ = std::string_view(#_name_);
#   define D2_TAG_MODULE_RUNTIME(_name_) const auto __module__ = _name_;
#   define D2_SAFE_BLOCK_BEGIN ::rs::ex::safe(__module__, [&]() {
#   define D2_SAFE_BLOCK_END });
} // d2

#endif // D2_EXCEPTIONS_HPP
