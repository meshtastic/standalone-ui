#!/bin/sh
set -eu

repo_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
device_ui=${1:-"$repo_dir/.pio/libdeps/native-fb-mui/meshtastic-device-ui"}
test_dir=$(mktemp -d)
trap 'rm -rf "$test_dir"' EXIT HUP INT TERM

"${CXX:-c++}" -std=c++17 -Wall -Wextra -Werror -DARCH_PORTDUINO \
    -I"$repo_dir/src" -I"$device_ui/include" \
    "$repo_dir/tests/native-options.cpp" "$repo_dir/src/NativeOptions.cpp" \
    -o "$test_dir/native-options"
"$test_dir/native-options"
