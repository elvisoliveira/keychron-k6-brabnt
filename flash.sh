#!/bin/sh
# Flash a firmware to the K6 over the Sonix bootloader.
#   ./flash.sh            -> flashes the built brabnt firmware
#   ./flash.sh <file.bin> -> flashes an arbitrary .bin
#   ./flash.sh -w [...]   -> waits for the bootloader to show up, then flashes
#
# Enter bootloader first: press Fn1+Space with USB-C plugged (RGB goes off). On a
# fresh stock keyboard (never flashed), instead open the case and short the two
# boot pads on the PCB while plugging USB-C. Confirmed when lsusb shows 0c45:7040.
set -eu

DIR="$(cd "$(dirname "$0")" && pwd)"
WAIT=0; [ "${1:-}" = "-w" ] && { WAIT=1; shift; }
BIN="${1:-$DIR/.qmk/keychron_k6_rgb_v2_ansi_brabnt.bin}"
FLASHER="$DIR/.tools/SonixFlasherC/sonixflasher"

[ -x "$FLASHER" ] || { echo "flasher missing — run ./bootstrap.sh first"; exit 1; }
[ -f "$BIN" ]   || { echo "firmware not found: $BIN — run ./bootstrap.sh"; exit 1; }

if [ "$WAIT" = 1 ]; then
    echo "waiting for bootloader (0c45:7040): press Fn1+Space with USB-C plugged... (Ctrl+C aborts)"
    until lsusb | grep -q 0c45:7040; do sleep 0.5; done
elif ! lsusb | grep -q 0c45:7040; then
    echo "bootloader (0c45:7040) not found."
    echo "press Fn1+Space with USB-C plugged (RGB goes off), then re-run."
    exit 1
fi

# SN32F248BF = SN32F24xB -> 0c45/7040, offset 0, no jumploader.
# No root: needs udev/70-sonix-bootloader.rules installed (see README).
echo "flashing: $BIN"
"$FLASHER" --vidpid 0c45/7040 --file "$BIN" --offset 0
