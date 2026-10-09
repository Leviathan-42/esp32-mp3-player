#pragma once

// --- SPI Bus ---
#define SPI_SCK   36   // Feather SCK
#define SPI_MOSI  35   // Feather MO / MOSI
#define SPI_MISO  37   // Feather MI / MISO

// --- TFT Display (SPI) ---
// Adafruit 2.0" ST7789 #4311
#define TFT_CS    10   // Feather D10 -> TFT CS / TCS
#define TFT_DC    9    // Feather D9 -> TFT D/C
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
#define ENC_SW2   6    // Feather D6 -> ANO SW2 / Down
#define ENC_SW3   8    // Feather A5 -> ANO SW3 / Right
#define ENC_SW4   16   // Feather A2 -> ANO SW4 / Up
#define ENC_SW5   15   // Feather A3 -> ANO SW5 / Left
