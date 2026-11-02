# keychron-k6-brabnt

`brabnt` QMK keymap for the **Keychron K6 RGB, ANSI** (MCU SN32F248BF), with
Bluetooth working. ABNT2 helpers + ergonomic remaps.

The original K6 uses a Sonix SN32 MCU (no mainline QMK support), so this builds
on upstream **SonixQMK** (pinned in `bootstrap.sh`). The K6 isn't upstream: its
definition (with Bluetooth via `iton_bt`) lives here in `keyboard/`, plus two
patches in `patches/`: `iton_bt_deinit()` for the driver, and reverting an SN32
SPI change (a trailing `0x00` left in the slave TX FIFO shifts every later
packet to the BT chip by one byte — cable works, Bluetooth breaks).

**Bluetooth:** hold `Fn1`+`Q` 3 s to pair (Q blinks blue, green once connected),
then pair from the host with `bluetoothctl pair` → `trust` → `connect` (plain
`connect` leaves it un-bonded and it won't reconnect). Reconnect: slider off → BT.

## Build & flash
```sh
./bootstrap.sh   # clone pinned SonixQMK, patch, link keyboard, build flasher, compile
# enter bootloader (below), then (or ./flash.sh -w first, it waits for it):
./flash.sh
```
`bootstrap.sh` generates `.qmk/` and `.tools/` (gitignored).

**Needs:** `qmk` (Arch: pacman `qmk`), `git`, `make`, `arm-none-eabi-gcc`,
`hidapi`+`libusb` headers, a running polkit agent (`pkexec`), and the OS on
**br-abnt2** (the `/?\|` and quote keys assume it).

## Enter the bootloader
Ready when `lsusb` shows `0c45:7040` (RGB off, keys dead).

**First time (stock fw)** — no key combo works, so open the case:
1. Unplug; mode slider to **Cable**.
2. Open the case, lift the PCB (no desoldering).
3. Short the **two boot pads** — on the K6 they're **not** under the spacebar
   (that's the K4). Unsure which? See the
   [QMK-on-K6 guide](https://github.com/CanUnesi/QMK-on-K6) (photo) or the
   [SonixQMK Discord](https://sonixqmk.github.io); wrong pads can fry it.
4. Hold the short, plug USB-C, release.
5. Check `lsusb | grep 0c45:7040`, then `./flash.sh`.

**After QMK** — press `Fn1`+`Space` (`QK_BOOT`) with USB-C plugged.

## Flash params
`0c45/7040`, offset `0x00`, no jumploader (SN32F24xB). The first QMK flash drops
code security CS1→CS0; later flashes need nothing special.

## Files
`keymap/` (layout) · `keyboard/` (K6 definition, LEDs/BT in `rgb/v2/ansi/ansi.c`) ·
`patches/` (QMK patches) · `bootstrap.sh` (build) · `flash.sh` (flash).
