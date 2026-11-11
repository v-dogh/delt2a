#pragma once

#include <stdexcept>
#include <string>

#define CHECK(expr) do { if (!(expr)) throw std::runtime_error( \
    std::string(__FILE__) + ":" + std::to_string(__LINE__) + ": " #expr); } while (false)

template<typename Exception, typename Func> void throws(Func&& callback)
{
    try { callback(); }
    catch (const Exception&) { return; }
    throw std::runtime_error("Expected exception was not thrown");
}
