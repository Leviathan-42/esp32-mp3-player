#include <Arduino.h>
#include "pins.h"

#ifdef HARDWARE_TEST

#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <AudioOutputI2S.h>
#include <SPI.h>
#include <math.h>

#define C_BG      0x0000
#define C_WHITE   0xFFFF
#define C_ACCENT  0x07FF
#define C_DIM     0x4208
#define C_GREEN   0x07E0
#define C_RED     0xF800

static Adafruit_ST7789 tft(&SPI, TFT_CS, TFT_DC, TFT_RST);
static AudioOutputI2S* audioOut = nullptr;

static const int buttonPins[] = {ENC_BTN, ENC_SW2, ENC_SW3, ENC_SW4, ENC_SW5};
static const char* buttonNames[] = {"SW1 CENTER", "SW2 DOWN", "SW3 RIGHT", "SW4 UP", "SW5 LEFT"};
static const int buttonCount = sizeof(buttonPins) / sizeof(buttonPins[0]);

enum AudioMode { AUDIO_OFF, AUDIO_SILENCE, AUDIO_TONE };

static int lastA = HIGH;
static int encoderTicks = 0;
static AudioMode audioMode = AUDIO_OFF;
static bool lastCenterPressed = false;
static float tonePhase = 0.0f;
static bool lastButtonStates[buttonCount] = {};

static bool audioActive() {
    return audioMode != AUDIO_OFF;
}

static void drawCounter() {
    tft.fillRect(58, 72, 80, 28, C_BG);
    tft.setTextSize(2);
    tft.setTextColor(C_WHITE);
    tft.setCursor(58, 76);
    tft.print(encoderTicks);
}

static void drawDacState() {
    tft.fillRect(198, 72, 100, 28, C_BG);
    tft.setTextSize(2);
    tft.setTextColor(audioActive() ? C_GREEN : C_DIM);
    tft.setCursor(198, 76);
    if (audioMode == AUDIO_TONE) {
        tft.print("TONE");
    } else if (audioMode == AUDIO_SILENCE) {
        tft.print("SIL");
    } else {
        tft.print("OFF");
    }
}

static void drawAudioNotice() {
    tft.fillRect(8, 222, 304, 14, C_BG);
    tft.setTextSize(1);
    tft.setTextColor(C_DIM);
    tft.setCursor(8, 226);
    if (audioMode == AUDIO_TONE) {
        tft.print("Tone output: listen for signal noise");
    } else if (audioMode == AUDIO_SILENCE) {
        tft.print("Digital silence: listen for analog noise");
    } else {
        tft.print("Center cycles OFF > SIL > TONE");
    }
}

static void drawButtonRow(int i) {
    bool pressed = digitalRead(buttonPins[i]) == LOW;
    int y = 116 + i * 20;
    tft.fillRect(8, y, 304, 16, pressed ? C_GREEN : C_DIM);
    tft.setTextSize(1);
    tft.setTextColor(pressed ? C_BG : C_WHITE);
    tft.setCursor(14, y + 4);
    tft.print(buttonNames[i]);
    tft.setCursor(230, y + 4);
    tft.print(pressed ? "PRESSED" : "open");
}

static void initDacTone() {
    audioOut = new AudioOutputI2S();
    audioOut->SetPinout(I2S_BCLK, I2S_LRCLK, I2S_DOUT);
    audioOut->SetRate(44100);
    audioOut->SetBitsPerSample(16);
    audioOut->SetChannels(2);
    audioOut->SetGain(0.45f);
    audioOut->begin();
}

static void writeToneChunk() {
    if (!audioOut) return;

    const float sampleRate = 44100.0f;
    const float frequency = 440.0f;
    const float step = 2.0f * PI * frequency / sampleRate;

    for (int i = 0; i < 128; i++) {
        int16_t sampleValue = audioMode == AUDIO_TONE ? (int16_t)(sinf(tonePhase) * 16000.0f) : 0;
        int16_t sample[2] = {sampleValue, sampleValue};
        audioOut->ConsumeSample(sample);
        tonePhase += step;
        if (tonePhase >= 2.0f * PI) tonePhase -= 2.0f * PI;
    }
}

