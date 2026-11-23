#

<div align="center">

<img alt="meshtastic" src="etc/MUI.png" width="80" height="80">

  <h1 align="center"> Meshtastic Standalone UI</h1>
  <p style="font-size:20px;" align="center">A versatile UI for the <a href="https://meshtastic.org">meshtastic® project</a> </p>
</div>

<!--Project specific badges here-->

<p align="center">
<a href="">
    <img alt="GitHub last commit" src="https://img.shields.io/github/last-commit/meshtastic/standalone-ui"></a>
    <a href="https://github.com/meshtastic/standalone-ui/issues">
    <img alt="GitHub issues" src="https://img.shields.io/github/issues/meshtastic/standalone-ui"></a>
  <a href="https://opencollective.com/meshtastic">
    <img alt="Open Collective backers" src="https://img.shields.io/opencollective/backers/meshtastic?label=support%20meshtastic">
  </a>
</p>

<p align="center">
Vectors and icons by <a href="https://www.svgrepo.com/" target="_blank">SVG Repo</a><br>
Graphics using <a href="https://lvgl.io/" target="_blank">LVGL</a> library
</p>

## :wave: Introduction

### PlatformIO projects for standalone display devices

This repository provides firmware source code for the following MUI devices and scenarios:

<p style="text-align:center;">

- RockBase NM-CYD-C5 TFT (esp32-c5) - serial
- LilyGo T-HMI 2.8inch TFT (esp32-s3) - serial
- ESP32-4848S040 4.0inch TFT (esp32-s3) - serial
- Makerfabs 4.0inch MaTouch TFT (esp32-s3) - serial
- WT32-SC01 Plus 3.5inch TFT (esp32-s3) - serial
- Native linux X11 local/remote application - ethernet TCP/IP or USB serial
- Native Linux framebuffer application with keyboard and mouse input

<img src="docs/T-HMI.jpg" alt="T-HMI" width="130" height="150"><img src="docs/ESP32-4848S040.jpg" alt="ESP32-4848S040" width="150" height="150"><img src="docs/WT32-SC01-Plus.jpg" alt="WT32-SC01 Plus" width="150" height="150"><img src="docs/native-mui.png" alt="Native MUI Client" width="190" height="150">

</p>

## :computer: Build & Flash

Please follow the prerequisites for setting up the <a href="https://meshtastic.org/docs/development/linux" target="_blank">Linux Development Environment</a> and the general <a href="https://meshtastic.org/docs/development/firmware/build/" target="_blank">build instructions.</a><br>
Then either install a <a href="https://meshtastic.org/docs/hardware/devices/linux-native-hardware/" target="_blank"> Linux native device</a> or use a esp32-s3 TFT device connected to a LoRa device via serial .<br><br>
Finally build the MUI standalone firmware using a linux terminal as follows:

```bash
# checkout the repository from github
git clone --depth=1 --recursive https://github.com/meshtastic/standalone-ui
cd standalone-ui

# activate the pio environment
source $HOME/.platformio/bin/penv/activate

# build and optionally upload the target(s) to your device(s)
pio run -e t-hmi-mui -t upload
pio run -e diy-replicator-mui -t upload
pio run -e wt32-sc01-plus-mui -t upload

# when connecting to another LoRa device its serial interface must be configured:
meshtastic --set serial.enabled true --set serial.baud 38400 --set serial.mode PROTO
# please configure the right GPIO serial ports for rx/tx, e.g. for RAK4631
meshtastic --set serial.rxd 8 --set serial.txd 6

# build the native MUI
pio run -e native-mui

# install MUI desktop icon
mkdir -p $HOME/.icons $HOME/.local/share/applications $HOME/.local/bin
cp etc/MUI.desktop $HOME/.local/share/applications
cp etc/MUI.png $HOME/.icons
cp .pio/build/native-mui/program $HOME/.local/bin/native-mui

# run MUI native from CLI to connect to meshtasticd daemon
.pio/build/native-mui/program

# run MUI native from CLI to connect to LoRa device via USB
.pio/build/native-mui/program -p /dev/ttyACM0
```

### Native display and input options

The `native-mui` target uses X11. Build `native-fb-mui` to use a Linux framebuffer;
the framebuffer driver detects the panel's actual resolution and pixel format.
Set `LV_LINUX_FBDEV_DEVICE` to select the framebuffer device (default `/dev/fb0`).

Command-line options override their corresponding environment variables:

| Option                       | Environment variable      | Values / default                                                                                                                                        |
| ---------------------------- | ------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `--size WIDTHxHEIGHT`        | `MUI_SIZE`                | Default `480x480`; X11 accepts width 320-800 and height 240-800. Framebuffer accepts positive 16-bit dimensions, then uses the detected panel geometry. |
| `--keyboard DEVICE`          | `MUI_KEYBOARD`            | Linux input path or `eventX`; unset uses the backend's existing input handling.                                                                         |
| `--pointer DEVICE`           | `MUI_POINTER`             | Linux pointer input path or `eventX`; unset uses the backend's existing input handling.                                                                 |
| `--keyboard-navigation MODE` | `MUI_KEYBOARD_NAVIGATION` | For a configured Linux keyboard: `contextual` (default) applies MUI's context-aware arrow navigation; `raw` passes keys directly to LVGL.               |
| `--cursor MODE`              | `MUI_CURSOR`              | For framebuffer-discovered and configured Linux pointers: `auto` (default), `always`, or `never`.                                                       |
| `--sidebar-scrollbar MODE`   | `MUI_SIDEBAR_SCROLLBAR`   | `auto` (default), `on`, or `off`; this changes the indicator without disabling navigation scrolling.                                                    |

In contextual mode, arrows move focus in menus while retaining their normal behavior
in text editors, open dropdowns and maps. An explicit empty device argument, such as
`--pointer=`, clears a device selected by the environment.

Keyboard navigation mode applies to the Linux device selected with `--keyboard` or
`MUI_KEYBOARD`. Cursor mode applies to framebuffer-discovered inputs and the device
selected with `--pointer` or `MUI_POINTER`. X11's built-in window input is handled by
the X11 driver.

Automatic framebuffer cursors require mouse-button capability detection. With an
older LVGL version that does not expose the evdev file descriptor, `auto` retains
the legacy cursor behavior for relative pointer devices. Use `never` to suppress
cursors on such versions, or `always` for buttonless relative pointing devices.

For a keyboard-driven framebuffer display, select the input path for your keyboard:

```sh
pio run -e native-fb-mui
MUI_SERVER=127.0.0.1 \
MUI_KEYBOARD=/dev/input/event0 \
MUI_KEYBOARD_NAVIGATION=contextual \
MUI_CURSOR=never \
MUI_SIDEBAR_SCROLLBAR=off \
LV_LINUX_FBDEV_DEVICE=/dev/fb0 \
.pio/build/native-fb-mui/program
```

Invalid sizes or mode values stop startup with an error. The native option parser
can be tested after installing the `native-fb-mui` dependencies:

```sh
sh tests/run-native-options.sh
```

## Stats

![Alt](https://repobeats.axiom.co/api/embed/7caf5492ebef98d9b88747eba781e592a9682f9d.svg "Repobeats analytics image")
