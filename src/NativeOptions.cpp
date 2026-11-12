#ifdef ARCH_PORTDUINO

#include "NativeOptions.h"
#include <charconv>
#include <limits>
#include <string_view>
#include <utility>

namespace native_ui
{

static const std::optional<std::string> &selected(const std::optional<std::string> &commandLine,
                                                  const std::optional<std::string> &environment)
{
    return commandLine ? commandLine : environment;
}

static bool parseDimension(std::string_view text, uint16_t &result)
{
    unsigned int value = 0;
    const auto parsed = std::from_chars(text.data(), text.data() + text.size(), value);
    if (parsed.ec != std::errc{} || parsed.ptr != text.data() + text.size() || value == 0 ||
        value > std::numeric_limits<uint16_t>::max())
        return false;
    result = static_cast<uint16_t>(value);
    return true;
}

static bool parseSize(const std::string &text, bool framebuffer, uint16_t &width, uint16_t &height)
{
    const auto separator = text.find('x');
    if (separator == std::string::npos || !parseDimension(std::string_view(text).substr(0, separator), width) ||
        !parseDimension(std::string_view(text).substr(separator + 1), height))
        return false;
    return framebuffer || (width >= 320 && width <= 800 && height >= 240 && height <= 800);
}

bool resolveDisplayOptions(const OptionValues &commandLine, const OptionValues &environment, bool framebuffer,
                           DisplayOptions &result, std::string &error)
{
    DisplayOptions options;
    if (const auto &value = selected(commandLine.size, environment.size);
        value && !parseSize(*value, framebuffer, options.width, options.height)) {
        error = framebuffer ? "size must be WIDTHxHEIGHT with dimensions from 1 to 65535"
                            : "size must be WIDTHxHEIGHT with width 320-800 and height 240-800";
        return false;
    }
    if (const auto &value = selected(commandLine.keyboard, environment.keyboard))
        options.input.keyboardDevice = *value;
    if (const auto &value = selected(commandLine.pointer, environment.pointer))
        options.input.pointerDevice = *value;

    if (const auto &value = selected(commandLine.keyboardNavigation, environment.keyboardNavigation)) {
        if (*value == "contextual")
            options.input.keyboardNavigation = true;
        else if (*value == "raw")
            options.input.keyboardNavigation = false;
        else {
            error = "keyboard-navigation must be contextual or raw";
            return false;
        }
    }

    if (const auto &value = selected(commandLine.cursor, environment.cursor)) {
        if (*value == "auto")
            options.input.cursorMode = DisplayDriverConfig::cursor_mode_t::AUTO;
        else if (*value == "always")
            options.input.cursorMode = DisplayDriverConfig::cursor_mode_t::ALWAYS;
        else if (*value == "never")
            options.input.cursorMode = DisplayDriverConfig::cursor_mode_t::NEVER;
        else {
            error = "cursor must be auto, always, or never";
            return false;
        }
    }

    if (const auto &value = selected(commandLine.sidebarScrollbar, environment.sidebarScrollbar)) {
        if (*value == "auto")
            options.navigation.scrollbarMode = DisplayDriverConfig::scrollbar_mode_t::AUTO;
        else if (*value == "on")
            options.navigation.scrollbarMode = DisplayDriverConfig::scrollbar_mode_t::ON;
        else if (*value == "off")
            options.navigation.scrollbarMode = DisplayDriverConfig::scrollbar_mode_t::OFF;
        else {
            error = "sidebar-scrollbar must be auto, on, or off";
            return false;
        }
    }

    result = std::move(options);
    error.clear();
    return true;
}

} // namespace native_ui

#endif
