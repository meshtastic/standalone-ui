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

- LilyGo T-HMI 2.8inch TFT (esp32-s3) - serial
- ESP32-4848S040 4.0inch TFT (esp32-s3) - serial
- WT32-SC01 Plus 3.5inch TFT (esp32-s3) - serial
- Native linux X11 local/remote application - ethernet TCP/IP

<img src="docs/T-HMI.jpg" alt="T-HMI" width="130" height="150"><img src="docs/ESP32-4848S040.jpg" alt="ESP32-4848S040" width="150" height="150"><img src="docs/WT32-SC01-Plus.jpg" alt="WT32-SC01 Plus" width="150" height="150"><img src="docs/native-mui.png" alt="Native MUI Client" width="190" height="150">

</p>

## :computer: Build & Flash

Please follow the prerequisites for setting up the <a href="https://meshtastic.org/docs/development/linux" target="_blank">Linux Development Environment</a> and the general <a href="https://meshtastic.org/docs/development/firmware/build/" target="_blank">build instructions.</a><br>

For debian based systems : 
```
sudo apt install libgpiod-dev libyaml-cpp-dev libbluetooth-dev libusb-1.0-0-dev libi2c-dev libuv1-dev
```

For fedora based systems : 
```
sudo dnf install gcc-c++ pkgconfig(yaml-cpp) pkgconfig(libgpiod) pkgconfig(bluez) pkgconfig(libusb-1.0) libi2c-devel pkgconfig(libuv)
```

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
```

## Stats

![Alt](https://repobeats.axiom.co/api/embed/7caf5492ebef98d9b88747eba781e592a9682f9d.svg "Repobeats analytics image")
