#pragma once

// NOTE: These pins are set for Wokwi simulation (ESP32-S3 DevKitC-1).
// GPIO 35/36/37 are tied to internal flash on the DevKitC-1 and can't be used.
// When flashing to real Adafruit Feather ESP32-S3 hardware, change:
//   SPI_SCK  -> 36
//   SPI_MOSI -> 35
//   SPI_MISO -> 37

// --- SPI Bus ---
#define SPI_SCK   7
#define SPI_MOSI  6
#define SPI_MISO  8

// --- TFT Display (SPI) ---
// Adafruit 2.0" ILI9341 #4311
#define TFT_CS    9
#define TFT_DC    10
#define TFT_RST   -1   // tied to 3.3V
#define TFT_BL    12

// --- SD Card (SPI, shared bus) ---
#define SD_CS     11

// --- I2S Audio (UDA1334A DAC) ---
#define I2S_BCLK  14
#define I2S_LRCLK 15
#define I2S_DOUT  16

// --- ANO Rotary Encoder (via breakout #5221) ---
#define ENC_A     17   // CLK
#define ENC_B     18   // DT
#define ENC_BTN   21   // push button (active LOW)
