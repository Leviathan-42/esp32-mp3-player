#include <Arduino.h>
#include "pins.h"
#include "display.h"
#include "encoder.h"
#include "player.h"
#include "filebrowser.h"

// ── App state ─────────────────────────────────────────────────────────────

enum AppState { STATE_BROWSER, STATE_PLAYING };
static AppState appState  = STATE_BROWSER;
static int      selected  = 0;      // highlighted file in browser
static int      currentTrackIdx = -1;

// Redraw throttle
static uint32_t lastRedraw   = 0;
static const uint32_t REDRAW_MS = 500;  // redraw player screen every 500ms

// ── helpers ───────────────────────────────────────────────────────────────

static const char* filePointers[MAX_FILES];  // const char* view of fileList

static void buildPointers() {
    for (int i = 0; i < fileCount; i++) {
        filePointers[i] = fileList[i];
    }
}

static void startTrack(int idx) {
    if (idx < 0 || idx >= fileCount) return;
    currentTrackIdx = idx;
    playTrack(fileList[idx]);
    appState = STATE_PLAYING;
    lastRedraw = 0;  // force immediate redraw
}

static const char* trackDisplayName(int idx) {
    if (idx < 0 || idx >= fileCount) return "---";
    const char* p = strrchr(fileList[idx], '/');
    return p ? p + 1 : fileList[idx];
}

// ── setup ─────────────────────────────────────────────────────────────────

void setup() {
    Serial.begin(115200);
    delay(200);
    Serial.println("MP3 Player booting...");

    initDisplay();
    drawMessage("Loading...");

    initEncoder();
    initPlayer();

    int found = scanSD();
    buildPointers();

    if (found == 0) {
        drawMessage("No MP3s on SD");
        Serial.println("No MP3 files found — halting");
        while (true) delay(1000);
    }

    drawBrowser(filePointers, fileCount, selected);
    Serial.printf("Ready. %d files.\n", fileCount);
}

// ── loop ──────────────────────────────────────────────────────────────────

void loop() {
    updateEncoder();
    updatePlayer();

    EncoderEvent evt = getEncoderEvent();

    switch (appState) {

        // ── File browser ──────────────────────────────────────────────────
        case STATE_BROWSER: {
            bool dirty = false;

            if (evt == ENC_CW) {
                selected = min(selected + 1, fileCount - 1);
                dirty = true;
            } else if (evt == ENC_CCW) {
                selected = max(selected - 1, 0);
                dirty = true;
            } else if (evt == ENC_CLICK) {
                startTrack(selected);
                return;  // skip redraw, startTrack triggers it
            }

            if (dirty) {
                drawBrowser(filePointers, fileCount, selected);
            }
            break;
        }

        // ── Now playing ───────────────────────────────────────────────────
        case STATE_PLAYING: {
            bool forceRedraw = false;

            if (evt == ENC_CW) {
                setVolume(getVolume() + 5);
                forceRedraw = true;
            } else if (evt == ENC_CCW) {
                setVolume(getVolume() - 5);
                forceRedraw = true;
            } else if (evt == ENC_CLICK) {
                togglePause();
                forceRedraw = true;
            } else if (evt == ENC_HOLD) {
                stopPlayback();
                appState = STATE_BROWSER;
                drawBrowser(filePointers, fileCount, selected);
                return;
            }

            // Track finished — auto-advance
            if (!playerIsPlaying() && !playerIsPaused()) {
                int next = (currentTrackIdx + 1) % fileCount;
                selected = next;
                startTrack(next);
                return;
            }

            // Periodic redraw (progress bar update)
            if (forceRedraw || millis() - lastRedraw >= REDRAW_MS) {
                drawPlayer(
                    trackDisplayName(currentTrackIdx),
                    getElapsedSecs(),
                    0,                           // totalSec unknown without ID3 parsing
                    getVolume(),
                    playerIsPlaying()
                );
                lastRedraw = millis();
            }
            break;
        }
    }
}
