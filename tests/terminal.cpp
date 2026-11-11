#include "check.hpp"
#include <core/platform/d2_text_layout.hpp>
#include <os/io/unix/d2_unix_system_io.hpp>
#include <cstdio>
#include <system_error>
#include <unistd.h>

using namespace d2;

std::string capture(sys::UnixTerminalOutput& output, PixelBuffer& buffer)
{
    auto file = std::tmpfile();
    CHECK(file);
    const auto saved = dup(STDOUT_FILENO);
    CHECK(saved >= 0 && dup2(fileno(file), STDOUT_FILENO) >= 0);
    try { output.write(buffer.data(), buffer.width(), buffer.height()); }
    catch (...)
    {
        dup2(saved, STDOUT_FILENO);
        close(saved);
        std::fclose(file);
        throw;
    }
    CHECK(dup2(saved, STDOUT_FILENO) >= 0);
    close(saved);
    std::rewind(file);
    std::string result;
    std::array<char, 1024> data;
    while (auto count = std::fread(data.data(), 1, data.size(), file))
        result.append(data.data(), count);
    std::fclose(file);
    CHECK(result.find('\0') == std::string::npos);
    return result;
}

int main()
{
    auto output = sys::SystemModule::make<sys::UnixTerminalOutput>(std::shared_ptr<IOContext>{});
    PixelBuffer buffer(4, 2);
    text::Layout("ab\ncd").draw(PixelBuffer::View(&buffer, 0, 0, 4, 2), pixel{});
    auto initial = capture(*output, buffer);
    CHECK(initial.find("ab  ") != std::string::npos);
    CHECK(initial.find("\x1b[2;1H") != std::string::npos);
    CHECK(capture(*output, buffer).empty());
    buffer.at(0, 0) = pixel{.r=255, .g=255, .b=255, .v='Z'};
    auto full = open("/dev/full", O_WRONLY);
    CHECK(full >= 0);
    auto saved = dup(STDOUT_FILENO);
    CHECK(saved >= 0 && dup2(full, STDOUT_FILENO) >= 0);
    bool failed = false;
    try { output->write(buffer.data(), buffer.width(), buffer.height()); }
    catch (const std::system_error&) { failed = true; }
    CHECK(dup2(saved, STDOUT_FILENO) >= 0);
    close(saved);
    close(full);
    CHECK(failed);
    CHECK(capture(*output, buffer).find("48;2;255;255;255") != std::string::npos);
    buffer.at(0, 0).r = 20;
    auto background_only = capture(*output, buffer);
    CHECK(background_only.find("48;2;20;255;255") != std::string::npos);
    buffer.set_size(2, 4);
    CHECK(!capture(*output, buffer).empty());
    CHECK(capture(*output, buffer).empty());
    PixelBuffer malformed_image(1, 1);
    malformed_image.at(0, 0) = pixel{.v = 127};
    CHECK(capture(*output, malformed_image).find("\x1b_G") == std::string::npos);
#if D2_LOCALE_MODE == UNICODE
    buffer.set_size(4, 1);
    text::Layout("界x").draw(PixelBuffer::View(&buffer, 0, 0, 4, 1), pixel{});
    auto wide = capture(*output, buffer);
    CHECK(wide.find("界x ") != std::string::npos);
    CHECK(capture(*output, buffer).empty());
    buffer.at(1, 0) = pixel{.v = 'q'};
    auto cut = capture(*output, buffer);
    CHECK(cut.find("界") == std::string::npos);
    CHECK(cut.find(" qx") != std::string::npos);
    buffer.fill(pixel{});
    text::Layout("界").draw(PixelBuffer::View(&buffer, 0, 0, 4, 1), pixel{});
    CHECK(capture(*output, buffer).find("界") != std::string::npos);
    buffer.fill(pixel{});
    auto erased = capture(*output, buffer);
    CHECK(erased.find("  ") != std::string::npos);
#endif
}
