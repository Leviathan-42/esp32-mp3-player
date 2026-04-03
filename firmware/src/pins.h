#pragma once

// --- TFT Display (SPI) ---
// Adafruit 2.0" ST7789 #4311
// Shared SPI bus: SCK=36, MOSI=35, MISO=37 (hardware SPI on ESP32-S3 Feather)
#define TFT_CS    9
#define TFT_DC    10
#define TFT_RST   -1   // tie RST pin to 3.3V on the board
#define TFT_BL    5    // backlight (PWM capable)

// --- SD Card (SPI, shared bus with TFT) ---
// Uses the SD slot built into TFT #4311
#define SD_CS     8

// --- I2S Audio (UDA1334A DAC) ---
#define I2S_BCLK  14
#define I2S_LRCLK 15
#define I2S_DOUT  16

// --- ANO Rotary Encoder (via breakout #5221) ---
#define ENC_A     17   // CLK
#define ENC_B     18   // DT
#define ENC_BTN   21   // push button (active LOW)
