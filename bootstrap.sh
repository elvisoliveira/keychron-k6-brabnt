#!/bin/sh
# From zero to a compiled 'brabnt' firmware for the Keychron K6 RGB (ANSI).
# Fetches the toolchain into .tools/, clones upstream SonixQMK (pinned), patches
# it, links this repo's keyboard + keymap into the tree, builds the SonixFlasherC
# flasher and compiles. Idempotent: safe to re-run. .qmk/ and .tools/ are gitignored.
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

# Toolchain, kept in .tools/ (nothing installed system-wide): the qmk CLI in a
# venv and the official Arm GNU Toolchain tarball (gcc + binutils + newlib).
ARM_NAME="arm-gnu-toolchain-14.2.rel1-x86_64-arm-none-eabi"
ARM_URL="https://developer.arm.com/-/media/Files/downloads/gnu/14.2.rel1/binrel/$ARM_NAME.tar.xz"
ARM_SHA256="62a63b981fe391a9cbad7ef51b17e49aeaa3e7b0d029b36ca1e9c3b2a9b78823"

for t in git make python3 curl; do
    command -v "$t" >/dev/null || { echo "missing prerequisite: $t"; exit 1; }
done

mkdir -p "$TOOLS"
if [ ! -x "$TOOLS/venv/bin/qmk" ]; then
    echo ">> installing qmk CLI into $TOOLS/venv"
    python3 -m venv "$TOOLS/venv"
    "$TOOLS/venv/bin/pip" install -q qmk
fi
if [ ! -x "$TOOLS/$ARM_NAME/bin/arm-none-eabi-gcc" ]; then
    echo ">> downloading $ARM_NAME"
    curl -fL -o "$TOOLS/$ARM_NAME.tar.xz" "$ARM_URL"
    echo "$ARM_SHA256  $TOOLS/$ARM_NAME.tar.xz" | sha256sum -c -
    tar -xJf "$TOOLS/$ARM_NAME.tar.xz" -C "$TOOLS"
    rm "$TOOLS/$ARM_NAME.tar.xz"
fi
PATH="$TOOLS/venv/bin:$TOOLS/$ARM_NAME/bin:$PATH"
export PATH

# 1) QMK tree, pinned.
if [ ! -d "$QMK/.git" ]; then
    echo ">> cloning $QMK_URL ($QMK_BRANCH)"
    git clone --depth 1 --branch "$QMK_BRANCH" "$QMK_URL" "$QMK"
    ( cd "$QMK" && git fetch --depth 1 origin "$QMK_PIN" && git checkout -q "$QMK_PIN" )
fi
echo ">> init submodules"
( cd "$QMK" && git submodule update --init --depth 1 lib/chibios lib/chibios-contrib lib/printf )

# 2) Patches (see README): iton_bt_deinit + SN32 SPI slave TX fix. Skipped if
#    already applied; a patch that no longer applies aborts the build.
for p in "$DIR"/patches/*.patch; do
    if ( cd "$QMK" && git apply --check "$p" 2>/dev/null ); then
        echo ">> applying $(basename "$p")"
        ( cd "$QMK" && git apply "$p" )
    elif ! ( cd "$QMK" && git apply --reverse --check "$p" 2>/dev/null ); then
        echo "patch neither applies nor is already applied: $p (QMK_PIN bumped?)"
        exit 1
    fi
done

# 3) SonixFlasherC (needs hidapi + libusb dev headers).
if [ ! -x "$TOOLS/SonixFlasherC/sonixflasher" ]; then
    echo ">> building SonixFlasherC"
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
