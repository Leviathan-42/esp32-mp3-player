# ESP32-S3 MP3 Player Wiring

This is the recommended compact wiring plan for the Adafruit Feather ESP32-S3
4MB Flash / 2MB PSRAM, Adafruit 2.0" ST7789 TFT with microSD, UDA1334A I2S DAC,
and ANO rotary navigation encoder breakout.

## Current Build Status

This document is the source of truth for the real Feather wiring harness. The
current build is wired according to this plan:

- No physical power switch is installed.
- TFT `RST` is not connected; insulate the loose reset lead if one was soldered
  to the Feather `RST` pad.
- The optional 100k pull-up from `D5` / `SW1` to `3V` may be installed for more
  reliable deep-sleep wake behavior.
- `firmware/src/pins.h` must be updated to match the "Firmware Pin Map" section
  before flashing this wiring onto the real Feather. Some firmware pins are
  currently still set for Wokwi simulation.

## Wire Color Legend

With only six colors, some colors repeat. Label both ends of every repeated
wire with tape or heat-shrink flags.

| Color  | Use                                                  |
| ------ | ---------------------------------------------------- |
| RED    | 3.3V power                                           |
| BLACK  | GND / common                                         |
| BLUE   | Clocks                                               |
| GREEN  | Data from Feather to module                          |
| WHITE  | Data from module to Feather, buttons, encoder inputs |
| YELLOW | Chip-select, control, reset, backlight               |

22 AWG is bulky for these small boards. It is fine for power and ground, but
for signal wires keep runs short, add strain relief, and avoid flexing the
solder joints.

## Power, Off, and Sleep

Recommended compact build: plug the LiPo directly into the Feather JST battery
connector and use the Feather USB-C port for charging/programming. The Feather
already has a charger and fuel gauge.

Do not feed a PowerBoost 5V output into the Feather BAT pin.

The ANO wheel can control sleep in software. It cannot truly turn the Feather
back on after the Feather EN pin has been pulled low, because the ESP32 is not
running while EN is off.

Target firmware behavior:

- Long press ANO center button: firmware turns off the TFT backlight and enters
  ESP32-S3 deep sleep.
- Press any ANO button: wake from deep sleep.
- Optional physical kill switch: wire a latching switch to Feather `EN` and
  `GND` for true off during storage.

The current physical build omits the optional kill switch. This means the
Feather can sleep in software, but it is not fully disconnected from the
battery during storage.

Optional power switch:

| Color | Feather pin | Switch pin        |
| ----- | ----------- | ----------------- |
| BLACK | GND         | Switch terminal 1 |
| WHITE | EN          | Switch terminal 2 |

The Feather EN pin is pulled high. Connecting EN to GND turns the Feather off.
With a simple latching switch, the "closed" switch position is off.

If no physical switch is installed, leave `EN` unconnected.

## Shared Power Rails

The Feather has two accessible `3V` pads. Use both to reduce crowding. This
plan assumes you only have one accessible `GND` pad, so ground still uses one
black pigtail splice.

```text
Feather 3V pad #1 -> TFT Vin

Feather 3V pad #2 -> RED splice -> UDA1334A VIN
                                -> 100k pull-up resistor

Feather GND       -> BLACK splice -> TFT GND
                          -> UDA1334A GND
                          -> ANO COMA
                          -> ANO COMB
```

## Multi-Wire Join Points

Most signal wires are one-to-one. These are the places where multiple wires meet:

| Node | Connections | Recommended join |
|---|---|---|
| `3V` display power | Feather `3V` pad #1, TFT `Vin` | Direct wire |
| `3V` accessory power | Feather `3V` pad #2, UDA1334A `VIN`, 100k pull-up resistor | Red pigtail splice |
| `GND` common | Feather `GND`, TFT `GND`, UDA1334A `GND`, ANO `COMA`, ANO `COMB`, optional EN switch | Black pigtail splice |
| Center button signal | Feather `D5`, ANO `SW1`, 100k pull-up resistor | Small T-splice near Feather or ANO |

Avoid putting more than one 22 AWG wire into the same PCB through-hole. Put one
short pigtail from the PCB pad to the splice, then branch from the splice.

For a compact soldered harness:

1. Strip about 6-8 mm from each wire end.
2. Tin each stripped end lightly.
3. Twist the pigtails for one node together.
4. Solder the twisted bundle until the solder wicks through the joint.
5. Trim sharp ends.
6. Cover the joint with heat-shrink.
7. Anchor the finished harness with tape, hot glue, E6000, or UV resin after
   testing so the solder joints are not flexed.

For the 100k pull-up, make a T-splice instead of a series splice:

