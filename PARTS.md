# DIY ESP32 MP3 Player — Parts & Project Details

## Overview

A handheld MP3 player built around the Adafruit Feather ESP32-S3, with a 2" IPS TFT display,
stereo I2S audio output, ANO rotary wheel navigation, and a 3D printed chassis.

---

## Parts List

| Component | Adafruit Product | ID | Price | Notes |
|---|---|---|---|---|
| MCU | ESP32-S3 Feather 4MB Flash / 2MB PSRAM | #5477 | $17.50 | PSRAM needed for audio buffering |
| Screen | 2.0" 320x240 Color IPS TFT (ST7789) | #4311 | $19.95 | SPI, built-in microSD slot, no touch |
| Audio DAC | I2S Stereo DAC Breakout - UDA1334A | #3678 | $5.95 | Stereo, 3.5mm headphone jack included |
| Power | PowerBoost 1000C Charger | #2465 | $19.95 | LiPo charging + 5V boost, USB passthrough |
| Battery | Lithium Ion Battery 2200mAh 3.7V | #1781 | ~$10.00 | |
| Encoder Breakout | ANO Rotary Navigation Encoder Breakout | #5221 | $1.50 | For clean wiring of the ANO wheel |
| Jumper Wires | Premium Female/Female Jumper Wires | #1957 | ~$2.00 | For connecting TFT to Feather via SPI |
| Power Switch | Panel Mount Latching Push Button | #3226 | ~$1.00 | Wired to PowerBoost enable pin |
| Storage | MicroSD Card 32GB (any brand) | — | ~$8.00 | For storing MP3 files |
| Pin Headers | Break-away 0.1" Male Headers | #392 | ~$1.50 | For soldering Feather/breakout boards |
| **ANO Encoder** | **ANO Rotary Navigation Encoder** | **#5001** | **owned** | Already have this |

**Estimated Total: ~$87**

---

## Wiring Overview

```
ESP32-S3 Feather
├── SPI  → TFT #4311 (SCK, MOSI, CS, DC, RST)
├── I2S  → UDA1334A DAC (BCLK, WSEL, DIN)
├── SPI  → MicroSD slot on TFT #4311 (shared SPI bus, separate CS)
├── GPIO → ANO Encoder via breakout #5221 (2 encoder pins + click)
└── BAT  → PowerBoost 1000C → LiPo 2200mAh
         → EN pin → panel mount power switch
```

---

## Software Stack

- **Framework:** Arduino (via PlatformIO or Arduino IDE)
- **Audio decoding:** [ESP8266Audio](https://github.com/earlephilhower/ESP8266Audio) — software MP3 decode on ESP32
- **Display:** Adafruit ST7789 library + Adafruit GFX
- **SD card:** SdFat or Arduino SD library
- **Encoder:** ESP32Encoder library

---

## Features Planned

- [ ] MP3 playback from SD card
- [ ] Track navigation via ANO wheel (spin = next/prev, click = play/pause)
- [ ] Display: track name, artist, progress bar, volume level
- [ ] Volume control via wheel (hold + spin or long press mode)
- [ ] Battery level indicator on screen
- [ ] Simple file browser for selecting tracks

---

## Tools Required

- Soldering iron + solder
- USB-C cable (for flashing firmware)
- 3D printer + filament (for chassis)
- Wire cutters/strippers

---

## Board Dimensions (for chassis design)

| Board | Dimensions |
|---|---|
| Adafruit Feather ESP32-S3 | 50.8mm × 22.9mm |
| TFT 2.0" #4311 | 55.0mm × 35.0mm |
| UDA1334A DAC #3678 | 22.0mm × 18.0mm |
| PowerBoost 1000C | 35.5mm × 22.0mm |
| ANO Encoder | 30mm diameter (wheel) |