static void drawLayout() {
    tft.fillScreen(C_BG);
    tft.setTextSize(2);
    tft.setTextColor(C_ACCENT);
    tft.setCursor(8, 8);
    tft.print("HARDWARE TEST v2");

    tft.setTextSize(1);
    tft.setTextColor(C_WHITE);
    tft.setCursor(8, 36);
    tft.print("Turn wheel: counter changes");
    tft.setCursor(8, 48);
    tft.print("Center: OFF > SILENCE > TONE");

    tft.setTextSize(2);
    tft.setTextColor(C_WHITE);
    tft.setCursor(8, 76);
    tft.print("ROT ");
    drawCounter();

    tft.setCursor(150, 76);
    tft.print("DAC ");
    drawDacState();

    for (int i = 0; i < buttonCount; i++) {
        drawButtonRow(i);
    }

    drawAudioNotice();
}

void setup() {
    Serial.begin(115200);
    delay(200);
    Serial.println("Hardware test booting...");

    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, TFT_CS);
    tft.init(240, 320);
    tft.setRotation(3);

#if TFT_BL >= 0
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);
#endif

    pinMode(ENC_A, INPUT_PULLUP);
    pinMode(ENC_B, INPUT_PULLUP);
    for (int i = 0; i < buttonCount; i++) {
        pinMode(buttonPins[i], INPUT_PULLUP);
        lastButtonStates[i] = digitalRead(buttonPins[i]) == LOW;
    }
    lastA = digitalRead(ENC_A);

    initDacTone();
    drawLayout();
}

void loop() {
    bool counterDirty = false;
    bool dacDirty = false;

    bool centerPressed = digitalRead(ENC_BTN) == LOW;
    if (centerPressed && !lastCenterPressed) {
        audioMode = (AudioMode)((audioMode + 1) % 3);
        dacDirty = true;
    }
    lastCenterPressed = centerPressed;

    if (!audioActive()) {
        int a = digitalRead(ENC_A);
        if (a != lastA && a == LOW) {
            int b = digitalRead(ENC_B);
            encoderTicks += (b == HIGH) ? 1 : -1;
            counterDirty = true;
        }
        lastA = a;
    }

    if (counterDirty) {
        drawCounter();
    }
    if (dacDirty) {
        drawDacState();
        drawAudioNotice();
    }

    if (!audioActive()) {
        for (int i = 0; i < buttonCount; i++) {
            bool pressed = digitalRead(buttonPins[i]) == LOW;
            if (pressed != lastButtonStates[i]) {
                lastButtonStates[i] = pressed;
                drawButtonRow(i);
            }
        }
    }

    writeToneChunk();
}

#else

#include "display.h"
#include "encoder.h"
#include "player.h"
#include "filebrowser.h"

// ── App state ─────────────────────────────────────────────────────────────

enum AppState { STATE_BROWSER, STATE_PLAYING };
static AppState appState  = STATE_BROWSER;
static int      selected  = 0;      // highlighted file in browser
static int      currentTrackIdx = -1;

// Redraw throttle. Full-screen TFT redraws can starve MP3/I2S playback, so the
// player view only redraws on entry or direct user actions for now.
static uint32_t lastRedraw   = 0;

// ── helpers ───────────────────────────────────────────────────────────────

static const char* filePointers[MAX_FILES];  // const char* view of fileList

static const char* trackDisplayName(int idx);

static void buildPointers() {
    for (int i = 0; i < fileCount; i++) {
        filePointers[i] = fileList[i];
    }
}

static void startTrack(int idx) {
    if (idx < 0 || idx >= fileCount) return;
    currentTrackIdx = idx;

    drawPlayer(
        trackDisplayName(currentTrackIdx),
        0,
        0,
        getVolume(),
        true
    );
    lastRedraw = millis();

    if (!playTrack(fileList[idx])) {
        drawMessage("Play failed");
        delay(900);
        drawBrowser(filePointers, fileCount, selected);
        return;
    }
    appState = STATE_PLAYING;
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
            } else if (evt == ENC_CCW) {
                setVolume(getVolume() - 5);
            } else if (evt == ENC_CLICK) {
                togglePause();
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

            if (forceRedraw || lastRedraw == 0) {
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

#endif