```text
Feather 3V ----[ 100k resistor ]----+
                                    |
Feather D5 -------------------------+---- ANO SW1
```

## TFT ST7789 Display and microSD

Use the Feather hardware SPI pins for the display and SD card. The TFT and SD
card share SCK, MOSI, and MISO. They use separate chip-select pins.

| Color  | Feather pin label | Feather GPIO | TFT pin label     | Purpose               |
| ------ | ----------------: | -----------: | ----------------- | --------------------- |
| RED    |                3V |            - | Vin / 3-5V        | Display power         |
| BLACK  |               GND |            - | GND               | Ground                |
| BLUE   |               SCK |       GPIO36 | SCK               | SPI clock             |
| GREEN  |         MO / MOSI |       GPIO35 | MOSI              | SPI data to TFT/SD    |
| WHITE  |         MI / MISO |       GPIO37 | MISO              | SPI data from SD      |
| YELLOW |               D10 |       GPIO10 | CS / TCS          | TFT chip select       |
| YELLOW |                D9 |        GPIO9 | D/C / DC          | TFT data/command      |
| YELLOW |               D11 |       GPIO11 | SDCS / SD Card CS | microSD chip select   |
| YELLOW |               D12 |       GPIO12 | BL / Lite         | PWM backlight control |

Notes:

- The display has automatic reset. Leave TFT `RST` unconnected at first.
- If reset is unreliable, wire TFT `RST` to Feather `RST`.
- Wire `BL` to `D12` for PWM dimming and sleep blanking.
- If a yellow lead is already soldered to Feather `RST`, leave the far end
  insulated unless the display needs the reset connection later.

## UDA1334A I2S DAC

Place the DAC close to the headphone jack or audio output. Keep the analog audio
side away from SPI clock/backlight wires where practical.

| Color | Feather pin label | Feather GPIO | UDA1334A pin label | Purpose                    |
| ----- | ----------------: | -----------: | ------------------ | -------------------------- |
| RED   |                3V |            - | VIN                | DAC power                  |
| BLACK |               GND |            - | GND                | Ground                     |
| BLUE  |                RX |       GPIO38 | BCLK               | I2S bit clock              |
| BLUE  |                TX |       GPIO39 | WSEL               | I2S word select / LR clock |
| GREEN |                A4 |       GPIO14 | DIN                | I2S audio data             |

Leave UDA1334A `MCLK` / `SCLK`, `MUTE`, `DEMP`, `PLL`, `SF0`, and `SF1`
unconnected for the normal I2S default configuration.

## ANO Encoder Breakout

Wire all ANO switches now. The five switch pins below are on RTC-capable GPIOs,
which makes deep-sleep wake practical.

| Color | Feather pin label | Feather GPIO | ANO breakout pin | Purpose                              |
| ----- | ----------------: | -----------: | ---------------- | ------------------------------------ |
| BLACK |               GND |            - | COMA             | Common for encoder and center switch |
| WHITE |                A0 |       GPIO18 | ENCA             | Rotary encoder A                     |
| WHITE |                A1 |       GPIO17 | ENCB             | Rotary encoder B                     |
| BLACK |               GND |            - | COMB             | Common for direction switches        |
| WHITE |                D5 |        GPIO5 | SW1              | Center click, sleep/wake             |
| WHITE |                D6 |        GPIO6 | SW2              | Down                                 |
| WHITE |                A5 |        GPIO8 | SW3              | Right                                |
| WHITE |                A2 |       GPIO16 | SW4              | Up                                   |
| WHITE |                A3 |       GPIO15 | SW5              | Left                                 |

The ANO breakout does not connect COMA and COMB together. Connect both to GND.

For more reliable deep-sleep wake, add one optional resistor. This is a pull-up
from the center-button signal to 3.3V. It is not a series resistor between the
Feather and the ANO breakout.

| Part          | From                     | To           | Purpose                                           |
| ------------- | ------------------------ | ------------ | ------------------------------------------------- |
| 100k resistor | Feather `D5` / ANO `SW1` | Feather `3V` | Keeps center button pulled high during deep sleep |

Physical wiring:

```text
Feather 3V ----[ 100k resistor ]----+
                                    |
Feather D5 -------------------------+---- ANO SW1

Feather GND ----------------------------- ANO COMA
```

You can put the resistor inline inside heat-shrink as a short branch from the
`D5`/`SW1` wire over to `3V`. Do not cut the `D5` to `SW1` wire and insert the
resistor in the middle; that would make button sensing unreliable.

The firmware should still enable internal pull-ups for all ANO inputs during
normal operation.

