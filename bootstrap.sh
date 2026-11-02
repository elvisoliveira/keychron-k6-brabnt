#!/bin/sh
# From zero to a compiled 'brabnt' firmware for the Keychron K6 RGB (ANSI).
# Clones upstream SonixQMK (pinned), patches the iton_bt driver, links this
# repo's keyboard + keymap into the tree, builds the SonixFlasherC flasher and
# compiles. Idempotent: safe to re-run. .qmk/ and .tools/ are gitignored.
set -eu

DIR="$(cd "$(dirname "$0")" && pwd)"
QMK="$DIR/.qmk"
TOOLS="$DIR/.tools"
KB="keychron/k6/rgb/v2/ansi"
KM="brabnt"

# Upstream SonixQMK, pinned so the build is reproducible. The K6 itself is not
# upstream: its definition lives in keyboard/ (linked in as keyboards/keychron/k6).
QMK_URL="https://github.com/SonixQMK/qmk_firmware.git"
QMK_BRANCH="sn32_develop"
QMK_PIN="62973ee48a976c6f7cc6d4f7b198bd48a2fdfe60"

for t in git qmk make arm-none-eabi-gcc; do
    command -v "$t" >/dev/null || { echo "missing prerequisite: $t"; exit 1; }
done

# 1) QMK tree, pinned.
if [ ! -d "$QMK/.git" ]; then
    echo ">> cloning $QMK_URL ($QMK_BRANCH)"
    git clone --depth 1 --branch "$QMK_BRANCH" "$QMK_URL" "$QMK"
    ( cd "$QMK" && git fetch --depth 1 origin "$QMK_PIN" && git checkout -q "$QMK_PIN" )
fi
echo ">> init submodules"
( cd "$QMK" && git submodule update --init --depth 1 lib/chibios lib/chibios-contrib lib/printf )

# 2) Patches (see README): iton_bt_deinit + SN32 SPI slave TX fix. Skipped if applied.
for p in "$DIR"/patches/*.patch; do
    if ( cd "$QMK" && git apply --check "$p" 2>/dev/null ); then
        echo ">> applying $(basename "$p")"
        ( cd "$QMK" && git apply "$p" )
    fi
done

# 3) SonixFlasherC (needs hidapi + libusb dev headers).
if [ ! -x "$TOOLS/SonixFlasherC/sonixflasher" ]; then
    echo ">> building SonixFlasherC"
    mkdir -p "$TOOLS"
    [ -d "$TOOLS/SonixFlasherC/.git" ] || \
        git clone --depth 1 https://github.com/SonixQMK/SonixFlasherC.git "$TOOLS/SonixFlasherC"
    ( cd "$TOOLS/SonixFlasherC" && make )
fi

# 4) Link our keyboard (its keymaps/brabnt points at keymap/) and build.
ln -sfn "$DIR/keyboard" "$QMK/keyboards/keychron/k6"
echo ">> building $KB:$KM"
( cd "$QMK" && make "$KB:$KM" )

echo
echo "built: $QMK/keychron_k6_rgb_v2_ansi_brabnt.bin"
echo "flash: enter bootloader (Fn1+Space with USB-C plugged), then ./flash.sh"
