#pragma once

#include "graphics/driver/DisplayDriverConfig.h"
#include <optional>
#include <string>

namespace native_ui
{

struct OptionValues {
    std::optional<std::string> size;
    std::optional<std::string> keyboard;
    std::optional<std::string> pointer;
    std::optional<std::string> keyboardNavigation;
    std::optional<std::string> cursor;
    std::optional<std::string> sidebarScrollbar;
};

struct DisplayOptions {
    uint16_t width = 480;
    uint16_t height = 480;
    DisplayDriverConfig::input_config_t input;
    DisplayDriverConfig::navigation_config_t navigation;
};

bool resolveDisplayOptions(const OptionValues &commandLine, const OptionValues &environment, bool framebuffer,
                           DisplayOptions &result, std::string &error);

} // namespace native_ui
