#include "NativeOptions.h"
#include <cassert>
#include <iostream>

using native_ui::DisplayOptions;
using native_ui::OptionValues;

static DisplayOptions resolve(const OptionValues &commandLine = {}, const OptionValues &environment = {}, bool framebuffer = true)
{
    DisplayOptions result;
    std::string error = "previous error";
    assert(native_ui::resolveDisplayOptions(commandLine, environment, framebuffer, result, error));
    assert(error.empty());
    return result;
}

static void expectInvalid(const OptionValues &commandLine, bool framebuffer = true)
{
    DisplayOptions result;
    result.width = 42;
    result.height = 43;
    std::string error;
    assert(!native_ui::resolveDisplayOptions(commandLine, {}, framebuffer, result, error));
    assert(!error.empty());
    assert(result.width == 42 && result.height == 43);
}

int main()
{
    const auto defaults = resolve();
    assert(defaults.width == 480 && defaults.height == 480);
    assert(defaults.input.keyboardDevice.empty() && defaults.input.pointerDevice.empty());
    assert(defaults.input.keyboardNavigation);
    assert(defaults.input.cursorMode == DisplayDriverConfig::cursor_mode_t::AUTO);
    assert(defaults.navigation.scrollbarMode == DisplayDriverConfig::scrollbar_mode_t::AUTO);

    OptionValues values;
    for (const auto &size : {"320x170", "256x128", "1x1", "65535x65535"}) {
        values.size = size;
        resolve(values);
    }
    for (const auto &size : {"", "320", "x170", "320x", "320x170junk", "320x170x1", "320X170", " 320x170", "320x170 ", "+320x170",
                             "-320x170", "0x170", "320x0", "65536x170", "320x65536", "99999999999999999999999x170"}) {
        values.size = size;
        expectInvalid(values);
    }
    values.size = "320x240";
    resolve(values, {}, false);
    values.size = "800x800";
    resolve(values, {}, false);
    for (const auto &size : {"319x240", "320x239", "801x480", "480x801"}) {
        values.size = size;
        expectInvalid(values, false);
    }

    OptionValues environment;
    environment.size = "320x170";
    environment.keyboard = "/dev/input/event3";
    environment.pointer = "/dev/input/event4";
    environment.keyboardNavigation = "raw";
    environment.cursor = "never";
    environment.sidebarScrollbar = "off";
    const auto fromEnvironment = resolve({}, environment);
    assert(fromEnvironment.width == 320 && fromEnvironment.height == 170);
    assert(fromEnvironment.input.keyboardDevice == "/dev/input/event3");
    assert(fromEnvironment.input.pointerDevice == "/dev/input/event4");
    assert(!fromEnvironment.input.keyboardNavigation);
    assert(fromEnvironment.input.cursorMode == DisplayDriverConfig::cursor_mode_t::NEVER);
    assert(fromEnvironment.navigation.scrollbarMode == DisplayDriverConfig::scrollbar_mode_t::OFF);

    OptionValues commandLine;
    commandLine.size = "640x480";
    commandLine.keyboard = "/dev/input/by-path/keyboard";
    commandLine.pointer = "";
    commandLine.keyboardNavigation = "contextual";
    commandLine.cursor = "always";
    commandLine.sidebarScrollbar = "on";
    const auto overridden = resolve(commandLine, environment);
    assert(overridden.width == 640 && overridden.height == 480);
    assert(overridden.input.keyboardDevice == "/dev/input/by-path/keyboard");
    assert(overridden.input.pointerDevice.empty());
    assert(overridden.input.keyboardNavigation);
    assert(overridden.input.cursorMode == DisplayDriverConfig::cursor_mode_t::ALWAYS);
    assert(overridden.navigation.scrollbarMode == DisplayDriverConfig::scrollbar_mode_t::ON);

    commandLine.cursor = "auto";
    commandLine.sidebarScrollbar = "auto";
    const auto automatic = resolve(commandLine, environment);
    assert(automatic.input.cursorMode == DisplayDriverConfig::cursor_mode_t::AUTO);
    assert(automatic.navigation.scrollbarMode == DisplayDriverConfig::scrollbar_mode_t::AUTO);

    environment.size = "invalid";
    environment.keyboardNavigation = "invalid";
    environment.cursor = "invalid";
    environment.sidebarScrollbar = "invalid";
    resolve(commandLine, environment);
    for (const auto &invalid : {"", "off", "RAW", "unknown"}) {
        OptionValues invalidOptions;
        invalidOptions.keyboardNavigation = invalid;
        expectInvalid(invalidOptions);
    }
    for (const auto &invalid : {"", "on", "AUTO", "unknown"}) {
        OptionValues invalidOptions;
        invalidOptions.cursor = invalid;
        expectInvalid(invalidOptions);
    }
    for (const auto &invalid : {"", "always", "OFF", "unknown"}) {
        OptionValues invalidOptions;
        invalidOptions.sidebarScrollbar = invalid;
        expectInvalid(invalidOptions);
    }

    std::cout << "PASS: native geometry, input options, invalid values, and CLI/environment precedence\n";
}