Firmware note: the current firmware reads encoder rotation and the center
button only. `SW2` through `SW5` are wired for directional controls and future
deep-sleep wake support, but code must be added before those buttons do
anything in the app.

## Firmware Pin Map

When flashing the real Adafruit Feather ESP32-S3 build, `firmware/src/pins.h`
should match this wiring:

```cpp
// --- SPI Bus ---
#define SPI_SCK   36   // Feather SCK
#define SPI_MOSI  35   // Feather MO / MOSI
#define SPI_MISO  37   // Feather MI / MISO

// --- TFT Display (SPI) ---
#define TFT_CS    10   // Feather D10 -> TFT CS / TCS
#define TFT_DC    9    // Feather D9  -> TFT D/C
#define TFT_RST   -1   // TFT RST left unconnected
#define TFT_BL    12   // Feather D12 -> TFT BL / Lite

// --- SD Card (SPI, shared bus) ---
#define SD_CS     11   // Feather D11 -> TFT SDCS

// --- I2S Audio (UDA1334A DAC) ---
#define I2S_BCLK  38   // Feather RX -> UDA BCLK
#define I2S_LRCLK 39   // Feather TX -> UDA WSEL
#define I2S_DOUT  14   // Feather A4 -> UDA DIN

// --- ANO Rotary Encoder (via breakout #5221) ---
#define ENC_A     18   // Feather A0 -> ANO ENCA
#define ENC_B     17   // Feather A1 -> ANO ENCB
#define ENC_BTN   5    // Feather D5 -> ANO SW1
```

The Wokwi simulation uses different pins because its ESP32-S3 DevKitC-1 board
does not have the same safe pin mapping as the Feather.

## Top-Level Diagram

```text
                         RED 3.3V  --------------------+
                                                       |
Feather ESP32-S3                                       |
----------------                                       |
3V  ---------------------------------------------------+--> TFT Vin
GND ---------------------------------------------------+--> TFT GND
SCK  GPIO36  -- BLUE  -------------------------------> TFT SCK
MO   GPIO35  -- GREEN -------------------------------> TFT MOSI
MI   GPIO37  -- WHITE <------------------------------- TFT MISO
D10  GPIO10  -- YELLOW ------------------------------> TFT CS / TCS
D9   GPIO9   -- YELLOW ------------------------------> TFT D/C / DC
D11  GPIO11  -- YELLOW ------------------------------> TFT SDCS
D12  GPIO12  -- YELLOW ------------------------------> TFT BL / Lite PWM
RST  -- optional YELLOW -----------------------------> TFT RST optional

3V  ---------------------------------------------------+--> UDA VIN
GND ---------------------------------------------------+--> UDA GND
RX   GPIO38  -- BLUE  -------------------------------> UDA BCLK
TX   GPIO39  -- BLUE  -------------------------------> UDA WSEL
A4   GPIO14  -- GREEN -------------------------------> UDA DIN

GND ---------------------------------------------------+--> ANO COMA
GND ---------------------------------------------------+--> ANO COMB
A0   GPIO18  -- WHITE <------------------------------- ANO ENCA
A1   GPIO17  -- WHITE <------------------------------- ANO ENCB
D5   GPIO5   -- WHITE <------------------------------- ANO SW1
D6   GPIO6   -- WHITE <------------------------------- ANO SW2
A5   GPIO8   -- WHITE <------------------------------- ANO SW3
A2   GPIO16  -- WHITE <------------------------------- ANO SW4
A3   GPIO15  -- WHITE <------------------------------- ANO SW5
```

## Pre-Power Checklist

Before plugging in USB or battery:

- Check continuity from every red wire to Feather `3V`.
- Check continuity from every black wire to Feather `GND`.
- Confirm there is no continuity between `3V` and `GND`.
- Confirm TFT `Vin` is not accidentally wired to Feather `BAT`.
- Confirm TFT `BL` goes to Feather `D12`, not directly to `3V`.
- Confirm the PowerBoost, if used later, is not connected to Feather `BAT`.
- Confirm ANO `COMA` and `COMB` are both grounded.

## References

- Adafruit ESP32-S3 Feather pinout: https://learn.adafruit.com/adafruit-esp32-s3-feather/pinouts
- Adafruit 2.0" ST7789 TFT pinout: https://learn.adafruit.com/2-0-inch-320-x-240-color-ips-tft-display/pinouts
- Adafruit UDA1334A pinout: https://learn.adafruit.com/adafruit-i2s-stereo-decoder-uda1334a/pinouts
- Adafruit ANO encoder breakout pinout: https://learn.adafruit.com/ano-rotary-encoder/pinouts
