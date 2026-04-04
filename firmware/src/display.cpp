#include "display.h"
#include "pins.h"
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <SPI.h>

// Color palette
#define C_BG      0x0000
#define C_WHITE   0xFFFF
#define C_ACCENT  0x07FF  // cyan
#define C_DIM     0x4208  // dark gray
#define C_GREEN   0x07E0
#define C_HEADER  0x0319  // dark teal

// Software SPI — works reliably in Wokwi simulation
static Adafruit_ILI9341 tft(TFT_CS, TFT_DC, SPI_MOSI, SPI_SCK, TFT_RST, SPI_MISO);

void initDisplay() {
    tft.begin();
    tft.setRotation(1);  // landscape: 320 wide x 240 tall
    tft.fillScreen(C_BG);

#if TFT_BL >= 0
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);
#endif
}

// ── helpers ───────────────────────────────────────────────────────────────

static void drawHeader(const char* label) {
    tft.fillRect(0, 0, 320, 22, C_ACCENT);
    tft.setTextColor(C_BG);
    tft.setTextSize(1);
    tft.setCursor(6, 7);
    tft.print(label);
}

static void drawProgressBar(int x, int y, int w, int h,
                             int val, int maxVal, uint16_t fillColor) {
    tft.fillRect(x, y, w, h, C_DIM);
    if (maxVal > 0) {
        int fill = (int)((float)w * val / maxVal);
        fill = constrain(fill, 0, w);
        tft.fillRect(x, y, fill, h, fillColor);
    }
}

static void formatTime(char* buf, int secs) {
    sprintf(buf, "%d:%02d", secs / 60, secs % 60);
}

// ── public screens ────────────────────────────────────────────────────────

void drawBrowser(const char** files, int count, int selected) {
    tft.fillScreen(C_BG);
    drawHeader("FILES");

    if (count == 0) {
        tft.setTextColor(C_DIM);
        tft.setTextSize(1);
        tft.setCursor(8, 110);
        tft.print("No MP3 files found on SD");
        return;
    }

    const int ROW_H     = 22;
    const int MAX_ROWS  = 9;
    const int START_Y   = 26;

    int startIdx = selected - MAX_ROWS / 2;
    startIdx = constrain(startIdx, 0, max(0, count - MAX_ROWS));

    for (int i = 0; i < MAX_ROWS && (startIdx + i) < count; i++) {
        int idx = startIdx + i;
        int y   = START_Y + i * ROW_H;

        if (idx == selected) {
            tft.fillRect(0, y, 320, ROW_H, C_ACCENT);
            tft.setTextColor(C_BG);
        } else {
            tft.setTextColor(idx % 2 == 0 ? C_WHITE : 0xC618);
        }

        tft.setTextSize(1);
        tft.setCursor(8, y + 7);

        // Strip path prefix, truncate to fit
        const char* name = strrchr(files[idx], '/');
        name = name ? name + 1 : files[idx];
        char buf[40];
        strncpy(buf, name, 39);
        buf[39] = '\0';
        tft.print(buf);
    }

    // Scrollbar
    if (count > MAX_ROWS) {
        int barH  = (int)(218.0f * MAX_ROWS / count);
        int barY  = 22 + (int)(218.0f * startIdx / count);
        tft.fillRect(315, 22, 5, 218, C_DIM);
        tft.fillRect(315, barY, 5, barH, C_ACCENT);
    }
}

void drawPlayer(const char* trackName, int elapsedSec, int totalSec,
                int volume, bool playing) {
    tft.fillScreen(C_BG);
    drawHeader(playing ? "NOW PLAYING" : "  PAUSED  ");

    // Track name (two lines if needed)
    tft.setTextColor(C_WHITE);
    tft.setTextSize(2);
    tft.setCursor(8, 30);
    char name[22];
    strncpy(name, trackName, 21);
    name[21] = '\0';
    tft.print(name);

    // Progress bar
    drawProgressBar(8, 80, 304, 10, elapsedSec, totalSec > 0 ? totalSec : 1, C_ACCENT);

    // Time labels
    char tBuf[8];
    tft.setTextSize(1);
    tft.setTextColor(C_DIM);
    formatTime(tBuf, elapsedSec);
    tft.setCursor(8, 94);
    tft.print(tBuf);
    if (totalSec > 0) {
        formatTime(tBuf, totalSec);
        int tw = strlen(tBuf) * 6;
        tft.setCursor(312 - tw, 94);
        tft.print(tBuf);
    }

    // Volume label + bar
    tft.setTextColor(C_DIM);
    tft.setCursor(8, 116);
    tft.print("VOL");
    drawProgressBar(34, 118, 240, 8, volume, 100, C_GREEN);
    tft.setTextColor(C_WHITE);
    char vBuf[5];
    sprintf(vBuf, "%3d%%", volume);
    tft.setCursor(280, 116);
    tft.print(vBuf);

    // Play / pause icon
    int cx = 160, cy = 170;
    if (playing) {
        tft.fillRect(cx - 12, cy, 9, 26, C_WHITE);
        tft.fillRect(cx + 4,  cy, 9, 26, C_WHITE);
    } else {
        // Filled triangle pointing right
        for (int r = 0; r < 26; r++) {
            tft.drawFastHLine(cx - 12 + r / 2, cy + r, 26 - r, C_WHITE);
        }
    }

    // Controls hint
    tft.setTextColor(C_DIM);
    tft.setTextSize(1);
    tft.setCursor(8, 226);
    tft.print("turn=vol  click=pause  hold=back");
}

void drawMessage(const char* msg) {
    tft.fillScreen(C_BG);
    tft.setTextColor(C_WHITE);
    tft.setTextSize(2);
    int16_t x1, y1;
    uint16_t w, h;
    tft.getTextBounds(msg, 0, 0, &x1, &y1, &w, &h);
    tft.setCursor((320 - w) / 2, (240 - h) / 2);
    tft.print(msg);
}
