# Keychron K8 V2 - VIA/OpenRGB Hybrid Firmware

Custom QMK firmware for the Keychron K8 V2 (ANSI RGB) that supports **both VIA and OpenRGB** with a software toggle.

## Features

-   🎮 **VIA Support** - Full VIA configurator compatibility
-   🌈 **OpenRGB Support** - Direct LED control via OpenRGB protocol v0.12
-   🔄 **Software Toggle** - Switch between modes with `Fn + O`
-   ✨ **Visual Feedback** - Animation shows current mode:
    -   RGB colors (Red→Green→Blue) = OpenRGB mode
    -   Pink = VIA mode

## Building

```bash
qmk compile -kb keychron/k8_version_2/ansi/rgb -km via
```

## Flashing

1. Enter DFU mode: Hold **ESC** while plugging in USB
2. Use QMK Toolbox to flash `keychron_k8_version_2_ansi_rgb_via.bin`

## Usage

| Mode    | Toggle                    | Software         |
| ------- | ------------------------- | ---------------- |
| OpenRGB | `Fn + O` (RGB animation)  | OpenRGB          |
| VIA     | `Fn + O` (Pink animation) | VIA Configurator |

**Default mode:** OpenRGB

## Technical Details

-   Based on Keychron's `wls_2025q1` branch
-   RAW_EPSIZE: 64 bytes (required for OpenRGB)
-   Protocol version: 0x0C

## Files Changed

| File                                                    | Description                     |
| ------------------------------------------------------- | ------------------------------- |
| `keyboards/keychron/k8_version_2/ansi/rgb/openrgb.c/h`  | OpenRGB protocol implementation |
| `keyboards/keychron/common/hybrid_switch_animation.c/h` | Mode switch animation           |
| `keyboards/keychron/common/keychron_common.c/h`         | ORGB keycode handler            |
| `quantum/via.c`                                         | Hybrid dispatch logic           |
| `tmk_core/protocol/usb_descriptor.h`                    | 64-byte endpoint size           |

## Credits

-   Based on [Keychron QMK Firmware](https://github.com/Keychron/qmk_firmware)
-   OpenRGB protocol from [QMK-OpenRGB](https://github.com/Kasper24/QMK-OpenRGB)
